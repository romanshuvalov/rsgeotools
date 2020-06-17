#ifndef rs_GEOM_H_INCLUDED
#define rs_GEOM_H_INCLUDED

#include "rsmx.h"

#define RS_GEOM_MAX_POINTS_PER_LINESTRING   1024
#define RS_GEOM_MAX_RINGS_PER_POLYGON       128

#define RS_GEOM_FLAG_DROP_NO_OUTER_RING     0x01


extern int rs_geom_flags;

typedef rs_vec2_t rs_point_t;

typedef struct rs_triangle_t {

    rs_point_t p[3];

} rs_triangle_t;


typedef struct rs_triangle_set_t {
    int t_count;
    int flags;
    int max_count;

    rs_triangle_t *t;

} rs_triangle_set_t;

typedef struct rs_indexed_triangle_set_t {

    int flags;

    int i_count; // = triangles count / 3
    int p_count;

    int max_i_count;
    int max_p_count;

    rs_point_t *p_array;
    uint32_t *i_array;

} rs_indexed_triangle_set_t;

typedef struct rs_linestring_t {

    int points_count;
    int max_points_count;
    int flags;
    int reserved0;

    rs_point_t *p; 

} rs_linestring_t;

typedef struct rs_static_linestring_t {
    int points_count;
    int flags;
} rs_static_linestring_t;


typedef struct rs_shape_t {

    int rings_count;
    int max_rings_count;
    int outer_rings_count;
    int flags;

    rs_linestring_t **rings; 

} rs_shape_t;

typedef struct rs_static_shape_t {
    int rings_count;
    int outer_rings_count;
    int flags;
    int reserved;
} rs_static_shape_t;


typedef struct rs_shape_metadata_t {
    int shape_type;
    int shape_points_count;
    float radius;
    float halflength;
    float halfwidth;
    float lin_ratio;
    float azimuth;
    float precision;
    rs_vec2_t center;
} rs_shape_metadata_t;

#define RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONVEX    0x0001
#define RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONCAVE   0x0002
#define RS_SHAPE_TYPE_UNKNOWN_COMPLEX   0x0008
#define RS_SHAPE_TYPE_UNKNOWN_MASK      0x000F

#define RS_SHAPE_TYPE_ROUND         0x0010

#define RS_SHAPE_TYPE_RECTANGLE     0x0100


typedef struct rs_angled_point_t {
    rs_point_t p;
    float azimuth;
} rs_angled_point_t;

rs_linestring_t* rs_linestring_create(int points_count); 
void rs_linestring_destroy(rs_linestring_t *ls);
void rs_linestring_append_point(rs_linestring_t *ls, rs_point_t p);
void rs_linestring_clear(rs_linestring_t *ls);

rs_linestring_t* rs_linestring_create_copy(rs_linestring_t *ls);
rs_linestring_t* rs_linestring_create_shifted(rs_linestring_t *ls, rs_point_t shift); 


float rs_linestring_distance_from_point(rs_linestring_t *ls, rs_point_t p);

rs_angled_point_t rs_linestring_get_middle_angled_point(rs_linestring_t *ls);

void rs_linestring_printf(rs_linestring_t *ls);
float rs_linestring_length(rs_linestring_t *ls);
float rs_linestring_max_segment_length(rs_linestring_t *ls);

void rs_shape_printf(rs_shape_t *sh);


typedef struct rs_point_set_t {

    rs_angled_point_t *p;
    int max_points_count;
    int points_count;

} rs_point_set_t;

#define POINT_SET_FLAG_ZIGZAG   0x0001
#define POINT_SET_FLAG_CLOSED   0x0002
#define POINT_SET_FLAG_RANDOMIZER_TYPE_SIMPLE   0x0004
#define POINT_SET_FLAG_RANDOMIZER_TYPE_ISLANDS  0x0008

rs_point_set_t* rs_point_set_create(int max_points_count);
void rs_point_set_add_point(rs_point_set_t *ps, rs_angled_point_t ap );

rs_point_set_t * rs_point_set_create_from_linestring(rs_linestring_t *ls, float period, float side_shift, float angle, int flags);
void rs_point_set_destroy(rs_point_set_t *ps);
void rs_point_set_filter_by_distance(rs_point_set_t *ps, float distance);
void rs_point_set_make_azimuths_smooth(rs_point_set_t *ps);

