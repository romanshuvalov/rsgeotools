/*

Copyright (c) 2020, Roman Shuvalov, www.romanshuvalov.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <bzlib.h>

#include <zlib.h>

using namespace std;



int file_is_present(char *s) {

    FILE *fp = fopen(s, "r");
    if (!fp) {
        return 0;
    };
    fclose(fp);
    return 1;

};


typedef struct gpak_file_header_t {

    uint32_t magic_num0;
    uint32_t magic_num1;
    uint32_t version;           // must be 2
    uint32_t flags;

    uint64_t timestamp_planet_osm;
    uint64_t timestamp_pak_creation;

    uint32_t pak_z;
    uint32_t pak_x;
    uint32_t pak_y;
    uint32_t reserved0;

    uint32_t subpak_z;         // tiles are compressed in subpaks
    uint32_t subpak_count;     // must be 4^(subtile_z - pak_z)
    uint32_t dest_z;           // actual tiles
    uint32_t dest_count_per_subpak; // must be 4^(dest_z - subtile_z)

    uint32_t parts_count;       // typically 5 parts for version=2: heightmap, layer:building, layer:area, layer:lines, layer:points
    uint32_t parts_mask;
    uint32_t parts_reserved0;
    uint32_t parts_reserved1;

    uint32_t hm_format;
    uint32_t hm_image_size;
    uint32_t hm_reserved0;
    uint32_t hm_reserved1;

    uint32_t gd_format;
    uint32_t gd_reserved0;
    uint32_t gd_reserved1;
    uint32_t gd_reserved2;


    uint32_t extensions_total_bytes;
    uint32_t extensions_flags;
    uint32_t extensions_reserved0;
    uint32_t extensions_reserved1;

} gpak_file_header_t;



typedef struct subpak_map_record_t {

    uint64_t bytes_shift;

    uint32_t bytes_len;
    uint32_t uncompressed_bytes_len;
    uint32_t compression_type;      // 0 uncompressed, 1 gzip, 2 bzip2
    uint32_t flags;


} subpak_map_record_t;

typedef struct tile_map_record_t {

    uint32_t bytes_shift;
    uint32_t bytes_len;
    uint32_t flags;
    uint32_t reserved0;

} tile_map_record_t;

/*

- file header
- subpak map records[subpak_count]
- array of compressed:
- - tile_map_record_t[tile_count_per_subpak][parts_count]
- - array of
- - - data: heightmap[image_size x image_size] or geodata (b or a or l or p) or something

*/


#define RS_COMPRESS_TYPE_NONE       0
#define RS_COMPRESS_TYPE_ZLIB       1
#define RS_COMPRESS_TYPE_BZIP2      2



uint32_t conv_xy_to_j(uint32_t x, uint32_t y) {

    uint32_t j = 0;

    for (int i = 0; i < 16; i++) {
        j += ( 1<<(i*2+0) ) * ( (x/(1<<i)) % 2 )
           + ( 1<<(i*2+1) ) * ( (y/(1<<i)) % 2 );
    };

    return j;

};


uint32_t conv_j_to_x(uint32_t j) {
    /*
    return
         1*((i/1)%2)
      +  2*((i/4)%2)
      +  4*((i/16)%2)
      +  8*((i/64)%2)
      + 16*((i/256)%2)
      + 32*((i/1024)%2)
      + 64*((i/4096)%2)
      +128*((i/16384)%2)
      +256*((i/65536)%2)

      +512*((i/262144)%2)
      +1024*((i/1048576)%2)
      +2048*((i/4194304)%2)
      +4096*((i/16777216)%2)
      +8192*((i/67108864)%2)
      +16384*((i/268435456)%2)
      ;
      */
    uint32_t r = 0;
    for (int i = 0; i < 15; i++) {
        r += (1<<i) * ( (j/(1<<(i*2+0))) % 2 );
    };
    return r;
};

uint32_t conv_j_to_y(uint32_t j) {
    /*
    return
         1*((i/2)%2)
      +  2*((i/8)%2)
      +  4*((i/32)%2)
      +  8*((i/128)%2)
      + 16*((i/512)%2)
      + 32*((i/2048)%2)
      + 64*((i/8192)%2)
      +128*((i/32768)%2)

      ;
      */
    uint32_t r = 0;
    for (int i = 0; i < 15; i++) {
        r += (1<<i) * ( (j/(1<<(i*2+1))) % 2 );
    };
    return r;
};


typedef struct buf_t {
    unsigned char *data;
    uint64_t data_len;
    uint64_t max_data_len;
} buf_t;

