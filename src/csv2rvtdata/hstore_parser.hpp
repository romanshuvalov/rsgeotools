/*
 * hstore_parser.hpp
 *
 *  Created on: 2016-10-13
 *      Author: Michael Reichert
 */

#ifndef SRC_HSTORE_PARSER_HPP_
#define SRC_HSTORE_PARSER_HPP_

#include <assert.h>
#include <stdexcept>
#include "postgres_parser.hpp"

namespace pg_array_hstore_parser {
    using HStorePartsType = char;

    /**
     * \brief track where we are during parsing hstore
     */
    enum class HStoreParts : HStorePartsType {
        NONE = 0,
        KEY = 1,
        SEPARATOR = 2,
        VALUE = 3,
        END = 4
    };

    /**
     * allows usage of ++HStoreParts
     */
    HStoreParts& operator++(HStoreParts& progress) {
        progress = static_cast<HStoreParts>(static_cast<HStorePartsType>(progress) + 1);
        assert(progress <= HStoreParts::END);
        return progress;
    }

    using StringPair = std::pair<std::string, std::string>;

    /**
     * \brief Parser class for hstores encoded as strings
     *
     * See the [documentation](https://www.postgresql.org/docs/current/static/hstore.html)
     * of PostgreSQL for the representation of hstores as strings.
     *
     * Keys and values are always strings.
     *
     */
    class HStoreParser : public PostgresParser<StringPair> {

        /// shortcut
        using postgres_parser_type = PostgresParser<StringPair>;

        /**
         * \brief Minimum length of a key value pair.
         *
         * Both key and value must be longer than 1 character each. The key-value separator needs two
         * characters. Quotation marks have to be added. Example: "k"=>"v"
         *
         * This results to 7 (left quotation mark does not count).
         *
         * We don't need to start parsing if m_current_position > m_string_repr.size() - MIN_KV_PAIR_LENGTH.
         */
        const size_t MIN_KV_PAIR_LENGTH = 7;

        /// track progress of hstore parsing
        HStoreParts m_parse_progress = HStoreParts::NONE;

        /// buffer for the characters of the key during its parsing
        std::string m_key_buffer;

        /// buffer for the characters of the value during its parsing
        std::string m_value_buffer;

        /**
         * \brief Add a character to either the key or the value buffer.
         *
         * It depends on the value of #m_parse_progress wether the character will be added to the key or the value buffer.
         *
         * \param character the character to be added
         *
         * \throws std::runtime_error if #m_parse_progress is neither HStoreParts::KEY nor HStoreParts::VALUE
         */
        void add_to_key_or_value(char character) {
            switch (m_parse_progress) {
            case HStoreParts::KEY :
                m_key_buffer.push_back(character);
                break;
            case HStoreParts::VALUE :
                m_value_buffer.push_back(character);
                break;
            default :
                assert(false && "You should call add_key_to_value(char) only if you are reading a character of a key or of a value!");
            }
        }

        /**
         * \brief reset key and value buffer
         */
        void reset_buffers() {
            m_key_buffer.clear();
            m_value_buffer.clear();
        }

        /**
         * \brief increase progress counter
         *
         * This helper method is used to shorten the long if-then-else constructs in get_next().
         */
        void increase_parse_progress() {
            ++m_parse_progress;
        }

        void invalid_syntax(std::string error) {
            std::string message = "Invalid hstore syntax at character ";
            message += std::to_string(m_current_position + 1);
            message += " of \"";
            message += m_string_repr;
            message += "\". \"\\";
            message += m_string_repr.at(m_current_position);
            message += "\" ";
            message += error;
            message += '\n';
            throw std::runtime_error(message);
        }

    public:
        /**
         * \param string_repr string representation of the hstore
         */
        HStoreParser(std::string& string_repr) : PostgresParser<StringPair>(string_repr) {};

        /**
         * has the parser reached the end of the hstore
         */
        bool has_next() {
            // m_current_position >= m_string_repr.size() is necessary for empty hstores because m_current_position and
            // m_string_repr.size() are unsigned.
            if (m_current_position > m_string_repr.size() - MIN_KV_PAIR_LENGTH || m_current_position >= m_string_repr.size()) {
                return false;
            }
            return true;
        }

