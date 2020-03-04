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
#include <stdlib.h>

#include "csv.h"

#include <geos_c.h>

#include <stdarg.h>



void my_printf( const char* str, ... ) {
  va_list arg;
  va_start( arg, str );

  vprintf( str, arg);
  va_end( arg );

};

int rs_strstr( char *haystack, char *needle ) {
   char *s = strstr(haystack, needle);
   if (s == haystack) {
        return 1;
   };
   return 0;
};





using namespace std;
//using namespace geos;
//using namespace geos::geom;
////using namespace geos::operation::polygonize;
////using namespace geos::operation::linemerge;
//using geos::util::GEOSException;
//using geos::util::IllegalArgumentException;



// 20037508.342789

void degrees2proj(double lon, double lat) {
    double x = lon * 20037508.342789 / 180;
    double y = log(tan((90.0 + lat) * M_PI / 360)) / (M_PI / 180);
    y = y * 20037508.342789 / 180;

    //printf("%.9f, %.9f \n", x, y);

}

void tc2proj(int zoom_level, double x, double y, double *output_x, double *output_y) {

    // !!!!! можно сократить на PI, оставляю для наглядности, компилятор сократит
    double px = ((x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI) * 20037508.342789 / M_PI;
    double py = ( M_PI - y*2.0*M_PI/( pow(2.0, zoom_level) ) ) * 20037508.342789 / M_PI;

    *output_x = px;
    *output_y = py;

//    printf("(3857) %.12f, %.12f \n", px, py);

};


void proj2tc(int zoom_level, double x, double y, double *output_x, double *output_y) {


    double tx =  pow(2.0, zoom_level) * (  1.0 + x/20037508.342789 ) / 2.0;
    double ty = -pow(2.0, zoom_level) * ( -1.0 + y/20037508.342789 ) / 2.0;

    *output_x = tx;
    *output_y = ty;

//    printf("(3857) %.12f, %.12f \n", px, py);

};

void degrees2tc(int zoom_level, double lon, double lat, double *output_x, double *output_y) {

    double x;
    double y;

    lon *= M_PI/180.0;
    lat *= M_PI/180.0;

    x = 0.5 / M_PI * pow(2.0, zoom_level) * (M_PI + lon);
    y = 0.5 / M_PI * pow(2.0, zoom_level) * ( M_PI - log( tan(M_PI/4.0 + lat/2.0) ) );

    *output_x = x;
    *output_y = y;

    //cout << "x=" << x << ", y=" << y << endl;


};

void tc2degrees(int zoom_level, double x, double y, double *output_lon, double *output_lat) {

//    x *= 256.0;
//    y *= 256.0;

    double lon;
    double lat;

    //lon = (x * M_PI) / 128.0 / pow(2.0, zoom_level) - M_PI;

    lon = (x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI;
    lat = 2.0 * atan(exp(M_PI - y*2.0*M_PI/pow(2.0, zoom_level))) - M_PI/2.0;

    lon *= 180.0 / M_PI;
    lat *= 180.0 / M_PI;

    *output_lon = lon;
    *output_lat = lat;
    //printf("(deg.) %.12f, %.12f (lon, lat) \n", lon, lat);
};


int file_is_present(char *s) {

    FILE *fp = fopen(s, "r");
    if (!fp) {
        return 0;
    };
    fclose(fp);
    return 1;

};


//GeometryFactory *global_factory;

#define RVT_X(c)  ( 256.0f/63488.0*(-1024L + ( ((c)>>0)  & 0xFFFF))  )
#define RVT_Y(c)  ( 256.0f/63488.0*(-1024L + ( ((c)>>16) & 0xFFFF))  )
#define RVT_PACK_XY(x,y)   (  ((uint16_t)(1024.0+(x))) | (((uint32_t)(1024.0+(y)))<<16)  )

int src_z; // = 12;
int src_x; // = 2609;
int src_y; // = 1323;
int dest_z;

//GEOSGeometry *tile_g;


GEOSGeometry* create_tile_bbox(int tile_x, int tile_y) {

    int tile_z = dest_z;

    double lon1, lat1, lon2, lat2;

    // lon2>lon1, lat2>lat1
    tc2degrees( tile_z, tile_x, tile_y+1, &lon1, &lat1);
    tc2degrees( tile_z, tile_x+1, tile_y, &lon2, &lat2);

    GEOSCoordSequence *cs = GEOSCoordSeq_create(5, 2);

    GEOSCoordSeq_setX(cs, 0, lon1 );
    GEOSCoordSeq_setY(cs, 0, lat1 );

    GEOSCoordSeq_setX(cs, 1, lon2 );
    GEOSCoordSeq_setY(cs, 1, lat1 );

    GEOSCoordSeq_setX(cs, 2, lon2 );
    GEOSCoordSeq_setY(cs, 2, lat2 );

    GEOSCoordSeq_setX(cs, 3, lon1 );
    GEOSCoordSeq_setY(cs, 3, lat2 );

    GEOSCoordSeq_setX(cs, 4, lon1 );
    GEOSCoordSeq_setY(cs, 4, lat1 );


    return GEOSGeom_createLinearRing(cs);


};

#define ROW_WKT         0
#define ROW_OSM_ID      1
#define ROW_WAY_ID      2
#define ROW_NAME        3
#define ROW_TYPE        4
#define ROW_BUILDING    5
#define ROW_BUILDING_PART    6
#define ROW_ALLTAGS     7

////
////#define ROW_BUILDING_LEVELS     7
////#define ROW_BUILDING_MIN_LEVELS   8
////#define ROW_HEIGHT     9
////#define ROW_MIN_HEIGHT     10
////#define ROW_BUILDING_HEIGHT     11
////#define ROW_BUILDING_MIN_HEIGHT     12
////
////
////#define ROW_BUILDING_COLOUR       13
////#define ROW_BUILDING_MATERIAL     14
////#define ROW_ROOF_COLOUR           15
////#define ROW_ROOF_MATERIAL         16
////
////#define ROW_AEROWAY         17
////
////
////#define ROW_AMENITY     18
////#define ROW_BARRIER     20
////#define ROW_LANDUSE     26
////#define ROW_LEISURE     27
////#define ROW_MAN_MADE    28
////#define ROW_MILITARY    29
////#define ROW_NATURAL     30
////#define ROW_POWER       31
////#define ROW_SHOP        34
////#define ROW_WATERWAY    37
////#define ROW_RESIDENTIAL 38
////#define ROW_HOUSENUMBER 39
////#define ROW_INT_NAME    40
////#define ROW_NAME_EN     41
////#define ROW_NAME_RU     42
////#define ROW_NAME_DE     43
////#define ROW_NAME_FR     44
////#define ROW_REF         45
////#define ROW_LAYER       46
////#define ROW_COLOUR      47
////#define ROW_HIGHWAY     48
////#define ROW_SURFACE     49
////
////#define ROW_ROOF_HEIGHT 50
////#define ROW_ROOF_SHAPE  51
////#define ROW_ROOF_ORIENTATION 52
////#define ROW_ROOF_ANGLE 53
////#define ROW_ROOF_LEVELS 54
////#define ROW_ROOF_DIRECTION 55
////
////#define ROW_WATER 56
////


#define LROW_NAME        2
#define LROW_TYPE        3
#define LROW_ZORDER      4
#define LROW_ALLTAGS     5


////#define LROW_NAME       2
////#define LROW_HIGHWAY    3
////#define LROW_WATERWAY   4
////#define LROW_RAILWAY    5
////#define LROW_AERIALWAY  6
////#define LROW_BARRIER    7
////#define LROW_MAN_MADE   8
////#define LROW_POWER      9
////#define LROW_LANES      10
////#define LROW_ONEWAY     11
////#define LROW_BRIDGE     12
////#define LROW_TUNNEL     13
////#define LROW_WIDTH      14
////#define LROW_GAUGE      15
////#define LROW_ELECTRIFIED    16
////#define LROW_LIT        17
////#define LROW_SURFACE    18
////#define LROW_NATURAL    19
////#define LROW_INT_NAME    20
////#define LROW_NAME_EN     21
////#define LROW_NAME_RU     22
////#define LROW_NAME_DE     23
////#define LROW_NAME_FR     24
////#define LROW_REF         25
////#define LROW_LAYER       26
////
////

#define PROW_NAME        2
#define PROW_TYPE        3
#define PROW_ALLTAGS     4



////#define PROW_NAME       2
////#define PROW_BARRIER    3
////#define PROW_HIGHWAY    4
////#define PROW_REF        5
////#define PROW_PLACE      8
////#define PROW_MAN_MADE   9
////#define PROW_NATURAL    10
////#define PROW_POWER      11
////#define PROW_ENTRANCE   12
////#define PROW_EMERGENCY  13
////#define PROW_AMENITY    14
////#define PROW_RAILWAY    15
////#define PROW_PUBLIC_TRANSPORT   16
////#define PROW_SHOP       17
////#define PROW_FORD       18
////#define PROW_HISTORIC   19
////#define PROW_LEISURE    20
////#define PROW_DESIGN     21 // power=tower
////#define PROW_MATERIAL   22
////#define PROW_STRUCTURE  23
////#define PROW_CROSSING   24
////#define PROW_SHELTER    25 // bus_stop
////#define PROW_BENCH      26
////#define PROW_RELIGION   27
////#define PROW_LEAF_TYPE  28
////#define PROW_CIRCUMFERENCE  29
////#define PROW_TOURISM    30
////#define PROW_INT_NAME   31
////#define PROW_NAME_EN    32
////#define PROW_NAME_RU    33
////#define PROW_NAME_DE    34
////#define PROW_NAME_FR    35
////#define PROW_LAYER      36


//FILE **fps; // array of (FILE*)

typedef struct buf_t {
    unsigned char *data;
    unsigned long data_len;
    unsigned long max_data_len;
} buf_t;

buf_t *bufs;

//FILE *fp_current;
int current_tile_i;

void write_byte(unsigned char b) {
    int i = current_tile_i;
    //fwrite(&b, 1, 1, fp_current);
    if ( bufs[i].data_len + 1 >= bufs[i].max_data_len ) {
        bufs[i].max_data_len += 1024;
        bufs[i].data = (unsigned char*) realloc( bufs[i].data, bufs[i].max_data_len );
    };
    memcpy( bufs[i].data + bufs[i].data_len, &b, 1 );
    bufs[i].data_len += 1;
};

void write_us(unsigned short us) {
    int i = current_tile_i;
    //fwrite(&us, 2, 1, fp_current);
    if ( bufs[i].data_len + 2 >= bufs[i].max_data_len ) {
        bufs[i].max_data_len += 1024;
        bufs[i].data = (unsigned char*) realloc( bufs[i].data, bufs[i].max_data_len );
    };
    memcpy( bufs[i].data + bufs[i].data_len, &us, 2 );
    bufs[i].data_len += 2;
};

void write_int(unsigned int value) {
    int i = current_tile_i;
    //fwrite(&value, 4, 1, fp_current);
    if ( bufs[i].data_len + 4 >= bufs[i].max_data_len ) {
        bufs[i].max_data_len += 1024;
        bufs[i].data = (unsigned char*) realloc( bufs[i].data, bufs[i].max_data_len );
    };
    memcpy( bufs[i].data + bufs[i].data_len, &value, 4 );
    bufs[i].data_len += 4;
};

void write_float(float value) {
    int i = current_tile_i;
    //fwrite(&value, 4, 1, fp_current);
    if ( bufs[i].data_len + 4 >= bufs[i].max_data_len ) {
        bufs[i].max_data_len += 1024;
        bufs[i].data = (unsigned char*) realloc( bufs[i].data, bufs[i].max_data_len );
    };
    memcpy( bufs[i].data + bufs[i].data_len, &value, 4 );
    bufs[i].data_len += 4;
};

//void write_data(int i, void *pointer, int bytes_count) {
//    //fwrite(pointer, bytes_count, 1, fp_current);
//    @error warning! bytes_count can be > 1024
//    if ( bufs[i].data_len + bytes_count >= bufs[i].max_data_len ) {
//        bufs[i].max_data_len += 1024;
//        bufs[i].data = realloc( bufs[i].max_data_len );
//    };
//    memcpy( bufs[i].data + bufs[i].data_len, &b, 1 );
//    bufs[i].data_len += 1;
//};

void write_string( const char *s ) {
    // !!!! warning, strlen(s)+1 must be less than 1024.

    int i = current_tile_i;
    int s_len = strlen(s) + 1; // including terminating zero
    if ( bufs[i].data_len + s_len >= bufs[i].max_data_len ) {
        bufs[i].max_data_len += 1024;
        bufs[i].data = (unsigned char*) realloc( bufs[i].data, bufs[i].max_data_len );
    };
    memcpy( bufs[i].data + bufs[i].data_len, s, s_len );
    bufs[i].data_len += s_len;
};



//#include "../streets/src/osmdefines.h"


//////#define OSM_BARRIER     0x10
//////
//////#define BARRIER_FENCE   1
//////#define BARRIER_WALL    2
//////#define BARRIER_HEDGE   3
//////#define BARRIER_BOLLARD 4
//////#define BARRIER_DITCH   5
//////#define BARRIER_CITY_WALL   6



//GEOSGeometry* GEOSClipByRect(GEOSGeometry *gout, double lon1, double lat1, double lon2, double lat2) {
//    return GEOSGeom_clone(gout);
//    //return GEOSGeom(gout);
//    //return gout;
//};


#include "hstore_parser.hpp"


int need_float_coordinates(GEOSGeometry *g_obj, int num_geometries, int tile_z, int tile_x, int tile_y) {

//    return 1;

    for (int i = 0; i < num_geometries; i++) {

        int is_polygon = ( GEOSGeomTypeId(g_obj) == GEOS_MULTIPOLYGON ) || ( GEOSGeomTypeId(g_obj) == GEOS_POLYGON );

        const GEOSGeometry *g_current = GEOSGetGeometryN(g_obj, i);

        int rings_count = (is_polygon) ? (1 + GEOSGetNumInteriorRings(g_current)) : 1;

        for (int r = 0; r < rings_count; r++) {

            const GEOSGeometry *g2 =
                (is_polygon) ?
                    (
                        (r == 0) ? (GEOSGetExteriorRing( g_current )) : (GEOSGetInteriorRingN(g_current, r-1))
                    )
                    :
                    ( g_current );


            int num_points = GEOSGeomGetNumPoints(g2);


            for (int j = 0; j < num_points; j ++ ) {

                GEOSGeometry *gp = GEOSGeomGetPointN(g2, j);
                double x, y;
                GEOSGeomGetX(gp, &x); // lon
                GEOSGeomGetY(gp, &y); // lat

                GEOSGeom_destroy(gp);

                degrees2tc(tile_z, x, y, &x, &y);

                x -= tile_x;
                y -= tile_y;

                if ( (x < -0.1) || (y < -0.1) ) {
                    return 1;
                };

                if ( (x > 1.1) || (y > 1.1) ) {
                    return 1;
                };


//                if ( op == 'b' ) {
//                    double ox = 0.25 + 0.5*icx;
//                    double oy = 0.25 + 0.5*icy;
//
//                    ox -= 0.5;
//                    oy -= 0.5;
//
//                    x -= ox;
//                    y -= oy;
//                };


            };

        };



    };

    return 0;

};



int can_use_three_byte_coordinates(GEOSGeometry *g_obj, int num_geometries, int tile_z, int tile_x, int tile_y, int *three_byte_coordinates_byte_ref) {


    float minx = 1.2;
    float miny = 1.2;
    float maxx = -0.2;
    float maxy = -0.2;

    float c_threshold = 1.0/16.0 - 2.0/256.0;


    for (int i = 0; i < num_geometries; i++) {

        int is_polygon = ( GEOSGeomTypeId(g_obj) == GEOS_MULTIPOLYGON ) || ( GEOSGeomTypeId(g_obj) == GEOS_POLYGON );

        const GEOSGeometry *g_current = GEOSGetGeometryN(g_obj, i);

        int rings_count = (is_polygon) ? (1 + GEOSGetNumInteriorRings(g_current)) : 1;

        for (int r = 0; r < rings_count; r++) {

            const GEOSGeometry *g2 =
                (is_polygon) ?
                    (
                        (r == 0) ? (GEOSGetExteriorRing( g_current )) : (GEOSGetInteriorRingN(g_current, r-1))
                    )
                    :
                    ( g_current );

            int num_points = GEOSGeomGetNumPoints(g2);


            for (int j = 0; j < num_points; j ++ ) {

                GEOSGeometry *gp = GEOSGeomGetPointN(g2, j);
                double x, y;
                GEOSGeomGetX(gp, &x); // lon
                GEOSGeomGetY(gp, &y); // lat

                GEOSGeom_destroy(gp);

                degrees2tc(tile_z, x, y, &x, &y);

                x -= tile_x;
                y -= tile_y;

                if ( (x < 0.0001) || (y < 0.0001) ) {
                    return 0;
                };

                if ( (x > 0.9999) || (y > 0.9999) ) {
                    return 0;
                };

                minx = (x < minx) ? x : minx;
                miny = (y < miny) ? y : miny;
                maxx = (x > maxx) ? x : maxx;
                maxy = (y > maxy) ? y : maxy;

                if ( (maxx-minx) > c_threshold ) {
                    return 0;
                };
                if ( (maxy-miny) > c_threshold ) {
                    return 0;
                };


            };

        };

    };


    int refx = (int) (256.0*minx);
    int refy = (int) (256.0*miny);
    *three_byte_coordinates_byte_ref = (refy << 8) | refx;

    return 1;

};

uint32_t pack_three_byte_coord(double x, double y, int b_ref) {

    double sc = 65536.0;

    x *= sc;
    y *= sc;

    double xshift = sc / 256.0 * ((b_ref>>0) & 0xFF);
    double yshift = sc / 256.0 * ((b_ref>>8) & 0xFF);

    x -= xshift;
    y -= yshift;

    int ix = (int)x;
    int iy = (int)y;

    if (ix < 0) {
        ix = 0;
    };
    if (iy < 0) {
        iy = 0;
    };
    if (ix > 4095) {
        ix = 4095;
    };
    if (iy > 4095) {
        iy = 4095;
    };

    uint32_t r = ( iy << 12 ) | ix;

    return r;

};




int main(int argc, char ** argv) {



    // Simplification test
//    initGEOS(&my_printf, &my_printf);
//    GEOSGeometry *g = GEOSGeomFromWKT("POLYGON ((40 10, 40 40, 10 40, 11 25, 10 10, 40 10))");
//    GEOSGeometry *g_simplified = GEOSSimplify(g, 1.1);
//    printf("%s\n\n%s\n", GEOSGeomToWKT(g), GEOSGeomToWKT(g_simplified));
//    finishGEOS();
//    return 0;



    // Little Endian: true
//    unsigned short test_us = 0;
//    unsigned char test_char = 1;
//
//    memcpy(&test_us, &test_char, 1);
//    printf("result is %d\n", test_us); // 1
//    exit(0);
//    return 0;




    char op = '?';

    if (argc > 1) {
        op = argv[1][0];
    };


    if ( (argc < 7) || ( strchr("abpl", op) == NULL ) ) {
        printf("Usage: geocsv2db <OP> z_scale tile_x tile_y dest_z csv_filename [ocean_csv_filename | ignored_ids_list_filename |, ...] \n");
        printf("Ops: \n");
        printf("  b - buildings\n");
        printf("  a - areas (landuse, natural, etc.)\n");
        printf("  l - lines (roads, railways, waterways, powerlines, pipelines...)\n");
        printf("  p - points (towers, trees, bus stops, barrier=gates...)\n");
        printf("\n");
        return -1;
    };


    initGEOS(&my_printf, &my_printf);


    GEOSWKTReader *r = GEOSWKTReader_create();



//    int tile_x, tile_y, z_scale;
//
    sscanf(argv[2], "%d", &src_z);
    sscanf(argv[3], "%d", &src_x);
    sscanf(argv[4], "%d", &src_y);
    sscanf(argv[5], "%d", &dest_z);

    char *csv_filename = argv[6];

    int ocean = 0;
    char *csv_filename_ocean;

    if (op == 'a') {
//        if (argc < 8) {
//            printf("Please specify ocean CSV for op=='a'. \n");
//            return -1;
//        };

        if (argc > 7) {
            csv_filename_ocean = argv[7];
            ocean = 1;
        };

    };


    int ignored_ids = 0;
    char *ignored_ids_filename;
    if (op == 'b') {
        if (argc < 8) {
//            printf("Ignored ids list filename is not specified. Ignored. \n");
        }
        else {
            ignored_ids_filename = argv[7];
            ignored_ids = 1;
        };
    };


    unsigned long long *ign_ids_array = NULL;
    uint32_t *ign_types_array = NULL;
    int ign_ids_array_count = 0;
    int ign_ids_array_max_count = 0;

    char ign_ids_s[500];

    if (ignored_ids) {

        FILE *ifp = fopen(ignored_ids_filename, "r");
        if (!ifp) {
            printf("Can't open %s. Not using ignored ids list. \n", ignored_ids_filename );
            ignored_ids = 0;
        }
        else {

            ign_ids_array_max_count = 4096;
            ign_ids_array = (unsigned long long*) malloc( sizeof(unsigned long long) * ign_ids_array_max_count );
            ign_types_array = (uint32_t*) malloc( sizeof(uint32_t) * ign_ids_array_max_count );

            while (!feof(ifp)) {

                ign_ids_s[0] = 0;

                char *cr = fgets( (char*)ign_ids_s, 500, ifp);

                if (cr == NULL) {
                    break;
                };

                int res1 = 0;

//                printf("-a-\n");

                unsigned int current_type;
                unsigned long long current_id;

                res1 = sscanf(ign_ids_s, "%d;%llu", &current_type, &current_id);

//                printf("-b-\n");

                if (res1 != 2) {
                    //printf("Error while reading %s, res != 2, probably corrupted file. \n", ignored_ids_filename );
                    // or just EOF or something
                    break;
                };

//                if (current_id == 1) {  // way
//                    current_id = 0x8000000000000000LL | current_id;
//                };

//                printf("-c-\n");

                if (ign_ids_array_count == ign_ids_array_max_count) {
                    ign_ids_array_max_count += 4096;
                    ign_ids_array = (unsigned long long*) realloc(ign_ids_array, sizeof(unsigned long long) * ign_ids_array_max_count );
                    ign_types_array = (uint32_t*) realloc( ign_types_array, sizeof(uint32_t) * ign_ids_array_max_count );
                };

//                printf("-d-\n");

                ign_ids_array[ign_ids_array_count] = current_id;
                ign_types_array[ign_ids_array_count] = current_type;
                ign_ids_array_count++;

//                printf("%llu -e-\n\n", current_id );

            };

            fclose(ifp);


        };

    };




    int z_diff = dest_z - src_z;

    if (z_diff < 0) {
        fprintf( stderr, "dest_z must be higher than z_scale or equal to z_scale. \n");
        exit(-1);
    };

    int t_side_count = 1 << z_diff;
    int t_total_count = t_side_count*t_side_count;

    if (t_total_count > 16384) {
        fprintf(stderr, "Too many tiles (%d). Aborted. \n");
        exit(-1);
    }

    int t_start_x = src_x * (1 << z_diff);
    int t_start_y = src_y * (1 << z_diff);

//    for (int i = 0; i < t_side_count; i++) {
//        for (int j = 0; j < t_side_count; j++) {
//               printf("%d_%d\n", t_start_x + i, t_start_y + j);
//        };
//    };


    for (int i = 0; i < t_total_count; i++) {
        //fps = (FILE**) malloc( sizeof(FILE*) * t_total_count );
        bufs = (buf_t*) malloc( sizeof(buf_t) * t_total_count );
    };




//    tile_g = create_tile_bbox();


//////    int key = ROW_BUILDING;




    FILE *in;
    int buf_size = 24*1024*1024;
    char *buf = (char*) malloc(buf_size);
    char *row[300];
    int n;

    in = fopen(csv_filename, "r");
    if (!in) {
        printf("Can't open %s. Aborted.\n", csv_filename);
        return -1;
    };

    char filename [200];


    FILE *in_ocean;
    if (ocean) {
        in_ocean = fopen(csv_filename_ocean, "r");
        if (!in_ocean) {
            printf("Can't open ocean csv (%s). Ocean is ignored.\n", csv_filename_ocean );
            ocean = 0;
        };
    };


    unsigned short *total_counts = (unsigned short*) malloc( sizeof(unsigned short) * t_total_count );

    for (int i = 0; i < t_side_count; i++) {
        for (int j = 0; j < t_side_count; j++) {
            //printf("%d_%d\n", t_start_x + i, t_start_y + j);
            int tile_x = t_start_x + j;
            int tile_y = t_start_y + i;
            int tile_i = i*t_side_count + j;
            //sprintf(filename, "%d_%d_%d_%c.data", dest_z, tile_x, tile_y, op);

            //fps[tile_i] = fopen(filename, "wb");
            //fp_current = fps[tile_i];
            bufs[tile_i].data = (unsigned char*) malloc(4096);
            bufs[tile_i].data_len = 0;
            bufs[tile_i].max_data_len = 4096;

            //fprintf(stderr, "(%d) fp_current = %08x\n", tile_i, fp_current);

            current_tile_i = tile_i;
            write_us(0); // objects num, will be updated later
            total_counts[tile_i] = 0;
        };
    };


    unsigned short us = 0;

    int c;
    int ni;

    // ================= 1. OSM Data ==============



    c = 0;
    ni = 0;
    while ((n = csv_row_fread(in, (unsigned char*) buf, buf_size, (unsigned char**) row, 300, ',', CSV_TRIM | CSV_QUOTES)) > 3) { // at least 3 bytes


        // First line
        if (c == 0) {
            c = 1;
            continue;
        };



        unsigned long long ids[2];
        ids[0] = 0; // for way
        ids[1] = 0; // for relation
        int res1 = sscanf( (char*) row[ROW_WAY_ID], "%llu", &(ids[0]) );
        int res2 = sscanf( (char*) row[ROW_OSM_ID], "%llu", &(ids[1]) );

        int ign_found = 0;

        if (ignored_ids) {

            if (res1 && res2) {

                // Checking if osm id or way id is in ignore list
                for (int ii = 0; ii < ign_ids_array_count; ii++) {

                    // 1 = way (way_id), 2 = relation (osm_id)
                    int id_row_i = ( ign_types_array[ii] - 1 );
                    id_row_i %= 2; // protection from getting out of bounds

                    if (ign_ids_array[ii] == ids[ id_row_i ] ) {
                        printf("MATCH! (%d) %lld \n", id_row_i+1, ids[ id_row_i ] );
                        ign_found = 1;
                        break;
                    };

                };

            };
        };

////        if (ign_found) {
////            continue;
////            // !!!! TODO: better save the object but set "outline" tag
////        };



//        printf("reading: [%s]\n", row[0]);

//        printf("=== %d === (%d) \n", ni, n );
//        ni++;
//
//        for (int i = 1; i < 6; i++) {
//            printf("%d) %s\n", i, row[i] );
//        };
//        printf("\n");
//        continue;


        GEOSGeometry *gout = GEOSWKTReader_read(r, (const char*) row[0]); // GEOSGeomFromWKT(wkt);

//        if ( (op == 'b') && (row[ROW_BUILDING][0]) ) {
//            printf("Got building=%s: \n%s\n\n", row[ROW_BUILDING], row[0] );
//            continue;
//        };




        if (!gout) {
            printf("Bad geometry (id: %s) (way_id: %s). Skipped. \n", row[ROW_OSM_ID], row[ROW_WAY_ID] );
            continue;
        };

        if ( ((op == 'b') || (op == 'a')) && (GEOSGeomTypeId(gout) != GEOS_MULTIPOLYGON) ) {
            printf("Wrong geom type (gout) (%s instead of multipolygon). Skipped. \n", GEOSGeomType(gout));
            GEOSGeom_destroy(gout);
            continue;
        };

        if ( ((op == 'l')) && (GEOSGeomTypeId(gout) != GEOS_LINESTRING) ) {
            printf("Wrong geom type for L (gout) (%s instead of linestring). Skipped. \n", GEOSGeomType(gout));
            printf("%s\n\n", row[0] );
            GEOSGeom_destroy(gout);
            continue;
        };

        if ( (op == 'p') && (GEOSGeomTypeId(gout) != GEOS_POINT)  ) {
            printf("Wrong geom type for P (gout) (%s instead of point). Skipped. \n", GEOSGeomType(gout));
            GEOSGeom_destroy(gout);
            continue;
        };


        int g_obj_created = 0;

        GEOSGeometry *g_obj;


        for (int ti = 0; ti < t_side_count; ti++) {
            for (int tj = 0; tj < t_side_count; tj++) {

//                printf("%d/%d:\n", ti, tj);

                if (g_obj_created) {
                    GEOSGeom_destroy(g_obj);
                };
                g_obj_created = 0;

                g_obj = gout;


                int tile_z = dest_z;
                int tile_x = t_start_x + tj;
                int tile_y = t_start_y + ti;
                int tile_i = ti*t_side_count + tj;

                //fp_current = fps[tile_i];
                current_tile_i = tile_i;


                double clip_lon1, clip_lat1, clip_lon2, clip_lat2;
    //             lon2>lon1, lat2>lat1
                tc2degrees( tile_z, tile_x, tile_y+1, &clip_lon1, &clip_lat1);
                tc2degrees( tile_z, tile_x+1, tile_y, &clip_lon2, &clip_lat2);



                if (op != 'b') {
                    if (op != 'p') {
                        g_obj = GEOSClipByRect(gout, clip_lon1, clip_lat1, clip_lon2, clip_lat2);

                        if (!g_obj) {

//                            printf("mark 1 a1\n");

                            continue;

//                            fprintf(stderr, "\n\ng_obj == 0\n");
//
//                            GEOSWKTWriter *wr = GEOSWKTWriter_create();
//                            printf("g_out: [%s]\n", GEOSWKTWriter_write(wr, gout) );
//                            GEOSWKTWriter_destroy(wr);
//
//                            exit(-1);
                        };

//                        printf("mark 1 a2\n");

                        g_obj_created = 1;

//////                        if ( GEOSGeomTypeId(g_obj) != GEOS_POINT ) {
//////                            printf("mark 1 a3 <============== \n");
//////                            continue;
//////                        };


                    };

//                    printf("mark 2\n");

                    if (op == 'l') {

                        if ( (GEOSGeomTypeId(g_obj) == GEOS_GEOMETRYCOLLECTION) && (GEOSGetNumGeometries(g_obj)==0) ) {

//                                printf("('a') Geometry collection. num = %d\n", GEOSGetNumGeometries(g_obj) );
                                continue;

                        }

                        else if ( (GEOSGeomTypeId(g_obj) != GEOS_MULTILINESTRING) && (GEOSGeomTypeId(g_obj) != GEOS_LINESTRING) ) {
                            printf("Wrong geom type (g_obj) (%s instead of multi-/linestring). Skipped. \n", GEOSGeomType(g_obj));
//                            printf("buf: [%s]\n", buf);

//                            GEOSWKTWriter *wr = GEOSWKTWriter_create();
//                            printf("g_obj: [%s]\n", GEOSWKTWriter_write(wr, g_obj) );
//                            GEOSWKTWriter_destroy(wr);

                            //GEOSGeom_destroy(gout);
                            GEOSGeom_destroy(g_obj);
                            continue;
                        };
                    }
                    else if (op == 'a') {

//                        printf("\n[%s]\n", buf);

//                        GEOSWKTWriter *wr = GEOSWKTWriter_create();
//                        printf("g_obj: [%s]\n", GEOSWKTWriter_write(wr, g_obj) );
//                        GEOSWKTWriter_destroy(wr);

//                        printf("g_obj = %016x, calling GEOSGeomTypeId()... \n", g_obj);

                        if ( (GEOSGeomTypeId(g_obj) == GEOS_GEOMETRYCOLLECTION) && (GEOSGetNumGeometries(g_obj)==0) ) {

//                            printf("('a') Geometry collection. num = %d, tile: %d/%d/%d\n", GEOSGetNumGeometries(g_obj), tile_z, tile_x, tile_y );
//                            printf("");
//                            GEOSGeom_destroy(g_obj);
                            continue;

                        }
                        else if ( (GEOSGeomTypeId(g_obj) != GEOS_MULTIPOLYGON) && (GEOSGeomTypeId(g_obj) != GEOS_POLYGON) ) {

                            printf("('a') Wrong geom type (g_obj) (%s instead of multi-/polygon). Skipped. tile: %d/%d/%d \n", GEOSGeomType(g_obj),  tile_z, tile_x, tile_y);
//                            printf("buf: [%s]\n", buf);
//
//                            GEOSWKTWriter *wr = GEOSWKTWriter_create();
//
//                            printf("g_obj: [%s]\n", GEOSWKTWriter_write(wr, g_obj) );
//
//                            GEOSWKTWriter_destroy(wr);

                            //GEOSGeom_destroy(gout);
//                            GEOSGeom_destroy(g_obj);
                            continue;
                        };



//                        printf("passed...\n");

                    };
                };

//                printf("mark 5\n");


                int num_geometries = GEOSGetNumGeometries(g_obj);

               //// !!!! use this to check for bad geometry types:
               //// printf("(%d/%d) '%c' %s (num %d). \n", tile_x, tile_y, c, GEOSGeomType(g_obj), num_geometries );

//                if (op == 'b') {
//                   //if ( (tile_x == 10398) && (tile_y == 5334) ) {
//                   if ( (tile_x == 9758) && (tile_y == 5914) ) {       // Crimea Alushta
//
//                        if (g_obj) {
//                            //printf("%02x [%s] (area) landuse[%s] natural[%s] amenity[%s] \n", b1, row[ROW_WATERWAY], row[ROW_LANDUSE], row[ROW_NATURAL], row[ROW_AMENITY]);
//                            printf("num_geometries: %d\nParent geometry:\n %s\n\nCropped geometry:\n%s\n\n", num_geometries, row[0], GEOSGeomToWKT(g_obj) );
//
////                            GEOSGeometry *tile_bbox = create_tile_bbox(tile_x, tile_y);
////                            printf("Tile bbox: %s\n", GEOSGeomToWKT(tile_bbox));
////                            GEOSGeom_destroy(tile_bbox);
//
//                        }
//                        else {
//                            printf("\n\nNO g_obj \n\n\n");
//                        };
//
//                    };
//                };


                if ( (op == 'p') && (num_geometries != 1) ) {
                    printf("Wrong num_geometries for point (%d instead of 1). Skipped. \n", num_geometries);
                    printf("type of g_obj is %s\n", GEOSGeomType(g_obj));
                    printf("buf = [%s]\n", buf);
                    printf("%d %d %d\n", tile_z, tile_x, tile_y);
                    printf("\n");

                    continue;
                }
                else if (num_geometries == 0) {

                    continue;

                };

                //printf("num_geometries = %d for geom type %d \n", num_geometries, GEOSGeomTypeId(g_obj) );


//                printf("trying... ");

        //        printf("%d polygons: \n", num_geometries);


                GEOSGeometry *g_envelope;
                const GEOSGeometry *g_envelope_exterior_ring;
                double ex1, ey1, ex2, ey2;
                double cx;
                double cy;
                int icx = 0;
                int icy = 0;
                uint32_t point_packed_coord = 0;


                if ( op != 'p' ) {
                    g_envelope = GEOSEnvelope( g_obj );
                    g_envelope_exterior_ring = GEOSGetExteriorRing( g_envelope );
            //        printf("envelope: %d points\n", GEOSGeomGetNumPoints(g_envelope));

                    GEOSGeometry *g_point0 = GEOSGeomGetPointN(g_envelope_exterior_ring, 0);
                    GEOSGeometry *g_point2 = GEOSGeomGetPointN(g_envelope_exterior_ring, 2);

                    GEOSGeomGetX( g_point0, &ex1);
                    GEOSGeomGetY( g_point0, &ey1);
                    GEOSGeomGetX( g_point2, &ex2);
                    GEOSGeomGetY( g_point2, &ey2);

                    GEOSGeom_destroy(g_point0);
                    GEOSGeom_destroy(g_point2);
                    GEOSGeom_destroy(g_envelope); // ??????????? !!!!!!!!!!!!!

                    degrees2tc(tile_z, ex1, ey1, &ex1, &ey1);
                    degrees2tc(tile_z, ex2, ey2, &ex2, &ey2);

                    // ey1 > ey2, but ex2 > ex1

                    // if ( (ey1 > limit) or (ey2 < limit) or (ex2 > limit) or (ex1 < limit) )
                    // then we can't use simple ring

                    cx = 0.5*(ex1+ex2);
                    cy = 0.5*(ey1+ey2);

                    cx -= tile_x;
                    cy -= tile_y;

            //        cx = min(cx, 0.9999);
            //        cx = max(cx, 0.0);
            //        cy = min(cy, 0.9999);
            //        cy = max(cy, 0.0);

                    icx = (int) (2.0*cx);
                    icy = (int) (2.0*cy);

            //        printf("%.4f, %.4f | %d %d \n", cx, cy, icx, icy);

                }
                else {


                    const GEOSGeometry *gp = g_obj; // GEOSGeomGetPointN(g2, j);
                    double x, y;
                    GEOSGeomGetX(gp, &x); // lon
                    GEOSGeomGetY(gp, &y); // lat

                    degrees2tc(tile_z, x, y, &x, &y);

                    x -= tile_x;
                    y -= tile_y;

                    if (  (x >= 1.0) || (y >= 1.0) || (x < 0.0) || (y < 0.0) ) {
                        // Outside of our tile
                        //printf("outside. %.3f, %.3f\n", cx, cy);
                        continue;
                    };

                    double scale = (63488.0);
                    x *= scale;
                    y *= scale;

        //            printf("(%.4f, %.4f) \n", x, y);

                    point_packed_coord = RVT_PACK_XY(x, y);


                };



                if ( (op == 'b') && ( (row[ROW_BUILDING][0]) || (row[ROW_BUILDING_PART][0]) )  ) {

//                    printf("building: [%s]\n", row[ROW_BUILDING] );

//////                   if ( (tile_x == 9758) && (tile_y == 5914) ) {       // Crimea Alushta
//
//                        if (g_obj) {
//                            printf("building=%s\nnum_geometries: %d\nParent geometry:\n %s\n\nCropped geometry:\n%s\n\n", row[ROW_BUILDING], num_geometries, row[0], GEOSGeomToWKT(g_obj) );
//
////                            GEOSGeometry *tile_bbox = create_tile_bbox(tile_x, tile_y);
////                            printf("Tile bbox: %s\n", GEOSGeomToWKT(tile_bbox));
////                            GEOSGeom_destroy(tile_bbox);
//
//                        }
//                        else {
//                            printf("\n\nNO g_obj \n\n\n");
//                        };
//
//////                    };



//                    char *building = row[ROW_BUILDING];
//                    char *building_part = row[ROW_BUILDING_PART];
//                    char *building_levels = row[ROW_BUILDING_LEVELS];
//                    char *amenity = row[ROW_AMENITY];
//                    char *shop = row[ROW_SHOP];
//                    char *housenumber = row[ROW_HOUSENUMBER];


                    if (  (cx >= 1.0) || (cy >= 1.0) || (cx < 0.0) || (cy < 0.0) ) {
                        // Outside of our tile
//                        printf("outside. %.3f, %.3f\n", cx, cy);
                        continue;
                    };


////                    int t_index[] = { ROW_BUILDING, ROW_BUILDING_PART, 0 };
////                    char* t_key_string[] = { "building", "building:part" };
////
////
                    int tags_count_byte_index = bufs[current_tile_i].data_len;
                    write_byte(0); // tags count, will be updated later
                    uint8_t tags_count_byte = 0;
////
////                    int i = 0;
////                    while (t_index[i] != 0) {
////                        if (row[ t_index[i] ][0]) {
////                            tags_count_byte++;
////                            write_byte(0);
////                            write_string( t_key_string[i] );
////                            write_byte(0);
////                            write_string(row[ t_index[i] ]);
////                        };
////                        i++;
////                    };

                    if (ign_found) {
                        // Add tag
                        tags_count_byte++;
                        write_byte(0);
                        write_string("RVT_RELATION_ROLE");
                        write_byte(0);
                        write_string("outline");
                    };

                    if (row[ROW_ALLTAGS][0]) {

                        //std::string alltags = alltags;
                        std::string alltags_str = row[ROW_ALLTAGS];
                        pg_array_hstore_parser::HStoreParser hstore (alltags_str);
                        while (hstore.has_next()) {
                            pg_array_hstore_parser::StringPair kv_pair = hstore.get_next();
////                            std::cout << "[" << kv_pair.first << "]->[" << kv_pair.second << "]\n";

                            tags_count_byte++;

                            const char *s_key = kv_pair.first.c_str();
                            const char *s_value = kv_pair.second.c_str();

////                            if ( (!strcmp(s_key, "type")) && (!strcmp(s_value, "multipolygon")) ) {
////                                write_byte(1);
////                            }
////                            else if ( (!strcmp(s_key, "building")) && (!strcmp(s_value, "residential")) ) {
////                                write_byte(2);
////                            }
////                            else if ( (!strcmp(s_key, "building")) && (!strcmp(s_value, "apartments")) ) {
////                                write_byte(3);
////                            }
////                            else if ( (!strcmp(s_key, "highway")) && (!strcmp(s_value, "residential")) ) {
////                                write_byte(4);
////                            }
////                            else if ( (!strcmp(s_key, "building")) && (!strcmp(s_value, "house")) ) {
////                                write_byte(5);
////                            }
////                            else {

//                                write_byte(5); // !!! FOR TEST

                                write_byte(0);
                                write_string( s_key );
                                write_byte(0);
                                write_string( s_value );

////                            };

                        };
                    };



                    memcpy( bufs[current_tile_i].data + tags_count_byte_index, &tags_count_byte, 1 );




                }
                else if (op == 'a') {
                    // Area




//                    char *landuse = row[ROW_LANDUSE];
//                    char *natural = row[ROW_NATURAL];
//                    char *amenity = row[ROW_AMENITY];
//                    char *barrier = row[ROW_BARRIER];
//                    char *leisure = row[ROW_LEISURE];
//                    char *power = row[ROW_POWER];
//                    char *waterway = row[ROW_WATERWAY];
//
//                    if ( ! (landuse[0] || natural[0] || amenity[0] || leisure[0] || barrier[0] || power[0] || waterway[0] ) ) {
//                        continue;
//                    };


//                    int t_index[] = { ROW_BUILDING, ROW_BUILDING_PART, 0 };
//                    char* t_key_string[] = { "building", "building:part" };


                    int tags_count_byte_index = bufs[current_tile_i].data_len;
                    write_byte(0); // tags count, will be updated later
                    uint8_t tags_count_byte = 0;

//                    int i = 0;
//                    while (t_index[i] != 0) {
//                        if (row[ t_index[i] ][0]) {
//                            tags_count_byte++;
//                            write_byte(0);
//                            write_string( t_key_string[i] );
//                            write_byte(0);
//                            write_string(row[ t_index[i] ]);
//                        };
//                        i++;
//                    };

                    if (row[ROW_ALLTAGS][0]) {
                        //std::string alltags = alltags;
                        std::string alltags_str = row[ROW_ALLTAGS];
                        pg_array_hstore_parser::HStoreParser hstore (alltags_str);
                        while (hstore.has_next()) {
                            pg_array_hstore_parser::StringPair kv_pair = hstore.get_next();
//                            std::cout << "[" << kv_pair.first << "]->[" << kv_pair.second << "]\n";

                            tags_count_byte++;

//                            write_byte(5); // !!! FOR TEST

                            write_byte(0);
                            write_string( kv_pair.first.c_str() );
                            write_byte(0);
                            write_string( kv_pair.second.c_str() );

                        };
                    };

                    memcpy( bufs[current_tile_i].data + tags_count_byte_index, &tags_count_byte, 1 );






                }
                else if (op == 'p') {

//                    char *natural = row[PROW_NATURAL];
//                    char *power = row[PROW_POWER];
//                    char *highway = row[PROW_HIGHWAY];
//                    char *railway = row[PROW_RAILWAY];
//                    char *barrier = row[PROW_BARRIER];
//                    char *entrance = row[PROW_ENTRANCE];

//                    if ( ! (natural[0] || power[0] || highway[0] || barrier[0] || entrance[0]) ) {
//                        continue;
//                    };


//                    int t_index[] = { ROW_NAME, ROW_BUILDING, ROW_BUILDING_PART, 0 };
//                    char* t_key_string[] = { "barrier", "highway", "man_made", "natural", "power", "entrance", "emergency", "amenity",
//                                                "railway",
//                                                "public_transport", "shop", "ford", "historic", "leisure", "design", "material",
//                                                "structure", "crossing", "shelter", "bench", "religion", "leaf_type", "circumference",
//                                                "tourism", "ref", "place",
//                                            "name", "int_name", "name:en", "name:ru", "name:de", "name:fr", "layer"};


                    int tags_count_byte_index = bufs[current_tile_i].data_len;
                    write_byte(0); // tags count, will be updated later
                    uint8_t tags_count_byte = 0;

//                    int i = 0;
//                    while (t_index[i] != 0) {
//                        if (row[ t_index[i] ][0]) {
//                            tags_count_byte++;
//                            write_byte(0);
//                            write_string( t_key_string[i] );
//                            write_byte(0);
//                            write_string(row[ t_index[i] ]);
//                        };
//                        i++;
//                    };

                    if (row[PROW_ALLTAGS][0]) {
                        //std::string alltags = alltags;
                        std::string alltags_str = row[PROW_ALLTAGS];
                        pg_array_hstore_parser::HStoreParser hstore (alltags_str);
                        while (hstore.has_next()) {
                            pg_array_hstore_parser::StringPair kv_pair = hstore.get_next();
//                            std::cout << "[" << kv_pair.first << "]->[" << kv_pair.second << "]\n";

                            tags_count_byte++;

//                            write_byte(5); // !!! FOR TEST

                            write_byte(0);
                            write_string( kv_pair.first.c_str() );
                            write_byte(0);
                            write_string( kv_pair.second.c_str() );

                        };
                    };



                    if (tags_count_byte == 0) {
                        bufs[current_tile_i].data_len -= 1;
                        continue;
                    };

                    memcpy( bufs[current_tile_i].data + tags_count_byte_index, &tags_count_byte, 1 );




                }
                else if (op == 'l') {


                    //printf("l start\n");

//                    char *highway = row[LROW_HIGHWAY];
//                    char *railway = row[LROW_RAILWAY];
//                    char *waterway = row[LROW_WATERWAY];
//                    char *barrier = row[LROW_BARRIER];
//
//                    if ( ! (highway[0] || railway[0] || waterway[0] || barrier[0] || row[LROW_POWER] || row[LROW_MAN_MADE] ) ) {
//                        continue;
//                    };
//
//
//                    int t_index[] = { LROW_HIGHWAY, LROW_RAILWAY, LROW_WATERWAY, LROW_BARRIER, LROW_POWER, LROW_MAN_MADE, LROW_AERIALWAY, LROW_BRIDGE, LROW_TUNNEL,
//                                        LROW_LANES, LROW_ONEWAY, LROW_LIT, LROW_SURFACE, LROW_WIDTH, LROW_ELECTRIFIED, LROW_GAUGE,
//                                        LROW_NAME, LROW_INT_NAME, LROW_NAME_EN, LROW_NAME_RU, LROW_NAME_DE, LROW_NAME_FR,
//                                        LROW_REF, LROW_LAYER,
//                                        0 };
//
//                    char* t_key_string[] = { "highway", "railway", "waterway", "barrier", "power", "man_made", "aerialway", "bridge", "tunnel",
//                                            "lanes", "oneway", "lit", "surface", "width", "electrified", "gauge",
//                                            "name", "int_name", "name:en", "name:ru", "name:de", "name:fr",
//                                            "ref", "layer" };


                    int tags_count_byte_index = bufs[current_tile_i].data_len;
                    write_byte(0); // tags count, will be updated later
                    uint8_t tags_count_byte = 0;

//                    int i = 0;
//                    while (t_index[i] != 0) {
//                        if (row[ t_index[i] ][0]) {
//                            tags_count_byte++;
//                            write_byte(0);
//                            write_string( t_key_string[i] );
//                            write_byte(0);
//                            write_string(row[ t_index[i] ]);
//                        };
//                        i++;
//                    };

                    if (row[LROW_ALLTAGS][0]) {
                        //std::string alltags = alltags;
                        std::string alltags_str = row[LROW_ALLTAGS];
                        pg_array_hstore_parser::HStoreParser hstore (alltags_str);
                        while (hstore.has_next()) {
                            pg_array_hstore_parser::StringPair kv_pair = hstore.get_next();
//                            std::cout << "[" << kv_pair.first << "]->[" << kv_pair.second << "]\n";

                            tags_count_byte++;

////                            write_byte(5); // !!! FOR TEST

                            write_byte(0);
                            write_string( kv_pair.first.c_str() );
                            write_byte(0);
                            write_string( kv_pair.second.c_str() );

                        };
                    };

                    memcpy( bufs[current_tile_i].data + tags_count_byte_index, &tags_count_byte, 1 );


                }
                else {
                    // unknown op or non-interested tags

        //            printf("ignoring: %s, %s, %s, %s.\n", row[ROW_NAME], row[ROW_BUILDING], row[ROW_LANDUSE], row[ROW_NATURAL]);

                    continue;
                };







                if ( op == 'p' ) {

                    write_int(point_packed_coord);

                }
                else {


                    int float_coordinates = 0;
                    if (op == 'b') {
                        if (need_float_coordinates(g_obj, num_geometries, tile_z, tile_x, tile_y)) {
                            float_coordinates = 1;
                        };
                    };

                    int three_byte_coordinates = 0;
                    int three_byte_coordinates_byte_ref = 0;
                    if (can_use_three_byte_coordinates(g_obj, num_geometries, tile_z, tile_x, tile_y, &three_byte_coordinates_byte_ref)) {
                        three_byte_coordinates = 1;
                    };


                    us = num_geometries;
                    if (float_coordinates) {
                        us |= (1 << 13);
                    }
                    else if (three_byte_coordinates) {
                        us |= (1 << 12);
                    }
                    else {
                        us |= (icx << 15) | (icy << 14);
                    };

                    write_us(us);

                    if (three_byte_coordinates) {
                        write_us(three_byte_coordinates_byte_ref);
                    };




                    for (int i = 0; i < num_geometries; i++) {

                        int is_polygon = ( GEOSGeomTypeId(g_obj) == GEOS_MULTIPOLYGON ) || ( GEOSGeomTypeId(g_obj) == GEOS_POLYGON );


            //            printf("i: %d of %d (geom type %d)", i, num_geometries, GEOSGeomTypeId(g_obj) );

                        const GEOSGeometry *g_current = GEOSGetGeometryN(g_obj, i);


            //            printf(" (current type %d)\n", GEOSGeomTypeId(g_current) );

                        int rings_count = (is_polygon) ? (1 + GEOSGetNumInteriorRings(g_current)) : 1;

            //                printf("rings count = %d\n", rings_count);


                        if (is_polygon) {
                            write_us(rings_count);
                        };



                        for (int r = 0; r < rings_count; r++) {

            //                printf("(r%d) ", r);

                            int reverse_orientation = 0;

                            const GEOSGeometry *g2 =
                                (is_polygon) ?
                                    (
                                        (r == 0) ? (GEOSGetExteriorRing( g_current )) : (GEOSGetInteriorRingN(g_current, r-1))
                                    )
                                    :
                                    ( g_current );

            //                printf("(( g2 is %s | %d )) ", GEOSGeomType(g2), is_polygon );
            //
            //                printf("-[-");

                            int num_points = GEOSGeomGetNumPoints(g2);

            //                printf("-]- num points %d geomtype of g2 is %s (is polygon %d) \n", num_points, GEOSGeomType(g2), is_polygon);

                            if (is_polygon) {
                                num_points -= 1; // -1 for linear ring, last == first

                                float total_angle = 0.0;

                                for (int j = 0; j < num_points; j++) {

                                    GEOSGeometry *gpA = GEOSGeomGetPointN(g2, j);
                                    GEOSGeometry *gpO = GEOSGeomGetPointN(g2, (j+1) % num_points );
                                    GEOSGeometry *gpB = GEOSGeomGetPointN(g2, (j+2) % num_points );

                                    double ax, ay, ox, oy, bx, by;
                                    GEOSGeomGetX(gpA, &ax); // lon
                                    GEOSGeomGetY(gpA, &ay); // lat
                                    GEOSGeomGetX(gpO, &ox); // lon
                                    GEOSGeomGetY(gpO, &oy); // lat
                                    GEOSGeomGetX(gpB, &bx); // lon
                                    GEOSGeomGetY(gpB, &by); // lat

                                    float angle = atan2( (ax-ox)*(oy-by) - (ay-oy)*(ox-bx), (ax-ox)*(ox-bx) + (ay-oy)*(oy-by) );

            //                        printf("%.2f, ", angle);

                                    total_angle += angle;

                                    GEOSGeom_destroy(gpA);
                                    GEOSGeom_destroy(gpO);
                                    GEOSGeom_destroy(gpB);

                                    //angle = atan2(OA.x * OB.y - OA.y * OB.x, OA.x * OB.x + OA.y * OB.y);

                                };

            //                    printf("total = %.6f | ", total_angle);

                                if (total_angle > 0.0) {
                                    reverse_orientation = 1;
                                };

                                if (r == 0) {
                                    reverse_orientation = 1 - reverse_orientation;
                                };

                            };


                            int j_start = 0;
                            int j_increment = 1;
                            if (reverse_orientation) {
                                j_start = num_points-1;
                                j_increment = -1;
                            }


            //                printf("    %s (%d points) \n", GEOSGeomType(g2), num_points);


                            write_us(num_points);

                            for (int j = j_start; (j < num_points) && (j >= 0); j += j_increment ) {

                                GEOSGeometry *gp = GEOSGeomGetPointN(g2, j);
                                double x, y;
                                GEOSGeomGetX(gp, &x); // lon
                                GEOSGeomGetY(gp, &y); // lat

                                GEOSGeom_destroy(gp);

                                degrees2tc(tile_z, x, y, &x, &y);

                                x -= tile_x;
                                y -= tile_y;

                                if (float_coordinates) {

                                    write_float(x);
                                    write_float(y);

                                }
                                else if (three_byte_coordinates) {

                                    uint32_t packed_coord = pack_three_byte_coord(x, y, three_byte_coordinates_byte_ref);
                                    write_byte( 0xFF & (packed_coord>>0) );
                                    write_byte( 0xFF & (packed_coord>>8) );
                                    write_byte( 0xFF & (packed_coord>>16) );

                                }
                                else {

                //                    double ox = 0.0;
                //                    double oy = 0.0;

                                    if ( op == 'b' ) {
                                        double ox = 0.25 + 0.5*icx;
                                        double oy = 0.25 + 0.5*icy;

                                        ox -= 0.5;
                                        oy -= 0.5;

                                        x -= ox;
                                        y -= oy;
                                    };


                //                    printf("        (%.5f, %.5f) \n", x, y );

                    //                float fx = (float) x;
                    //                float fy = (float) y;

                                    double scale = (63488.0);
                                    x *= scale;
                                    y *= scale;

                                    uint32_t packed_coord = RVT_PACK_XY(x, y);


                                    write_int(packed_coord);

                                };

                            };

                        };



                    };

                };

        //        printf("\n");

                total_counts[tile_i]++;

//                if (gout != g_obj) {
//                    GEOSGeom_destroy(g_obj);
//                };

            };
        };

        if (g_obj_created) {
            GEOSGeom_destroy(g_obj);
            g_obj_created = 0;
        };

        GEOSGeom_destroy(gout);


    };



    // ============= 2. Ocean for area ========================



    if (ocean) {

        c = 0;
        ni = 0;
        while ((n = csv_row_fread(in_ocean, (unsigned char*) buf, buf_size, (unsigned char**) row, 300, ',', CSV_TRIM | CSV_QUOTES)) > 3) { // at least 3 bytes


            // First line
            if (c == 0) {
                c = 1;
                continue;
            };

            GEOSGeometry *gout = GEOSWKTReader_read(r, (const char*) row[0]); // GEOSGeomFromWKT(wkt);

            if (!gout) {
                printf("Bad geometry (ocean). Skipped. \n" );
                continue;
            };

//////            if ( GEOSGeomTypeId(gout) != GEOS_MULTIPOLYGON ) {
//////                printf("Wrong geom type (ocean gout) (%s instead of multipolygon). Skipped. \n", GEOSGeomType(gout));
//////                GEOSGeom_destroy(gout);
//////                continue;
//////            };

            if ( (GEOSGeomTypeId(gout) != GEOS_MULTIPOLYGON) && (GEOSGeomTypeId(gout) != GEOS_POLYGON) ) {
                printf("Ocean ('a') Wrong geom type (gout) (%s instead of multi-/polygon). Skipped. \n", GEOSGeomType(gout) );
                continue;
            };

            int g_obj_created = 0;

            GEOSGeometry *g_obj;


            for (int ti = 0; ti < t_side_count; ti++) {
                for (int tj = 0; tj < t_side_count; tj++) {

//                    printf("\nTile start\n");

                    if (g_obj_created) {
                        GEOSGeom_destroy(g_obj);
                    };
                    g_obj_created = 0;

                    g_obj = gout;

                    int tile_z = dest_z;
                    int tile_x = t_start_x + tj;
                    int tile_y = t_start_y + ti;
                    int tile_i = ti*t_side_count + tj;

                    //fp_current = fps[tile_i];
                    current_tile_i = tile_i;


                    //double clip_lon1, clip_lat1, clip_lon2, clip_lat2;

                    // shapefiles are in 3857 projection
                    // (x2,y2) must be > (x1,y1)
                    double clip_x1, clip_y1, clip_x2, clip_y2;


//                    tc2degrees( tile_z, tile_x, tile_y+1, &clip_lon1, &clip_lat1);
//                    tc2degrees( tile_z, tile_x+1, tile_y, &clip_lon2, &clip_lat2);
                    tc2proj( tile_z, tile_x, tile_y+1, &clip_x1, &clip_y1 );
                    tc2proj( tile_z, tile_x+1, tile_y, &clip_x2, &clip_y2 );

                    //g_obj = GEOSClipByRect(gout, clip_lon1, clip_lat1, clip_lon2, clip_lat2);
                    g_obj = GEOSClipByRect(gout, clip_x1, clip_y1, clip_x2, clip_y2);

//                    printf("%.4f, %.4f\n%.4f, %.4f\n", clip_x1, clip_y1, clip_x2, clip_y2 );
//                    exit(-1);

                    if (!g_obj) {
                        continue;
                    };
                    g_obj_created = 1;



                    int num_geometries = GEOSGetNumGeometries(g_obj);

//                    printf("num_geometries %d\n", num_geometries);

                    if (num_geometries == 0) {
//                        printf("num_geometries is 0. wkt:\n%s\n", GEOSGeomToWKT(g_obj) );
                        continue;
                    };

//                    printf("For tile %d/%d/%d:\n%s\n\n", tile_z, tile_x, tile_y, GEOSGeomToWKT(g_obj) );
//
//                    printf("num_geometries %d\n", num_geometries);



                    GEOSGeometry *g_envelope;
                    const GEOSGeometry *g_envelope_exterior_ring;
                    double ex1, ey1, ex2, ey2;
                    double cx;
                    double cy;
                    int icx = 0;
                    int icy = 0;


                    if ( 1 ) {
                        g_envelope = GEOSEnvelope( g_obj );
                        g_envelope_exterior_ring = GEOSGetExteriorRing( g_envelope );
                //        printf("envelope: %d points\n", GEOSGeomGetNumPoints(g_envelope));

                        GEOSGeometry *g_point0 = GEOSGeomGetPointN(g_envelope_exterior_ring, 0);
                        GEOSGeometry *g_point2 = GEOSGeomGetPointN(g_envelope_exterior_ring, 2);

                        GEOSGeomGetX( g_point0, &ex1);
                        GEOSGeomGetY( g_point0, &ey1);
                        GEOSGeomGetX( g_point2, &ex2);
                        GEOSGeomGetY( g_point2, &ey2);

                        GEOSGeom_destroy(g_point0);
                        GEOSGeom_destroy(g_point2);
                        GEOSGeom_destroy(g_envelope); // ??????????? !!!!!!!!!!!!!

                        proj2tc(tile_z, ex1, ey1, &ex1, &ey1);
                        proj2tc(tile_z, ex2, ey2, &ex2, &ey2);



                        // ey1 > ey2, but ex2 > ex1

                        // if ( (ey1 > limit) or (ey2 < limit) or (ex2 > limit) or (ex1 < limit) )
                        // then we can't use simple ring

                        cx = 0.5*(ex1+ex2);
                        cy = 0.5*(ey1+ey2);

                        cx -= tile_x;
                        cy -= tile_y;

                        icx = (int) (2.0*cx);
                        icy = (int) (2.0*cy);


                    };


                    if (op == 'a') {
                        // Area
//                        unsigned char b1 = 0;
//                        b1 = OSM_A1_NATURAL_WATER;
//                        write_byte(b1);

                        write_byte(1);
                        write_byte(0);
                        write_string("natural");
                        write_byte(0);
                        write_string("water");

                    };



                    if (1)  {

                        us = num_geometries | (icx << 15) | (icy << 14);
                        write_us(us);




                        for (int i = 0; i < num_geometries; i++) {

                            int is_polygon = ( GEOSGeomTypeId(g_obj) == GEOS_MULTIPOLYGON ) || ( GEOSGeomTypeId(g_obj) == GEOS_POLYGON );

                            const GEOSGeometry *g_current = GEOSGetGeometryN(g_obj, i);

                            int rings_count = (is_polygon) ? (1 + GEOSGetNumInteriorRings(g_current)) : 1;

                            if (is_polygon) {
                                write_us(rings_count);
                            };

                            for (int r = 0; r < rings_count; r++) {

                                int reverse_orientation = 0;

                                const GEOSGeometry *g2 =
                                    (is_polygon) ?
                                        (
                                            (r == 0) ? (GEOSGetExteriorRing( g_current )) : (GEOSGetInteriorRingN(g_current, r-1))
                                        )
                                        :
                                        ( g_current );

                                int num_points = GEOSGeomGetNumPoints(g2);

                                if (is_polygon) {
                                    num_points -= 1; // -1 for linear ring, last == first

                                    float total_angle = 0.0;

                                    for (int j = 0; j < num_points; j++) {

                                        GEOSGeometry *gpA = GEOSGeomGetPointN(g2, j);
                                        GEOSGeometry *gpO = GEOSGeomGetPointN(g2, (j+1) % num_points );
                                        GEOSGeometry *gpB = GEOSGeomGetPointN(g2, (j+2) % num_points );

                                        double ax, ay, ox, oy, bx, by;
                                        GEOSGeomGetX(gpA, &ax); // lon
                                        GEOSGeomGetY(gpA, &ay); // lat
                                        GEOSGeomGetX(gpO, &ox); // lon
                                        GEOSGeomGetY(gpO, &oy); // lat
                                        GEOSGeomGetX(gpB, &bx); // lon
                                        GEOSGeomGetY(gpB, &by); // lat

                                        float angle = atan2( (ax-ox)*(oy-by) - (ay-oy)*(ox-bx), (ax-ox)*(ox-bx) + (ay-oy)*(oy-by) );

                                        total_angle += angle;

                                        GEOSGeom_destroy(gpA);
                                        GEOSGeom_destroy(gpO);
                                        GEOSGeom_destroy(gpB);

                                        //angle = atan2(OA.x * OB.y - OA.y * OB.x, OA.x * OB.x + OA.y * OB.y);

                                    };


                                    if (total_angle > 0.0) {
                                        reverse_orientation = 1;
                                    };

                                    if (r == 0) {
                                        reverse_orientation = 1 - reverse_orientation;
                                    };

                                };


                                int j_start = 0;
                                int j_increment = 1;
                                if (reverse_orientation) {
                                    j_start = num_points-1;
                                    j_increment = -1;
                                }

                                write_us(num_points);

                                for (int j = j_start; (j < num_points) && (j >= 0); j += j_increment ) {

                                    GEOSGeometry *gp = GEOSGeomGetPointN(g2, j);
                                    double x, y;
                                    GEOSGeomGetX(gp, &x); // 3857 proj x
                                    GEOSGeomGetY(gp, &y); // 3857 proj y

                                    GEOSGeom_destroy(gp);

////                                    printf("(%.4f %.4f)->", x, y);

                                    proj2tc(tile_z, x, y, &x, &y);

                                    x -= tile_x;
                                    y -= tile_y;

////                                    printf("(%.4f %.4f) ", x, y);

                                    double scale = (63488.0);
                                    x *= scale;
                                    y *= scale;

                                    uint32_t packed_coord = RVT_PACK_XY(x, y);

                                    write_int(packed_coord);

                                };

////                                printf("\n\n");

                            };



                        };

                    };


                    total_counts[tile_i]++;

    //                if (gout != g_obj) {
    //                    GEOSGeom_destroy(g_obj);
    //                };

                };
            };

            if (g_obj_created) {
                GEOSGeom_destroy(g_obj);
                g_obj_created = 0;
            };

            GEOSGeom_destroy(gout);


        };  // while ocean fread

    }; // if ocean



    GEOSWKTReader_destroy(r);

//    fprintf(stderr, "Writing to .data files...\n");


    for (int i = 0; i < t_side_count; i++) {
        for (int j = 0; j < t_side_count; j++) {
            //printf("%d_%d\n", t_start_x + i, t_start_y + j);
            int tile_x = t_start_x + j;
            int tile_y = t_start_y + i;
            int tile_i = i*t_side_count + j;
            sprintf(filename, "%d_%d_%d_%c.data", dest_z, tile_x, tile_y, op);


            unsigned short us = total_counts[tile_i];
            memcpy( bufs[tile_i].data, &us, 2 );


            FILE * fp = fopen(filename, "wb");

            fwrite( bufs[tile_i].data, bufs[tile_i].data_len, 1, fp );

            fclose(fp);


            free(bufs[tile_i].data);

        };
    };

//    fprintf(stderr, "Done. \n");


//    for (int i = 0; i < t_total_count; i++) {
//
//        unsigned short us = total_counts[i];
//        memcpy( bufs[i].data, &us, 2 );
//
//
//        FILE *
//
//
////        fp_current = fps[i];
////        fseek(fp_current, 0, SEEK_SET);
////        write_us(total_counts[i]);
////        fclose(fp_current);
//    };


    if (ocean) {
        fclose(in_ocean);
    };

    fclose(in);

    free(buf);

//    for (int i = 0; i < t_total_count; i++) {
//        fclose( fps[i] );
//    };
    free(bufs);

    free(total_counts);

    finishGEOS();

    if (ignored_ids) {
        free(ign_types_array);
        free(ign_ids_array);
    };

    //printf("Done.\n");

    exit(0);
}