void buf_init(buf_t *buf, unsigned int max_data_len) {
    buf->data_len = 0;
    buf->max_data_len = max_data_len;
    buf->data = (unsigned char*) malloc( max_data_len );
};

void buf_write(buf_t *buf, unsigned char *data, uint64_t data_len) {

    if ( buf->data_len + data_len >= buf->max_data_len ) {
        buf->max_data_len += (data_len & 0xFFF00000) + 0x00100000;
        buf->data = (unsigned char*) realloc( buf->data, buf->max_data_len );
    };
    memcpy( buf->data + buf->data_len, data, data_len );
    buf->data_len += data_len;

};

void fread_to_buf(buf_t *buf, FILE *fp, uint64_t data_len) {

    if ( buf->data_len + data_len >= buf->max_data_len ) {
        buf->max_data_len += ( (data_len) & 0xFFF00000) + 0x00100000;
        buf->data = (unsigned char*) realloc( buf->data, buf->max_data_len );
    };

    fread(buf->data + buf->data_len, data_len, 1, fp);
    buf->data_len += data_len;
};


int main(int argc, char ** argv) {




    char op = ' ';

    if (argc > 1) {
        op = argv[1][0];
    };

    if ( (argc < 9) || ( strchr("hgw", op) == NULL ) ) {
        fprintf( stderr, "Usage: geotilepacker <OP> pak_z pak_x pak_y subpak_z dest_z <heightmap_image_size/w_parts_count> planet_timestamp\n");
        fprintf( stderr, "Ops: \n");
        fprintf( stderr, "  h - heightmap (1 part) \n");
        fprintf( stderr, "  g - geodata (4 parts: b, a, l, p) \n");
        fprintf( stderr, "  w - generalized world \n");
        fprintf( stderr, "\n");
        return -1;
    };


    int pak_x, pak_y, pak_z;
    sscanf(argv[2], "%d", &pak_z);
    sscanf(argv[3], "%d", &pak_x);
    sscanf(argv[4], "%d", &pak_y);

    int subpak_z;
    sscanf(argv[5], "%d", &subpak_z);

    int dest_z;
    sscanf(argv[6], "%d", &dest_z);

    int subpak_z_diff = subpak_z - pak_z;
    int dest_z_diff = dest_z - subpak_z;

    if (subpak_z_diff < 0) {
        fprintf( stderr, "subpak_z must be higher than pak_z or equal to pak_z. \n");
        exit(-1);
    };

    if (dest_z_diff < 0) {
        fprintf( stderr, "dest_z must be higher than subpak_z or equal to subpak_z. \n");
        exit(-1);
    };

    int image_size = 0;

    if (op == 'h') {
        sscanf(argv[7], "%d", &image_size);

        if ( (image_size & (image_size - 1)) != 0 ) {
            fprintf( stderr, "Error, image_size must be power of 2.\nAborted.\n");
            exit(-1);
        };
    };

    image_size += 2 + 1;

    unsigned long long planet_timestamp = 0;

    sscanf(argv[8], "%llu", &planet_timestamp);


    int parts_count = 0; // heightmap, b, a, l, p

    if (op == 'w') {
        sscanf(argv[7], "%d", &parts_count);
        if ( (parts_count < 1) || (parts_count > 8) ) {
            fprintf( stderr, "parts_count must be in range [1..8]. Aborted. \n");
            exit(-1);
        };
    }
    else if (op == 'h') {
        parts_count = 1;
    }
    else if (op == 'g') {
        parts_count = 4;
    };




    buf_t buf;
    buf_init(&buf, 1024*1024);


    int subpak_side_count = 1 << subpak_z_diff;
    int subpak_total_count = subpak_side_count*subpak_side_count;

    if (subpak_total_count > 65536) {
        fprintf(stderr, "Too many subpaks (%d). Aborted. \n", subpak_total_count);
        exit(-1);
    };

    int tile_side_count_per_subpak = 1 << dest_z_diff;
    int tile_total_count_per_subpak = tile_side_count_per_subpak * tile_side_count_per_subpak;

    if (tile_side_count_per_subpak > 65536) {
        fprintf(stderr, "Too many tiles per subpak (%d). Aborted. \n", tile_side_count_per_subpak);
        exit(-1);
    };

    char part_c_arr[] = {'q', 'b', 'a', 'l', 'p', '0', '1', '2', '3', '4', '5', '6', '7'};
    char *part_c = part_c_arr;

    if (parts_count == 4) {
        part_c = &part_c_arr[1];
    };

    if (op == 'w') {
        part_c = &part_c_arr[5];
    };


    FILE * fp;
    char s[200];

    if (1) {

        char gpak_filename[200];
        sprintf(gpak_filename, "%d_%d_%d_%c.gpak", pak_z, pak_x, pak_y, op);

        fp = fopen(gpak_filename, "wb+");

        gpak_file_header_t hdr;
        memset(&hdr, 0, sizeof(struct gpak_file_header_t));

        hdr.magic_num0 = 0x4B415047; // 'GPAK' for little-endian
        hdr.magic_num1 = 0x00000000;
        hdr.version = 2;

        hdr.timestamp_pak_creation = planet_timestamp;
        hdr.timestamp_planet_osm = planet_timestamp;

        hdr.pak_x = pak_x;
        hdr.pak_y = pak_y;
        hdr.pak_z = pak_z;

        hdr.subpak_z = subpak_z;
        hdr.subpak_count = subpak_total_count;
        hdr.dest_z = dest_z;
        hdr.dest_count_per_subpak = tile_total_count_per_subpak;

        hdr.parts_count = parts_count;
        hdr.parts_mask = 0b00000001;    // h
        if (op == 'g') {
            hdr.parts_mask = 0b00011110;
        };
        if (op == 'w') {
            hdr.parts_mask = 0;
            for (int i = 0; i < parts_count; i++) {
                hdr.parts_mask |= (0b00100000 << i);
            };
        }; 
        hdr.hm_image_size = image_size;




        fwrite( &hdr, sizeof(struct gpak_file_header_t), 1, fp );

        subpak_map_record_t rec;
        memset(&rec, 0, sizeof(struct subpak_map_record_t) );

        for (int i = 0; i < subpak_total_count; i++) {
            fwrite( &rec, sizeof(struct subpak_map_record_t), 1, fp );
        };

        int subpak_start_x = pak_x * (1 << subpak_z_diff);
        int subpak_start_y = pak_y * (1 << subpak_z_diff);

        int subpak_bytes_shift = 0;

        int total_tiles_written = 0;
        int total_tiles_skipped = 0;

        int total_subpak_written = 0;
        int total_subpak_skipped = 0;

        uint64_t total_written_bytes = 0;
        uint64_t total_data_uncompressed_bytes = 0;

        int total_methods[2];
        total_methods[0] = 0;
        total_methods[1] = 0;


        for (int si = 0; si < subpak_total_count; si++) {

            int subpak_x = subpak_start_x + conv_j_to_x(si);
            int subpak_y = subpak_start_y + conv_j_to_y(si);

            int tile_start_x = subpak_x << dest_z_diff;
            int tile_start_y = subpak_y << dest_z_diff;

            int bytes_shift = 0;
            buf.data_len = 0;


            tile_map_record_t tilerec;
            memset(&tilerec, 0, sizeof(struct tile_map_record_t) );

            for (int i = 0; i < tile_total_count_per_subpak * parts_count; i++) {
                buf_write(&buf, (unsigned char*) (&tilerec), sizeof(struct tile_map_record_t));
            };


            for (int ti = 0; ti < tile_total_count_per_subpak; ti++) {

                for (int pi = 0; pi < parts_count; pi++) {

                    unsigned int current_shift = buf.data_len;

                    int tile_x = tile_start_x + conv_j_to_x(ti);
                    int tile_y = tile_start_y + conv_j_to_y(ti);

                    if (op == 'w') {
                        sprintf(s, "%d_%d_%d_w%c.data", dest_z, tile_x, tile_y, part_c[pi] );
                    }
                    else {
                        sprintf(s, "%d_%d_%d_%c.data", dest_z, tile_x, tile_y, part_c[pi] );
                    };

                    FILE *hfp = fopen(s, "rb");

                    if (!hfp) {
                        total_tiles_skipped++;
                        continue;
                    };

                    fseek(hfp, 0L, SEEK_END);
                    long int data_len = ftell(hfp);
                    fseek(hfp, 0L, SEEK_SET);

                    if (data_len < 3) {
                        // 2 bytes mean no data
                        fclose(hfp);
                        total_tiles_skipped++;
                        continue;
                    };

                    fread_to_buf(&buf, hfp, data_len);

                    fclose(hfp);

                    tile_map_record_t *current_rec = (tile_map_record_t*) (buf.data + sizeof(struct tile_map_record_t) * (ti*parts_count + pi));
                    current_rec->bytes_shift = current_shift;
                    current_rec->bytes_len = buf.data_len - current_shift;

                    total_tiles_written++;

                };


            };


            if ( buf.data_len >  sizeof(tile_map_record_t) * tile_total_count_per_subpak * parts_count ) {

                uint32_t compressed_len[2];
                char *compressed_data[2];
                int ci;

                // 1. zlib
                ci = 0;

                if (op == 'h') {

                    // don't use zlib for heightmap data
                    compressed_len[ci] = buf.data_len * 10;
                    compressed_data[ci] = (char*) malloc(4);

                }
                else {

                    uint64_t bound = compressBound(buf.data_len);

                    compressed_len[ci] = bound;
                    compressed_data[ci] = (char*) malloc(compressed_len[ci]);

                    int res = compress2( (unsigned char*)(compressed_data[ci]), &bound, (unsigned char*)buf.data, buf.data_len, 9);

                    if (res != Z_OK) {
                        printf("ERROR WHEN COMPRESING VIA ZLIB. \n");
                        exit(-1);
                    };

                    compressed_len[ci] = bound;

                };


                // 2. bzip2
                ci = 1;

                compressed_len[ci] = buf.data_len + buf.data_len/50 + 600;
                compressed_data[ci] = (char*) malloc(compressed_len[ci]);

                int blockSize100k = 6;
                int verbosity = 0;
                int workFactor = 0;

                BZ2_bzBuffToBuffCompress( compressed_data[ci], &(compressed_len[ci]), (char*)buf.data, buf.data_len, blockSize100k, verbosity, workFactor );


                // Compression done.

                if (compressed_len[0] < compressed_len[1] ) {
                    ci = 0;
                };

                fwrite( compressed_data[ci], compressed_len[ci], 1, fp);

                free(compressed_data[0]);
                free(compressed_data[1]);

                fseek(fp, sizeof(gpak_file_header_t) + sizeof(subpak_map_record_t)*si, SEEK_SET);

                subpak_map_record_t rec;

                rec.bytes_shift = subpak_bytes_shift;
                rec.bytes_len = compressed_len[ci];
                rec.uncompressed_bytes_len = buf.data_len;
                rec.compression_type = 1 + ci;
                rec.flags = 0;

                fwrite(&rec, sizeof(subpak_map_record_t), 1, fp );

                fseek(fp, 0, SEEK_END);

                subpak_bytes_shift += compressed_len[ci];
                total_subpak_written++;
                total_written_bytes += compressed_len[ci];
                total_data_uncompressed_bytes += buf.data_len;
                total_methods[ci]++;

            }
            else {

                //printf("Subpak skipped: %d/%d/%d\n", subpak_z, subpak_x, subpak_y );

                total_subpak_skipped++;


                fseek(fp, sizeof(gpak_file_header_t) + sizeof(subpak_map_record_t)*si, SEEK_SET);

                subpak_map_record_t rec;
                memset(&rec, 0, sizeof(subpak_map_record_t));

                rec.bytes_shift = subpak_bytes_shift;

                fwrite(&rec, sizeof(subpak_map_record_t), 1, fp );

                fseek(fp, 0, SEEK_END);


            };

        };

        fclose(fp);

        if (total_data_uncompressed_bytes != 0) {

            float megabytes_uncompressed = 1.0*total_data_uncompressed_bytes/1024.0/1024.0;
            float megabytes_compressed = 1.0*total_written_bytes/1024.0/1024.0;

            fprintf(stderr, "Done. \nSubpaks: total %d, written %d, skipped %d\nTiles (parts): total %d, written %d, skipped %d\n",
                subpak_total_count, total_subpak_written, total_subpak_skipped,
                parts_count*tile_total_count_per_subpak*subpak_total_count, total_tiles_written, total_tiles_skipped);
            fprintf(stderr, "ZLIB used %d times, BZIP2 used %d times \n", total_methods[0], total_methods[1]);
            fprintf(stderr, "Total compression ratio: %.1f\% (%.1f MB -> %.1f MB)\n", 100.0*total_written_bytes/total_data_uncompressed_bytes,
                megabytes_uncompressed, megabytes_compressed);
        }
        else {

            fprintf(stderr, "total_data_uncompressed_bytes is %ld. No data. Removing empty %s. \n", total_data_uncompressed_bytes, gpak_filename);

            sprintf(s, "rm %s", gpak_filename);

            system(s);

        };

    }
    else {

        // Wrong op

    };

    free(buf.data);


    return 0;
}