        /**
         * \brief Returns the next key value pair as a pair of strings.
         *
         * \throws std::runtime_error if parsing fails due to a syntax error
         */
        StringPair get_next() {
            reset_buffers();
            m_parse_progress = HStoreParts::NONE;
            char accumulated_backslashes = 0; // counts preceding backslashes
            bool quoted_string = false; // track if the key/value is surrounded by quotation marks

            while (m_current_position < m_string_repr.size() && m_parse_progress != HStoreParts::END) {
                /// Only two special characters have to be escaped in hstores (not more): quotation marks and backslashes.
                /// All other characters inside keys and values do not need any escaping.

                // Handling of escaped quotation marks and backslashes
                if (accumulated_backslashes == 1) {
                    switch (m_string_repr.at(m_current_position)) {
                    case '"':
                        add_to_key_or_value('"');
                        accumulated_backslashes = 0;
                        break;
                    case '\\':
                        add_to_key_or_value('\\');
                        accumulated_backslashes = 0;
                        break;
                    default:
                        invalid_syntax("is no valid escape sequence in a hstore key or value.");
                    }
                } else if (m_string_repr.at(m_current_position) == '\\') {
                    accumulated_backslashes++;

                // Handling of all other characters
                } else if (m_string_repr.at(m_current_position) == '=') {
                    // Key/Values must be surrounded by quotation marks if they contain a '='.
                    if (!quoted_string && (m_parse_progress == HStoreParts::KEY)) {
                        increase_parse_progress();
                    } else if (!quoted_string && (m_parse_progress == HStoreParts::VALUE)) {
                        invalid_syntax("\'=\' is not allowed at the end of a value.");
                    } else if (quoted_string && (m_parse_progress == HStoreParts::KEY || m_parse_progress == HStoreParts::VALUE)) {
                        add_to_key_or_value(m_string_repr.at(m_current_position));
                    } else if (m_parse_progress == HStoreParts::NONE) {
                        invalid_syntax("\'=\' is not allowed there.");
                    }
                } else if (m_string_repr.at(m_current_position) == '>') {
                    // Key/Values must be surrounded by quotation marks if they contain a '>'.
                    if (!quoted_string && (m_parse_progress == HStoreParts::KEY || m_parse_progress == HStoreParts::VALUE)) {
                        invalid_syntax("\'>\' is not allowed inside a key/value or at its end without a preceding '='.");
                    } else if (quoted_string && (m_parse_progress == HStoreParts::KEY || m_parse_progress == HStoreParts::VALUE)) {
                        add_to_key_or_value(m_string_repr.at(m_current_position));
                    } else if (m_parse_progress != HStoreParts::SEPARATOR) {
                        invalid_syntax("\'>\' is not allowed there.");
                    }
                } else if (m_string_repr.at(m_current_position) == '"') {
                    switch (m_parse_progress) {
                    case HStoreParts::NONE :
                    case HStoreParts::SEPARATOR :
                        increase_parse_progress();
                        quoted_string = true;
                        break;
                    case HStoreParts::KEY :
                    case HStoreParts::VALUE :
                        increase_parse_progress();
                        quoted_string = false;
                        break;
                    case HStoreParts::END :
                        invalid_syntax("double '\"' inserted.");
                    }
                } else if (m_string_repr.at(m_current_position) == ' ' || m_string_repr.at(m_current_position) == ',') {
                    /// If a space or a comma appears in a key or value, the key/value has to be surrounded by quotation marks.
                    if (m_parse_progress == HStoreParts::KEY || m_parse_progress == HStoreParts::VALUE) {
                        if (quoted_string) {
                            add_to_key_or_value(m_string_repr.at(m_current_position));
                        } else {
                            increase_parse_progress();
                        }
                    }
                } else {
                    // We only reach this branch if the current character is none of the following: comma, space, quotation mark,
                    // =, >
                    switch (m_parse_progress) {
                    case HStoreParts::KEY :
                    case HStoreParts::VALUE :
                        add_to_key_or_value(m_string_repr.at(m_current_position));
                        break;
                    case HStoreParts::NONE :
                    case HStoreParts::SEPARATOR :
                        // There may be spaces before a key or between "=>" and the begin of a value.
                        increase_parse_progress();
                        add_to_key_or_value(m_string_repr.at(m_current_position));
                        break;
                    default :
                        break;
                    }
                }
                m_current_position++;
            }
            return std::make_pair(m_key_buffer, m_value_buffer);
        }
    };
} // namespace pg_array_hstore_parser


#endif /* SRC_HSTORE_PARSER_HPP_ */