rs_point_set_t * rs_point_set_create_from_triangle(rs_triangle_t *tr, float distance, float random_k);
rs_point_set_t * rs_point_set_create_from_quad(float distance, float random_k, float quad_side, int randomizer_flags);
rs_point_set_t * rs_point_set_create_from_quad_random_islands(int islands_count, int points_per_island, float island_radius, float quad_side, int randomizer_flags);


float rs_triangle_area(rs_triangle_t *tr);
float rs_shape_area(rs_shape_t *sh);

int rs_is_point_in_triangle(rs_point_t p, rs_triangle_t *tr);
int rs_is_point_in_linestring(rs_point_t p, rs_linestring_t *ls, float threshold);


rs_triangle_set_t* rs_triangle_set_create(int max_triangles_count);
rs_triangle_set_t* rs_triangle_set_create_triangulated(rs_shape_t *src);
rs_triangle_set_t* rs_triangle_set_create_triangulated_adv(rs_shape_t *src, double point_distance_epsilon);

rs_triangle_set_t* rs_triangle_set_create_from_indexed_triangle_set(rs_indexed_triangle_set_t *tset);

void rs_triangle_set_append_triangle(rs_triangle_set_t *dest,  rs_point_t v0, rs_point_t v1, rs_point_t v2 );
void rs_triangle_set_destroy(rs_triangle_set_t *dest);
void rs_triangle_set_add_max_count(rs_triangle_set_t *dest, int max_count_increment);



rs_triangle_set_t** rs_triangle_set_array_create_subdivided_and_triangulated(rs_shape_t *src, float sc, int subtiles_side_count );
void rs_triangle_set_array_destroy(rs_triangle_set_t** ar, int subtiles_side_count);


void rs_shape_analyze_metadata( rs_shape_metadata_t* metadata, rs_shape_t *sh );


rs_shape_t* rs_shape_create( int max_rings_count );
void rs_shape_destroy( rs_shape_t *dest );
void rs_shape_append_ring(rs_shape_t *dest, rs_linestring_t *ring);

void rs_shape_calculate_outer_rings(rs_shape_t *dest);

unsigned char* rs_static_shape_data_create( rs_shape_t *sh, int *out_content_len );

rs_shape_t* rs_shape_create_from_static( rs_static_shape_t* static_shape );

rs_angled_point_t rs_shape_get_middle_angled_point(rs_shape_t *sh);


#define RS_POLYGON_CLOSED          0 //  (ClipperLib::EndType::etClosedPolygon)
#define RS_LINESTRING_CLOSED       1 //     (ClipperLib::EndType::etClosedLine)
#define RS_LINESTRING_OPEN_BUTT    2 //     (ClipperLib::EndType::etOpenButt)
#define RS_LINESTRING_OPEN_SQUARE  3 //     (ClipperLib::EndType::etOpenSquare)
#define RS_LINESTRING_OPEN_ROUND   4 //     (ClipperLib::EndType::etOpenRound)

enum EndType {etClosedPolygon, etClosedLine, etOpenButt, etOpenSquare, etOpenRound};

rs_shape_t* rs_shape_create_buffered_from_linestring(rs_linestring_t *src, float factor, int ls_type);
rs_shape_t* rs_shape_create_buffered_from_polygon(rs_shape_t *src, float factor, int p_type);

rs_shape_t* rs_shape_create_with_offset(rs_shape_t *src, float offset);

rs_shape_t* rs_shape_create_segmentized(rs_shape_t *src, float max_segment_length);

rs_shape_t* rs_shape_create_box(float x, float y, float halflength, float halfwidth, float azimuth);

rs_shape_t *rs_shape_create_simplified(rs_shape_t *src, float tolerance);
rs_shape_t *rs_shape_create_convex_hull(rs_shape_t *src);

// enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
#define RS_CLIP_INTERSECTION    0
#define RS_CLIP_UNION           1
#define RS_CLIP_DIFFERENCE      2
#define RS_CLIP_XOR             3

rs_shape_t* rs_shape_create_clipped(rs_shape_t *subj, rs_shape_t *clip, int op, int subj_is_closed);

rs_shape_t* rs_shape_create_no_side_effect(rs_shape_t *p, float sc);

int rs_shape_get_total_points(rs_shape_t *sh);

rs_point_t rs_shape_get_middle_point(rs_shape_t *sh);
rs_point_t rs_linestring_get_middle_point(rs_linestring_t *ls);

float rs_shape_perimeter(rs_shape_t *sh);

rs_linestring_t *rs_linestring_create_convex_hull(rs_linestring_t *src_ls);

#endif // H_INCLUDED

