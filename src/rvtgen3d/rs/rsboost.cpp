// Based on Boost Custom polygon example code
// https://www.boost.org/doc/libs/1_68_0/libs/geometry/example/c08_custom_non_std_example.cpp

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "rsboost.h"

#include <boost/assert.hpp>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>


#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/util/add_const_if_c.hpp>


extern "C" {

    #include "rsmem.h"
    #include "rsdebug.h"

}


// Sample polygon, having legacy methods
// (similar to e.g. COM objects)

class rs_boost_linestring_t
{

    rs_linestring_t *ls;


    public :

        typedef rs_point_t value_type;

        rs_boost_linestring_t(rs_linestring_t *ls1) {
            ls = ls1;
        };

        void add_point(rs_point_t const& p) {
            rs_linestring_append_point(ls, p);
        };

        // Const access
        rs_point_t const& get_point(std::size_t i) const
        {
            //BOOST_ASSERT(i < points.size());
            return ls->p[i]; // points[i];
        }

        // Mutable access
        rs_point_t & get_point(std::size_t i)
        {
//            BOOST_ASSERT(i < points.size());
            return ls->p[i];
        }


        int point_count() const {
            return ls->points_count;
        }
        void erase_all() {
            rs_linestring_clear(ls);
        }

        inline void set_size(int n) {
            (void)(n); // <-- unused parameter warning supressor
        }
};


// ----------------------------------------------------------------------------
// Adaption: implement iterator and range-extension, and register with Boost.Geometry

// 1) implement iterator (const and non-const versions)
template<typename MyPolygon>
struct custom_iterator : public boost::iterator_facade
                            <
                                custom_iterator<MyPolygon>,
                                rs_point_t,
                                boost::random_access_traversal_tag,
                                typename boost::mpl::if_
                                    <
                                        boost::is_const<MyPolygon>,
                                        rs_point_t const,
                                        rs_point_t
                                    >::type&
                            >
{
    // Constructor for begin()
    explicit custom_iterator(MyPolygon& polygon)
        : m_polygon(&polygon)
        , m_index(0)
    {}

    // Constructor for end()
    explicit custom_iterator(bool, MyPolygon& polygon)
        : m_polygon(&polygon)
        , m_index(polygon.point_count())
    {}


    // Default constructor
    explicit custom_iterator()
        : m_polygon(NULL)
        , m_index(-1)
    {}

    typedef typename boost::mpl::if_
        <
            boost::is_const<MyPolygon>,
            rs_point_t const,
            rs_point_t
        >::type my_point_type;

private:
    friend class boost::iterator_core_access;


    typedef boost::iterator_facade
        <
            custom_iterator<MyPolygon>,
            rs_point_t,
            boost::random_access_traversal_tag,
            my_point_type&
        > facade;

    MyPolygon* m_polygon;
    int m_index;

    bool equal(custom_iterator const& other) const
    {
        return this->m_index == other.m_index;
    }
    typename facade::difference_type distance_to(custom_iterator const& other) const
    {
        return other.m_index - this->m_index;
    }

    void advance(typename facade::difference_type n)
    {
        m_index += n;
        if(m_polygon != NULL
            && (m_index >= m_polygon->point_count()
            || m_index < 0)
            )
        {
            m_index = m_polygon->point_count();
        }
    }

    void increment()
    {
        advance(1);
    }

    void decrement()
    {
        advance(-1);
    }

    // const and non-const dereference of this iterator
    my_point_type& dereference() const
    {
        return m_polygon->get_point(m_index);
    }
};




// 2) Implement Boost.Range const functionality
//    using method 2, "provide free-standing functions and specialize metafunctions"
// 2a) meta-functions
namespace boost
{
    template<> struct range_mutable_iterator<rs_boost_linestring_t>
    {
        typedef custom_iterator<rs_boost_linestring_t> type;
    };

    template<> struct range_const_iterator<rs_boost_linestring_t>
    {
        typedef custom_iterator<rs_boost_linestring_t const> type;
    };

    // RangeEx
    template<> struct range_size<rs_boost_linestring_t>
    {
        typedef std::size_t type;
    };

} // namespace 'boost'


// 2b) free-standing function for Boost.Range ADP
inline custom_iterator<rs_boost_linestring_t> range_begin(rs_boost_linestring_t& polygon)
{
    return custom_iterator<rs_boost_linestring_t>(polygon);
}

inline custom_iterator<rs_boost_linestring_t const> range_begin(rs_boost_linestring_t const& polygon)
{
    return custom_iterator<rs_boost_linestring_t const>(polygon);
}

inline custom_iterator<rs_boost_linestring_t> range_end(rs_boost_linestring_t& polygon)
{
    return custom_iterator<rs_boost_linestring_t>(true, polygon);
}

inline custom_iterator<rs_boost_linestring_t const> range_end(rs_boost_linestring_t const& polygon)
{
    return custom_iterator<rs_boost_linestring_t const>(true, polygon);
}



// 3) optional, for writable geometries only, implement push_back/resize/clear
namespace boost {
	namespace geometry {
		namespace traits {

			template<> struct push_back<rs_boost_linestring_t>
			{
				static inline void apply(rs_boost_linestring_t& polygon, rs_point_t const& point)
				{
					polygon.add_point(point);
				}
			};

			template<> struct resize<rs_boost_linestring_t>
			{
				static inline void apply(rs_boost_linestring_t& polygon, std::size_t new_size)
				{
					polygon.set_size(new_size);
				}
			};

			template<> struct clear<rs_boost_linestring_t>
			{
				static inline void apply(rs_boost_linestring_t& polygon)
				{
					polygon.erase_all();
				}
			};

		}
	}
}


// 4) register with Boost.Geometry
BOOST_GEOMETRY_REGISTER_POINT_2D(rs_point_t, float, boost::geometry::cs::cartesian, x, y)

BOOST_GEOMETRY_REGISTER_LINESTRING(rs_boost_linestring_t)


rs_shape_t *rs_boost_shape_create_simplified(rs_shape_t *src, float tolerance) {

    rs_shape_t *sh = rs_shape_create(src->rings_count);

    for (int ri = 0; ri < src->rings_count; ri++) {


        rs_linestring_t *ls_src = src->rings[ri];

        rs_linestring_t *ls_simplified = rs_linestring_create(ls_src->points_count);

        rs_boost_linestring_t b_ls_src(ls_src);
        rs_boost_linestring_t b_ls_simplified(ls_simplified);

        boost::geometry::simplify(b_ls_src, b_ls_simplified, tolerance);

        rs_shape_append_ring(sh, ls_simplified);

    };

    return sh;

};

rs_shape_t *rs_boost_shape_create_convex_hull(rs_shape_t *src) {

    rs_linestring_t *ls_src = src->rings[0];

    rs_shape_t *sh = rs_shape_create(1);
    sh->outer_rings_count = 1;

    rs_linestring_t *ls_convex_hull = rs_linestring_create(ls_src->points_count + 1);

    rs_boost_linestring_t b_ls_src(ls_src);
    rs_boost_linestring_t b_ls_convex_hull(ls_convex_hull);

    boost::geometry::convex_hull(b_ls_src, b_ls_convex_hull);

    ls_convex_hull->points_count--;

    rs_shape_append_ring(sh, ls_convex_hull);

    return sh;

};
