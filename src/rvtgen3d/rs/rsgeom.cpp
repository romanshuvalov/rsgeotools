
extern "C" {
    #include "rsgeom.h"
    #include "rsdebug.h"
    #include "rsmx.h"
    #include "rsmem.h"
    #include "rsboost.h"
    #include "rsnoise.h"
}

int rs_geom_flags = 0;

#include <string.h>



#include <math.h>

#ifdef RS_MACOS
    #include <array> // needed for mac os
    #include <vector> // needed for mac os
#endif

#include "clipper/clipper.hpp"
#include "earcut/earcut.hpp"

#ifndef M_PI_2
    #define M_PI_2		1.57079632679489661923
#endif

rs_linestring_t* rs_linestring_create(int max_points_count) {

    rs_linestring_t *ls = (rs_linestring_t*) rs_mem_alloc( sizeof(rs_linestring_t), RS_MEM_POOL_AUTO );
    rs_app_assert_memory(ls, "geom", __LINE__);

    ls->p = (rs_point_t*) rs_mem_alloc_adv( sizeof(rs_point_t) * max_points_count, "geom", __LINE__, -1 );
    rs_app_assert_memory(ls->p, "geom", __LINE__);
    ls->max_points_count = max_points_count;
    ls->points_count = 0;
    ls->flags = 0;

    return ls;

};

rs_linestring_t* rs_linestring_create_copy(rs_linestring_t *ls) {

    rs_linestring_t *dest = rs_linestring_create(ls->points_count);

    for (int i = 0; i < ls->points_count; i++) {
        rs_linestring_append_point(dest, ls->p[i] );
    };

    return dest;

};


rs_linestring_t* rs_linestring_create_shifted(rs_linestring_t *ls, rs_point_t shift) {

    rs_linestring_t *dest = rs_linestring_create(ls->points_count);

    for (int i = 0; i < ls->points_count; i++) {
        rs_linestring_append_point(dest, rs_vec2_add(ls->p[i], shift) );
    };

    return dest;

};



void rs_linestring_destroy(rs_linestring_t *ls) {

    rs_mem_free(ls->p);
    rs_mem_free(ls);

};

void rs_linestring_append_point(rs_linestring_t *ls, rs_point_t p) {

    if ( ls->points_count == ls->max_points_count  ) {
        ls->max_points_count *= 2;
        ls->p = (rs_point_t*) rs_mem_realloc(ls->p, sizeof(rs_point_t) * ls->max_points_count );
        rs_app_assert_memory(ls->p, "geom", __LINE__);
    };

    ls->p[ ls->points_count++ ] = p;


};

void rs_linestring_clear(rs_linestring_t *ls) {

    ls->points_count = 0;

};

using namespace ClipperLib;

#define CLIPPER_FLOAT_PRECISION  (4096.0)

#define DEBUGPOLYf  DEBUG30f


rs_shape_t* rs_shape_create_from_solution_paths(Paths solution) {

    rs_shape_t* dest = rs_shape_create(solution.size());

    for (int or_i = 1; or_i >= 0; or_i--) {
        for (size_t i = 0; i < solution.size(); i++) {
            Path path = solution[i];

            if ( (int)(Orientation(path)) == or_i) {

                dest->outer_rings_count += or_i;

                rs_linestring_t *ring = rs_linestring_create( path.size() );
                for (int j = path.size()-1; j >= 0; j--) { // invert because of cull face
                    IntPoint ip = path[j];
                    rs_linestring_append_point(ring, rs_vec2(1.0/CLIPPER_FLOAT_PRECISION*ip.X, 1.0/CLIPPER_FLOAT_PRECISION*ip.Y));
                };

                rs_shape_append_ring(dest, ring);

            };

        };
    };


    return dest;


};

rs_shape_t* rs_shape_create_clipped(rs_shape_t *subj_polygon, rs_shape_t *clip_polygon, int op, int subj_is_closed) {

    DEBUGPOLYf("polygon create clipped: op %d, subj_is_closed %d \n", op, subj_is_closed);

    Paths solution;
    PolyTree polytree_solution;

    Paths subj;
    Paths clp;

    Clipper clpr;

    for (int r = 0; r < subj_polygon->rings_count; r++) {

        Path subj;
        DEBUGPOLYf("SUBJ: (r = %d of %d), points_count = %d\n", r, subj_polygon->rings_count, subj_polygon->rings[r]->points_count);
        for (int i = 0; i < subj_polygon->rings[r]->points_count; i++) {
            subj << IntPoint( CLIPPER_FLOAT_PRECISION * subj_polygon->rings[r]->p[i].x, CLIPPER_FLOAT_PRECISION * subj_polygon->rings[r]->p[i].y );
        };

        if (!subj_is_closed) {
            // repeat first point
            subj << IntPoint( CLIPPER_FLOAT_PRECISION * subj_polygon->rings[r]->p[0].x, CLIPPER_FLOAT_PRECISION * subj_polygon->rings[r]->p[0].y );
        };

        clpr.AddPath(subj, ptSubject, subj_is_closed ? true : false);

    };

    for (int r = 0; r < clip_polygon->rings_count; r++) {

        Path subj;
        DEBUGPOLYf("CLIP: (r = %d of %d), points_count = %d\n", r, clip_polygon->rings_count, clip_polygon->rings[r]->points_count);
        for (int i = 0; i < clip_polygon->rings[r]->points_count; i++) {
            subj << IntPoint( CLIPPER_FLOAT_PRECISION * clip_polygon->rings[r]->p[i].x, CLIPPER_FLOAT_PRECISION * clip_polygon->rings[r]->p[i].y );
        };

        clpr.AddPath(subj, ptClip, true);

    };

    clpr.Execute( ClipperLib::ClipType(op), polytree_solution, pftEvenOdd, pftEvenOdd);

    if (subj_is_closed) {
        ClosedPathsFromPolyTree(polytree_solution, solution);
    }
    else {
        OpenPathsFromPolyTree(polytree_solution, solution);
    };

    DEBUGPOLYf("solution polytree childs count: %d \n", solution.size());

    return rs_shape_create_from_solution_paths(solution);

};


float rs_linestring_length(rs_linestring_t *ls) {
    float len = 0.0;
    if (ls->points_count < 2) {
        return 0.0;
    };
    for (int i = 1; i < ls->points_count; i++) {
        len += rs_vec2_distance( ls->p[i], ls->p[i-1] );
    };
    return len;
};

float rs_linestring_max_segment_length(rs_linestring_t *ls) {

    if (ls->points_count < 2) {
        return 0.0;
    };

    float dist = 0.0;
    for (int i = 1; i < ls->points_count; i++) {
        float new_dist = rs_vec2_distance(ls->p[i], ls->p[i-1]);
        if ( new_dist > dist ) {
            dist = new_dist;
        };
    };

    return dist;

};

int rs_linestring_segments_count(rs_linestring_t *ls, float max_segment_length) {
    int c = 0;
    for (int i = 1; i < ls->points_count; i++) {
        c += 1 + (int)((rs_vec2_distance( ls->p[i], ls->p[i-1] )) / max_segment_length);
    };
    return c;
};


rs_shape_t* rs_shape_create_segmentized(rs_shape_t *src, float max_segment_length) {

    rs_shape_t* dest = rs_shape_create(src->rings_count);

    for (int i = 0; i < src->rings_count; i++) {

        rs_linestring_t *src_ring = src->rings[i];

        int total_max_segments = rs_linestring_segments_count(src_ring, max_segment_length);

        total_max_segments = (total_max_segments & 0xFFFFFFE0) | (0x1F);

        rs_linestring_t *ring = rs_linestring_create( total_max_segments );

        for (int j = 0; j < src_ring->points_count; j++) {


            if (j == 0) {
                rs_linestring_append_point(ring, src_ring->p[j]);
            }
            else {

                int segments = 1 + (int)((rs_vec2_distance( src_ring->p[j], src_ring->p[j-1] )) / max_segment_length);
                if (segments > 150) {

                    rs_critical_alert_and_halt_sprintf("segments > 150: distance %.3f\nj=%d\np[j]=(%.3f, %.3f) ",
                                                        rs_vec2_distance( src_ring->p[j], src_ring->p[j-1] ),
                                                        j, src_ring->p[j].x, src_ring->p[j].y );

                    continue; // this should never happen
                }
                float step = 1.0 / segments;
                for (int s = 0; s < segments; s++) {
                    rs_linestring_append_point(ring, rs_vec2_add( rs_vec2(0.02*((s+1)%2), 0.02*(s%2)), rs_vec2_linear(src_ring->p[j-1], src_ring->p[j], step*(s+1)) ) );

                };


            };

        };

        rs_shape_append_ring(dest, ring);

    };

    return dest;

};


rs_shape_t* rs_shape_create_buffered_adv(rs_linestring_t **src, int ls_count, float factor, int end_type) {

    DEBUGPOLYf("create buffered adv start: ls_count %d, factor %.3f\n", ls_count, factor);

    Paths solution;

    ClipperOffset co(2.6, 512.0 / 4096.0 * CLIPPER_FLOAT_PRECISION  );

    for (int r = 0; r < ls_count; r++) {

        Path subj;
        DEBUGPOLYf("(r = %d of %d), points_count = %d\n", r, ls_count, src[r]->points_count);
        for (int i = 0; i < src[r]->points_count; i++) {
            subj << IntPoint( CLIPPER_FLOAT_PRECISION * src[r]->p[i].x, CLIPPER_FLOAT_PRECISION * src[r]->p[i].y );
        };

        co.AddPath(subj, jtMiter, (ClipperLib::EndType) end_type) ;

    };
    co.Execute(solution, factor * CLIPPER_FLOAT_PRECISION);

    DEBUGPOLYf("solution size: %d (ls_count %d)\n", solution.size(), ls_count);

    return rs_shape_create_from_solution_paths(solution);

};

rs_shape_t* rs_shape_create_buffered_from_linestring(rs_linestring_t *src, float factor, int ls_type) {
    return rs_shape_create_buffered_adv( &src, 1, factor, ls_type );
};

rs_shape_t* rs_shape_create_buffered_from_polygon(rs_shape_t *src, float factor, int p_type) {
    return rs_shape_create_buffered_adv( src->rings, src->rings_count, factor, p_type );
};

rs_shape_t* rs_shape_create_with_offset(rs_shape_t *src, float line_offset) {

    rs_shape_t *p_1 = rs_shape_create_buffered_from_polygon(src, line_offset, RS_LINESTRING_OPEN_SQUARE);
    rs_shape_t *p_2 = rs_shape_create_buffered_from_polygon(src, line_offset*1.2, RS_LINESTRING_OPEN_BUTT);

    rs_shape_t *p_intersection = rs_shape_create_clipped(p_1, p_2, RS_CLIP_INTERSECTION, 0);

    rs_shape_destroy(p_1);
    rs_shape_destroy(p_2);

    return p_intersection;

};


int rs_shape_inner_rings_are_safe(rs_shape_t *src, float distance_epsilon) {

    if (src->rings_count < 1) {
        return 1;
    };

    rs_linestring_t *r0 = src->rings[0];
    for (int ri = 1; ri < src->rings_count; ri++) {
        rs_linestring_t *r = src->rings[ri];

        for (int pi = 0; pi < r->points_count; pi++) {
            for (int p0i = 0; p0i < r0->points_count; p0i++) {

                if ( rs_vec2_distance_sqr( r->p[pi], r0->p[p0i] ) < (distance_epsilon*distance_epsilon) ) {
                    return 0;
                };

            };
        };

    };

    return 1;

};


rs_triangle_set_t* rs_triangle_set_create(int max_triangles_count) {

    rs_triangle_set_t* dest = (rs_triangle_set_t*) rs_mem_alloc_adv(sizeof(rs_triangle_set_t), "geom", __LINE__, RS_MEM_POOL_AUTO);
    rs_app_assert_memory(dest, "geom", __LINE__);

    dest->t = (rs_triangle_t*) rs_mem_alloc( sizeof(rs_triangle_t) * max_triangles_count, RS_MEM_POOL_AUTO );
    rs_app_assert_memory(dest->t, "geom", __LINE__);
    dest->t_count = 0;
    dest->max_count = max_triangles_count;
    dest->flags = 0;

    return dest;

};

rs_indexed_triangle_set_t* rs_indexed_triangle_set_create(int max_i_count, int max_p_count) {

    rs_indexed_triangle_set_t* dest = (rs_indexed_triangle_set_t*) rs_mem_alloc_adv(sizeof(rs_indexed_triangle_set_t), "geom", __LINE__, RS_MEM_POOL_AUTO);
    rs_app_assert_memory(dest, "geom", __LINE__);


    dest->p_array = (rs_point_t*) rs_mem_alloc( sizeof(rs_point_t) * max_p_count, RS_MEM_POOL_AUTO );
    rs_app_assert_memory(dest->p_array, "geom", __LINE__);
    dest->p_count = 0;

    dest->i_array = (uint32_t*) rs_mem_alloc( sizeof(uint32_t) * max_i_count, RS_MEM_POOL_AUTO );
    rs_app_assert_memory(dest->i_array, "geom", __LINE__);
    dest->i_count = 0;

    dest->max_i_count = max_i_count;
    dest->max_p_count = max_p_count;

    dest->flags = 0;

    return dest;

};

void rs_triangle_set_add_max_count(rs_triangle_set_t *dest, int max_count_increment) {
    dest->max_count += max_count_increment;
    dest->t = (rs_triangle_t*) rs_mem_realloc( dest->t, sizeof(rs_triangle_t) * dest->max_count );
    rs_app_assert_memory(dest->t, "geom", __LINE__);
};

void rs_indexed_triangle_set_add_max_p_count(rs_indexed_triangle_set_t *dest, int max_count_increment) {
    dest->max_p_count += max_count_increment;
    dest->p_array = (rs_point_t*) rs_mem_realloc( dest->p_array, sizeof(rs_point_t) * dest->max_p_count );
    rs_app_assert_memory(dest->p_array, "geom", __LINE__);
};

void rs_indexed_triangle_set_add_max_i_count(rs_indexed_triangle_set_t *dest, int max_count_increment) {
    dest->max_i_count += max_count_increment;
    dest->i_array = (uint32_t*) rs_mem_realloc( dest->i_array, sizeof(uint32_t) * dest->max_i_count );
    rs_app_assert_memory(dest->i_array, "geom", __LINE__);
};



void rs_triangle_set_append_triangle(rs_triangle_set_t *dest,  rs_point_t v0, rs_point_t v1, rs_point_t v2 ) {


    if (dest->t_count == dest->max_count) {

        rs_triangle_set_add_max_count(dest, 24);

    };

    dest->t[ dest->t_count ].p[0] = v0;
    dest->t[ dest->t_count ].p[1] = v1;
    dest->t[ dest->t_count ].p[2] = v2;

    dest->t_count++;

};

void rs_indexed_triangle_set_append_point(rs_indexed_triangle_set_t *dest, rs_point_t p ) {

    if (dest->p_count == dest->max_p_count) {
        rs_indexed_triangle_set_add_max_p_count(dest, 128);
    };

    dest->p_array[ dest->p_count ] = p;
    dest->p_count++;

};

void rs_indexed_triangle_set_append_triangle(rs_indexed_triangle_set_t *dest, int i0, int i1, int i2 ) {

    if (dest->i_count == dest->max_i_count) {
        rs_indexed_triangle_set_add_max_i_count(dest, 128);
    };

    dest->i_array[ dest->i_count + 0 ] = i0;
    dest->i_array[ dest->i_count + 1 ] = i1;
    dest->i_array[ dest->i_count + 2 ] = i2;

    dest->i_count += 3;

};


#define TRIANGLE_SAFE_THRESHOLD (0.0001f)

#define DEBUGTRIf DEBUG30f


#if 1 // initial version of triangulator


rs_triangle_set_t* rs_triangle_set_create_triangulated(rs_shape_t *src) {
    return rs_triangle_set_create_triangulated_adv(src, 0.000001);
};

rs_triangle_set_t* rs_triangle_set_create_triangulated_adv(rs_shape_t *src, double point_distance_epsilon ) {



    if (src->rings_count == 0) {
        DEBUGTRIf("rs_triangle_set_create_triangulated() No rings, returning empty set\n");
        rs_triangle_set_t* dest = rs_triangle_set_create(0);
        return dest;
    };

    if (src->rings[0]->points_count == 0) {
        DEBUGTRIf("rs_triangle_set_create_triangulated() No points in first ring, returning empty set\n");
        rs_triangle_set_t* dest = rs_triangle_set_create(0);
        return dest;
    };

    int outer_rings_count = src->outer_rings_count;

    if (!outer_rings_count) {

        if (src->rings_count == 1) {
            DEBUGTRIf("rs_triangle_set_create_triangulated() WARNING, no outer rings (total rings %d, points count = %d)\n", src->rings_count, src->rings[0]->points_count);
            outer_rings_count = 1;
        }
        else {
            if ( (rs_geom_flags & RS_GEOM_FLAG_DROP_NO_OUTER_RING) || (!rs_shape_inner_rings_are_safe(src, point_distance_epsilon)) ) {
                rs_triangle_set_t* dest = rs_triangle_set_create(0);
                return dest;
            }
            else {
                DEBUGTRIf("rs_triangle_set_create_triangulated() WARNING, no outer rings, inner rings should be safe (total rings %d, points count = %d)\n", src->rings_count, src->rings[0]->points_count);
                outer_rings_count = 1;
            };
        };

    };




    using Coord = double;
    using N = uint32_t;
    using Point = std::array<Coord, 2>;
    std::vector<std::vector<Point>> polys;

    std::vector<Point> global_poly;

    std::vector<N> indices; // result

    rs_triangle_set_t* dest = NULL;

    DEBUGTRIf("rings 0 points count %d\n", src->rings[0]->points_count);


    for (int outer_i = 0; outer_i < outer_rings_count; outer_i++ ) {

        DEBUGTRIf("==== outer_i = %d of %d: ====\n", outer_i, outer_rings_count);

        std::vector<Point> poly;


        rs_vec2_t last_point = rs_vec2( src->rings[outer_i]->p[0].x + 10000.0, src->rings[outer_i]->p[0].y );
        rs_vec2_t start_point;



        for (int i = 0; i < src->rings[outer_i]->points_count; i++) {


            rs_point_t v = rs_vec2( src->rings[outer_i]->p[i].x, src->rings[outer_i]->p[i].y );
            if (i == 0) {
                start_point = v;
            };

            if ( rs_vec2_distance_sqr(last_point, v) < point_distance_epsilon ) {
                continue;
            };

            if ( i == src->rings[outer_i]->points_count-1 ) {
                if ( rs_vec2_distance_sqr(start_point, v) < point_distance_epsilon ) {
                    continue;
                };
            };


            last_point = v;

            DEBUGTRIf("point: %.5f, %.5f\n", src->rings[outer_i]->p[i].x, src->rings[outer_i]->p[i].y);

            poly.push_back( {src->rings[outer_i]->p[i].x, src->rings[outer_i]->p[i].y} );

        };



        polys.push_back(poly);
        global_poly.insert( global_poly.end(), poly.begin(), poly.end() );


        //  Holes

        DEBUGTRIf("adding holes\n");

        // !!!! WARNING:
        // adding a hole, even if it's outside current outer_ring, may cause problems
        // if so, check for every ring if it is inside current outer_ring

        for (int r = outer_rings_count; r < src->rings_count; r++) {
            DEBUGTRIf("===== INNER RING #%d ====\n", r);
            std::vector<Point> hole;
            for (int i = 0; i < src->rings[r]->points_count; i++) {
                hole.push_back( {src->rings[r]->p[i].x, src->rings[r]->p[i].y} );
                DEBUGTRIf("   point %d: (%.4f, %.4f)\n", i, src->rings[r]->p[i].x, src->rings[r]->p[i].y );
            };
            polys.push_back(hole);
            global_poly.insert( global_poly.end(), hole.begin(), hole.end() );
        };

        // Do earcut

        DEBUGTRIf("Calling triangulation function \n");
        indices = mapbox::earcut<N>(polys );

        int tri_result_size =  indices.size();

        // (Re)alloc Triangle set

        if (!dest) {
            dest = rs_triangle_set_create(tri_result_size);
        }
        else {
            rs_triangle_set_add_max_count(dest, tri_result_size);
        };

        // Append result to triangle set

        for (int i = 0; i < tri_result_size; i+=3) {

                DEBUGTRIf("==== %d %d %d \n", indices[i+0], indices[i+1], indices[i+2]);
                rs_triangle_set_append_triangle(dest,
                                                rs_vec2(global_poly[indices[i+0]][0], global_poly[indices[i+0]][1]),
                                                rs_vec2(global_poly[indices[i+1]][0], global_poly[indices[i+1]][1]),
                                                rs_vec2(global_poly[indices[i+2]][0], global_poly[indices[i+2]][1])
                                                );
        };

        indices.clear();
        polys.clear();

        global_poly.clear();

    };


    return dest;

};


#endif


rs_triangle_set_t** rs_triangle_set_array_create_subdivided_and_triangulated(rs_shape_t *src, float sc, int subtiles_side_count ) {



    int subtiles_total_count = subtiles_side_count * subtiles_side_count;

    size_t array_bytes_len = sizeof(rs_triangle_set_t*) * ( subtiles_total_count + 1);

    rs_triangle_set_t** triangle_set_array = (rs_triangle_set_t**) rs_mem_alloc_adv( array_bytes_len, "geom", __LINE__, -1 );

    memset( triangle_set_array, 0, array_bytes_len );


    float subtile_halfsize = 0.5  * sc / subtiles_side_count;

    for (int i = 0; i < subtiles_side_count; i++) {

        for (int j = 0; j < subtiles_side_count; j++) {

            int ar_i = i*subtiles_side_count + j;


            float x_center =    1.0 * ( (1.0*j) + 0.5 ) * sc / subtiles_side_count;
            float y_center =    1.0 * ( (1.0*i) + 0.5 ) * sc / subtiles_side_count;


            rs_shape_t *p_quad = rs_shape_create_box( x_center, y_center, subtile_halfsize, subtile_halfsize, 0.0 );

            rs_shape_t *p_intersection = rs_shape_create_clipped(src, p_quad, RS_CLIP_INTERSECTION, 1);

            if (p_intersection->outer_rings_count != 0) {
                triangle_set_array[ar_i] = rs_triangle_set_create_triangulated(p_intersection);
            }
            else {
                // leave as is (NULL)
            };

            rs_shape_destroy(p_quad);
            rs_shape_destroy(p_intersection);


        };

    };



    return triangle_set_array;

};

void rs_triangle_set_array_destroy(rs_triangle_set_t** ar, int subtiles_side_count) {

    int subtiles_total_count = subtiles_side_count * subtiles_side_count;

    for (int i = 0; i < subtiles_total_count; i++) {
        if (ar[i] != NULL) {
            rs_triangle_set_destroy(ar[i]);
        };
    };

    rs_mem_free(ar);

};



void rs_triangle_set_destroy(rs_triangle_set_t *dest) {
    rs_mem_free(dest->t);
    rs_mem_free(dest);
};




rs_shape_t* rs_shape_create( int max_rings_count ) {
    rs_shape_t* dest = (rs_shape_t*) rs_mem_alloc( sizeof(rs_shape_t), RS_MEM_POOL_AUTO );
    rs_app_assert_memory(dest, "geom", __LINE__);
    dest->rings = (rs_linestring_t **) rs_mem_alloc( sizeof(rs_linestring_t*) * max_rings_count, RS_MEM_POOL_AUTO);
    rs_app_assert_memory(dest->rings, "geom", __LINE__);
    dest->max_rings_count = max_rings_count;
    dest->rings_count = 0;
    dest->outer_rings_count = 0;
    dest->flags = 0;
    return dest;
};

void rs_shape_destroy( rs_shape_t *dest ) {

    for (int i = 0; i < dest->rings_count; i++) {
        rs_linestring_destroy(dest->rings[i]);
    };

    rs_mem_free(dest->rings);
    rs_mem_free(dest);

};

void rs_shape_append_ring(rs_shape_t *dest, rs_linestring_t *ring) {

    if (dest->rings_count == dest->max_rings_count) {
        dest->max_rings_count++;
        dest->rings = (rs_linestring_t**) rs_mem_realloc( dest->rings, sizeof(rs_linestring_t*) * dest->max_rings_count );
        rs_app_assert_memory(dest->rings, "geom", __LINE__);
    };

    dest->rings[ dest->rings_count++ ] = ring;

};

void rs_shape_calculate_outer_rings(rs_shape_t *dest) {

    dest->outer_rings_count = 0;

    int first_inner_index = -1;

    for (int i = 0; i < dest->rings_count; i++) {

        rs_linestring_t *ls = dest->rings[i];

        int num_points = ls->points_count - 1; // -1 for linear ring, last == first

        float total_angle = 0.0;

        for (int j = 0; j < num_points; j++) {

            rs_point_t pA = ls->p[j];
            rs_point_t pO = ls->p[ (j+1) % num_points ];
            rs_point_t pB = ls->p[ (j+2) % num_points ];

            float angle = atan2( (pA.x-pO.x)*(pO.y-pB.y) - (pA.y-pO.y)*(pO.x-pB.x), (pA.x-pO.x)*(pO.x-pB.x) + (pA.y-pO.y)*(pO.y-pB.y) );
            total_angle += angle;

        };

        if (total_angle > 0.0) {
            // counter-clockwise, outer
            dest->outer_rings_count++;
            if (first_inner_index != -1) {
                // all outer rings must be first, re-sort
                rs_linestring_t *ls_temp = dest->rings[i];
                dest->rings[i] = dest->rings[first_inner_index];
                dest->rings[first_inner_index] = ls_temp;
                first_inner_index++;
            };
        }
        else {
            // clockwise, inner
            if (first_inner_index == -1) {
                first_inner_index = i;
            };
        };

    };


};

rs_shape_t* rs_shape_create_from_static( rs_static_shape_t* static_shape ) {

    rs_shape_t* sh = rs_shape_create( static_shape->rings_count );
    sh->outer_rings_count = static_shape->outer_rings_count;
    sh->flags = static_shape->flags;

    unsigned char *pointer = (unsigned char*) static_shape;
    pointer += sizeof(rs_static_shape_t);

    for (int r = 0; r < static_shape->rings_count; r++) {

        rs_static_linestring_t* static_ls = (rs_static_linestring_t*) pointer;

        rs_linestring_t *ls = rs_linestring_create(static_ls->points_count);

        pointer += sizeof( rs_static_linestring_t );

        for (int i = 0; i < static_ls->points_count; i++) {
            rs_point_t p;
            memcpy(&p, pointer, sizeof(rs_point_t));
            rs_linestring_append_point( ls, p );
            pointer += sizeof(rs_point_t);
        };

        rs_shape_append_ring(sh, ls);
    };

    return sh;

};

unsigned char* rs_static_shape_data_create( rs_shape_t *sh, int *out_content_len ) {

    int total_points_count = 0;
    int total_len = 0;

    for (int r = 0; r < sh->rings_count; r++ ) {
        total_points_count += sh->rings[r]->points_count;
    };

    total_len = sizeof(rs_static_shape_t) + sh->rings_count * sizeof(rs_static_linestring_t) + total_points_count * sizeof(rs_point_t);

    unsigned char *data = (unsigned char*) rs_mem_alloc_adv( total_len, "geom", __LINE__, -1 );
    rs_app_assert_memory(data, "geom", __LINE__);
    unsigned char *data_pointer = data;

    rs_static_shape_t data_shape_hdr;
    data_shape_hdr.rings_count = sh->rings_count;
    data_shape_hdr.outer_rings_count = sh->outer_rings_count;
    data_shape_hdr.flags = 0;

    memcpy(data_pointer, &data_shape_hdr, sizeof(rs_static_shape_t));
    data_pointer += sizeof(rs_static_shape_t);

    for (int r = 0; r < sh->rings_count; r++ ) {

        rs_static_linestring_t bdata_geom_hdr;
        bdata_geom_hdr.points_count = sh->rings[r]->points_count;
        bdata_geom_hdr.flags = 0;

        memcpy(data_pointer, &bdata_geom_hdr, sizeof(rs_static_linestring_t));
        data_pointer += sizeof(rs_static_linestring_t);
        memcpy(data_pointer, sh->rings[r]->p, sizeof(rs_point_t) * bdata_geom_hdr.points_count );
        data_pointer += sizeof(rs_point_t) * bdata_geom_hdr.points_count;
    };

    *out_content_len = total_len;

    return data;

};

rs_shape_t* rs_shape_create_box(float x, float y, float halflength, float halfwidth, float azimuth) {

    // TODO: implement azimuth
    RS_UNUSED_PARAM(azimuth);

    rs_shape_t* sh = rs_shape_create(1);

    rs_linestring_t *ls = rs_linestring_create(4);

    rs_linestring_append_point( ls, rs_vec2( x + halflength, y + halfwidth ) );
    rs_linestring_append_point( ls, rs_vec2( x + halflength, y - halfwidth ) );
    rs_linestring_append_point( ls, rs_vec2( x - halflength, y - halfwidth ) );
    rs_linestring_append_point( ls, rs_vec2( x - halflength, y + halfwidth ) );

    rs_shape_append_ring(sh, ls);

    sh->outer_rings_count = 1;

    return sh;

};


rs_shape_t *rs_shape_create_simplified(rs_shape_t *src, float tolerance) {
    return rs_boost_shape_create_simplified(src, tolerance);
};

rs_shape_t *rs_shape_create_convex_hull(rs_shape_t *src) {

    return rs_boost_shape_create_convex_hull(src);


};


void rs_point_set_destroy(rs_point_set_t *ps) {

    rs_mem_free(ps->p);
    ps->max_points_count = 0;
    ps->points_count = 0;

    rs_mem_free(ps);

};

void rs_point_set_filter_by_distance(rs_point_set_t *ps, float distance) {

    // !!! Warning, this function rearranges points

    for (int i = 0; i < ps->points_count; i++) {
        for (int j = i+1; j < ps->points_count; j++) {
            if ( rs_vec2_distance( ps->p[i].p, ps->p[j].p ) < distance ) {
                ps->points_count--;
                ps->p[j] = ps->p[ ps->points_count ];
                i--;
                break;
            };
        };
    };

};

rs_point_set_t* rs_point_set_create(int max_points_count) {

    rs_point_set_t *ps = (rs_point_set_t*) rs_mem_alloc( sizeof(rs_point_set_t), RS_MEM_POOL_AUTO );
    rs_app_assert_memory(ps, "geom", __LINE__);

    ps->max_points_count = max_points_count;
    ps->points_count = 0;
    ps->p = (rs_angled_point_t*) rs_mem_alloc_adv( ps->max_points_count * sizeof(rs_angled_point_t), "geom", __LINE__, RS_MEM_POOL_AUTO );
    rs_app_assert_memory(ps->p, "geom", __LINE__);

    return ps;

};

void rs_point_set_add_point(rs_point_set_t *ps, rs_angled_point_t ap ) {

    if (ps->points_count+1 >= ps->max_points_count) {
        ps->max_points_count = (ps->max_points_count*2) + 256;
        ps->p = (rs_angled_point_t*) rs_mem_realloc( ps->p, ps->max_points_count * sizeof(rs_angled_point_t) );
        rs_app_assert_memory(ps->p, "geom", __LINE__);
    };

    ps->p[ps->points_count] = ap;
    ps->points_count++;

};

rs_point_set_t * rs_point_set_create_from_quad(float distance, float random_k, float quad_side, int randomizer_flags) {

    RS_UNUSED_PARAM(random_k);
    RS_UNUSED_PARAM(randomizer_flags);

    int side_points = (int) ( (0.998*quad_side) / distance);
    rs_point_set_t *ps = rs_point_set_create(side_points*side_points);
    for (int x = 0; x < side_points; x++) {
        for (int y = 0; y < side_points; y++) {
            rs_angled_point_t ap;
            ap.azimuth = x + 1.7*y;
            ap.p = rs_vec2( 0.001*quad_side + 1.0*distance*x, 0.001*quad_side + 1.0*distance*y );
            ap.p.x = rs_clamp(ap.p.x, 0.001*quad_side, 0.999*quad_side);
            ap.p.y = rs_clamp(ap.p.y, 0.001*quad_side, 0.999*quad_side);
            rs_point_set_add_point( ps, ap );
        };
    };

    return ps;

};

rs_point_set_t * rs_point_set_create_from_quad_random_islands(int islands_count, int points_per_island, float island_radius, float quad_side, int randomizer_flags) {

    RS_UNUSED_PARAM(randomizer_flags);

    int total_points = islands_count * points_per_island;
    rs_point_set_t *ps = rs_point_set_create(total_points);

    for (int ii = 0; ii < islands_count; ii++) {

        float cx = (0.5 + 0.5*rs_noise(ii*2+0, 0)) * quad_side;
        float cy = (0.5 + 0.5*rs_noise(ii*2+1, 0)) * quad_side;

        for (int pi = 0; pi < points_per_island; pi++) {

            rs_angled_point_t ap;
            ap.azimuth = 0.9*ii;
            ap.p.x = cx + island_radius * (0.5 + 0.5*rs_noise(pi*2+0, ii));
            ap.p.y = cy + island_radius * (0.5 + 0.5*rs_noise(pi*2+1, ii));
            ap.p.x = rs_clamp(ap.p.x, 0.002*quad_side, 0.998*quad_side);
            ap.p.y = rs_clamp(ap.p.y, 0.002*quad_side, 0.998*quad_side);
            rs_point_set_add_point( ps, ap );

        };

    };

    return ps;

};


rs_point_set_t* rs_point_set_create_from_linestring(rs_linestring_t *ls, float period1, float side_shift, float angle, int flags) {

    rs_point_set_t *ps = rs_point_set_create(256);

    float period = period1;

    float distance_passed = period;

    float sign = 1.0;

    for ( int ni = 0; ni < ls->points_count + ((flags & POINT_SET_FLAG_CLOSED) ? 1 : 0); ni++ ) {

        if (ni == 0) {
            continue; // !!! TEMPORARY ignore first. TODO: optimize
        };

        int i = ni % ls->points_count;
        int i_prev = (i-1 + ls->points_count) % ls->points_count;

        rs_point_t point = ls->p[i];
        rs_point_t prev_point = ls->p[i_prev];

        rs_vec2_t va = rs_vec2_sub( point, prev_point );
        float azimuth = atan2( va.y, va.x );

        rs_vec2_t shift_vector = rs_vec2( side_shift*cos(M_PI_2+azimuth), side_shift*sin(M_PI_2+azimuth) );

        float d = rs_vec2_distance( point, prev_point );

        if (distance_passed + d < period) {
            distance_passed += d;
            continue;
        };

        if (distance_passed + d > period) {

            int num = (int) ( (distance_passed+d) / period );

            int ks = period - distance_passed;

            for (int ni = 0; ni < num; ni++) {

                float r_k = 0.0;
                if (flags & POINT_SET_FLAG_RANDOMIZER_TYPE_SIMPLE) {
                    r_k = 0.4*(rs_noise(ni, (int)(angle*17.0 + side_shift*5.0) ));
                };

                float k = ( period*(1.0*ni+r_k) + ks ) / d;

                rs_point_t emit_point = rs_vec2_linear( prev_point, point, k );
                emit_point = rs_vec2_add( emit_point, rs_vec2_mult(shift_vector, sign) );

                rs_angled_point_t apoint;
                apoint.p = rs_vec2( emit_point.x, emit_point.y );
                apoint.azimuth = azimuth+angle*sign;


                rs_point_set_add_point( ps, apoint );

                if (flags & POINT_SET_FLAG_ZIGZAG) {
                    sign = -sign;
                };

            };

            distance_passed += d;
            distance_passed -= period*num;

        };


    };

    return ps;

};

void rs_point_set_make_azimuths_smooth(rs_point_set_t *ps) {
    if (ps->points_count < 2) {
        return;
    };

    for (int pi = 0; pi < ps->points_count; pi++) {
        if (pi == 0) {
            ps->p[pi].azimuth = atan2( ps->p[1].p.y - ps->p[0].p.y, ps->p[1].p.x - ps->p[0].p.x );
        }
        else if (pi == ps->points_count-1) {
            ps->p[pi].azimuth = atan2( ps->p[pi].p.y - ps->p[pi-1].p.y, ps->p[pi].p.x - ps->p[pi-1].p.x );
        }
        else {
            ps->p[pi].azimuth = atan2( ps->p[pi+1].p.y - ps->p[pi-1].p.y, ps->p[pi+1].p.x - ps->p[pi-1].p.x );
        };
    };

};


float rs_triangle_area(rs_triangle_t *tr) {

    return fabs( tr->p[0].x * (tr->p[1].y - tr->p[2].y) + tr->p[1].x * (tr->p[2].y - tr->p[0].y) + tr->p[2].x * (tr->p[0].y - tr->p[1].y)) / 2.0;

};

float rs_shape_area(rs_shape_t *sh) {

    // TODO: optimize

    float area = 0.0;

    rs_triangle_set_t *tset = rs_triangle_set_create_triangulated(sh);

    for (int ti = 0; ti < tset->t_count; ti++) {
        area += rs_triangle_area( &(tset->t[ti]) );
    };

    rs_triangle_set_destroy(tset);

    return area;
};


float get_sign3 (rs_point_t p1, rs_point_t p2, rs_point_t p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}


int rs_is_point_in_triangle(rs_point_t p, rs_triangle_t *tr) {

    bool b1, b2, b3;

    b1 = get_sign3(p, tr->p[0], tr->p[1]) < 0.0f;
    b2 = get_sign3(p, tr->p[1], tr->p[2]) < 0.0f;
    b3 = get_sign3(p, tr->p[2], tr->p[0]) < 0.0f;

    return ((b1 == b2) && (b2 == b3));

};

int rs_is_point_in_linestring(rs_point_t p, rs_linestring_t *ls, float threshold) {

	// TODO: optimize

    int r = 0;

    rs_shape_t *sh = rs_shape_create_buffered_from_linestring(ls, threshold, RS_POLYGON_CLOSED );

    rs_triangle_set_t *tset = rs_triangle_set_create_triangulated(sh);

    for (int ti = 0; ti < tset->t_count; ti++) {
        if (rs_is_point_in_triangle(p,  &tset->t[ti] )) {
            r = 1;
            break;
        };
    };

    rs_triangle_set_destroy(tset);

    rs_shape_destroy(sh);

    return r;

};



rs_point_set_t* rs_point_set_create_from_triangle(rs_triangle_t *tr, float distance, float random_k) {

    random_k /= distance;

    rs_point_set_t *ps;

    rs_vec2_t p0 = tr->p[0];
    rs_vec2_t dp1 = rs_vec2_sub( tr->p[1], p0 );
    rs_vec2_t dp2 = rs_vec2_sub( tr->p[2], p0 );

    int c1 = 1 + (int)(rs_vec2_length(dp1) / distance);
    int c2 = 1 + (int)(rs_vec2_length(dp2) / distance);

    ps = rs_point_set_create( (c1+1) * (c2+1) );

    for (int i1 = 0; i1 < c1; i1++) {
        for (int i2 = 0; i2 < (c2 - i1*c2/c1); i2++) {

            float k1 = 1.0*i1/(c1);
            float k2 = 1.0*i2/(c2);

            k1 += random_k / c1 * rs_noise(i1, i2);
            k2 += random_k / c1 * rs_noise(i1, i2);

            rs_angled_point_t ap;
            ap.azimuth = 0.0;
            ap.p = rs_vec2_add( p0, rs_vec2_add( rs_vec2_mult(dp1, k1), rs_vec2_mult(dp2, k2) ) );

            rs_point_set_add_point( ps, ap );

        };
    };

    return ps;

};




void rs_shape_analyze_metadata( rs_shape_metadata_t* metadata, rs_shape_t *sh ) {


    memset(metadata, 0, sizeof(rs_shape_metadata_t));


    if (sh->rings_count == 0) {
        metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_COMPLEX; // No shape at all
        return;
    };

    rs_linestring_t *ls = sh->rings[0];

    float xmin = ls->p[0].x;
    float xmax = ls->p[0].x;
    float ymin = ls->p[0].y;
    float ymax = ls->p[0].y;

    rs_vec2_t gc = ls->p[0];

    for (int i = 1; i < ls->points_count; i++) {
        gc = rs_vec2_add(gc, ls->p[i]);
        xmin = rs_min( ls->p[i].x, xmin );
        xmax = rs_max( ls->p[i].x, xmax );
        ymin = rs_min( ls->p[i].y, ymin );
        ymax = rs_max( ls->p[i].y, ymax );
    };

    rs_vec2_t c = rs_vec2( 0.5*(xmin + xmax), 0.5*(ymin + ymax) );
    metadata->center = c;

    gc = rs_vec2_mult(gc, 1.0 / ls->points_count);

    float bb_radius = 0.5 * rs_max( ymax - ymin, xmax - xmin );

    metadata->radius = bb_radius;



    if (sh->rings_count > 1) {
        metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_COMPLEX;
        return;
    };

    if (ls->points_count < 3) {
        metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_COMPLEX;
        return;
    };


    if (ls->points_count == 4) {

        // dot product will be about 0.0 for 90-deg

        rs_vec2_t edge0 = rs_vec2_sub(ls->p[1], ls->p[0]);
        rs_vec2_t edge1 = rs_vec2_sub(ls->p[2], ls->p[1]);
        rs_vec2_t edge2 = rs_vec2_sub(ls->p[3], ls->p[2]);
        rs_vec2_t edge3 = rs_vec2_sub(ls->p[0], ls->p[3]);

        float a1 = fabs( rs_vec2_dot(edge0, edge1) );
        float a2 = fabs( rs_vec2_dot(edge2, edge3) );

        const float rect_threshold = 0.2;

        if ( (a1 < rect_threshold) && (a2 < rect_threshold) ) {

            metadata->shape_type = RS_SHAPE_TYPE_RECTANGLE;

            float edge0_len = rs_vec2_length(edge0);
            float edge1_len = rs_vec2_length(edge1);
            float edge0_az = atan2(edge0.y, edge0.x);
            float edge1_az = atan2(edge1.y, edge1.x);

            if (edge0_len > edge1_len) {
                metadata->halflength = 0.5*edge0_len;
                metadata->halfwidth = 0.5*edge1_len;
                metadata->azimuth = edge0_az;
            }
            else {
                metadata->halflength = 0.5*edge1_len;
                metadata->halfwidth = 0.5*edge0_len;
                metadata->azimuth = edge1_az;
            };

            return;

        }
        else {

            // skewed parallelogam or something
            metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONVEX; // !!! warning, not any 4-point shape is convex, but in most cases it is. dangerous.

            return;
        };

    };


    // checking for roundness
    if (ls->points_count > 7) {

        float nominal_radius = rs_vec2_distance( ls->p[0], c );

        float rmin = nominal_radius * 15.0;
        float rmax = 0.0;

        float dmin = nominal_radius * 15.0;
        float dmax = 0.0;

        for (int i = 1; i < ls->points_count; i++) {
            float r = rs_vec2_distance(ls->p[i], c);
            float d = rs_vec2_distance(ls->p[i], ls->p[i-1]);
            rmin = rs_min(rmin, r);
            rmax = rs_max(rmax, r);
            dmin = rs_min(dmin, d);
            dmax = rs_max(dmax, d);
        };

        float ravg = 0.5*(rmax + rmin);


        if ( ((rmax - rmin) / nominal_radius) < 0.08 ) {

            if (dmax < ravg) {

                metadata->radius = ravg;
                metadata->shape_type = RS_SHAPE_TYPE_ROUND;
                return;

            };

        };

    };

    // checking for convex shape
    // from here: https://gis.stackexchange.com/questions/157567/testing-if-geometry-is-convex-using-jts

    metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONVEX;

    int sign = 0;
    for (int i = 1; i < ls->points_count; i++) {

        rs_point_t c0 = ls->p[ ((i-1)+ls->points_count) % ls->points_count ];
        rs_point_t c1 = ls->p[ ((i  )+ls->points_count) % ls->points_count ];
        rs_point_t c2 = ls->p[ ((i+1)+ls->points_count) % ls->points_count ];

        double dx1 = c1.x - c0.x;
        double dy1 = c1.y - c0.y;
        double dx2 = c2.x - c1.x;
        double dy2 = c2.y - c2.y;
        double z = dx1 * dy2 - dy1 * dx2;
        int s = (z >= 0.0) ? 1 : -1;
        if (sign == 0) {
            sign = s;
        }
        else if (sign != s) {
            metadata->shape_type = RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONCAVE;

            break;
        }
    }

    if (metadata->shape_type == RS_SHAPE_TYPE_UNKNOWN_SIMPLE_CONCAVE) {

        // TODO: check if it's "almost" concave, safe for approximation

        return;
    };

    // Convex
    return;


};

float rs_distance_to_segment(rs_vec2_t v, rs_vec2_t w, rs_vec2_t p) {
    // from here: https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
    // Return minimum distance between line segment vw and point p
    const float l2 = rs_vec2_distance_sqr(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 < 0.00001) {
        return rs_vec2_distance(p, v);   // v == w case
    };
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    const float t = rs_max(0.0, rs_min(1.0, rs_vec2_dot( rs_vec2_sub(p,v), rs_vec2_sub(w,v) ) / l2));
    const rs_vec2_t projection = rs_vec2_add(v, rs_vec2_mult( rs_vec2_sub(w,v), t));  // Projection falls on the segment
    return rs_vec2_distance(p, projection);
}

float rs_linestring_distance_from_point(rs_linestring_t *ls, rs_point_t p) {

    float min_distance = 11000.0;

    for (int i = 1; i < ls->points_count; i++) {

        min_distance = rs_min(min_distance, rs_distance_to_segment( ls->p[i-1], ls->p[i], p) );

    };

    return min_distance;

};

void rs_linestring_printf(rs_linestring_t *ls) {

    DEBUG10f("Linestring (%d points): ", ls->points_count);

    for (int i = 0; i < ls->points_count; i++) {
        DEBUG10f("(%.2f, %.2f) ", ls->p[i].x, ls->p[i].y);
    }
    DEBUG10f("\n");

};

void rs_shape_printf(rs_shape_t *sh) {
    DEBUG10f("Shape (%d rings) (%d outers) \n", sh->rings_count, sh->outer_rings_count);
    for (int ri = 0; ri < sh->rings_count; ri++) {
        DEBUG10f("// Ring %d: \n", ri);
        rs_linestring_printf(sh->rings[ri]);
    };
};

int rs_shape_get_total_points(rs_shape_t *sh) {

    int r = 0;
    for (int ri = 0; ri < sh->rings_count; ri++) {
        r += sh->rings[ri]->points_count;
    };

    return r;

};

float rs_shape_perimeter(rs_shape_t *sh) {

    float perimeter = 0.0;
    for (int ri = 0; ri < sh->rings_count; ri++) {
        rs_linestring_t *ls = sh->rings[ri];

        for (int pi = 1; pi < ls->points_count; pi++) {
            perimeter += rs_vec2_distance( ls->p[pi-1], ls->p[pi] );
        };

    };

    return perimeter;

};

rs_angled_point_t rs_shape_get_middle_angled_point(rs_shape_t *sh) {

    rs_angled_point_t ap;
    ap.azimuth = 0.0;
    ap.p = rs_vec2(0.0, 0.0);

    if (sh->rings_count < 1) {
        return ap;
    };

    rs_linestring_t *ls = sh->rings[0];

    if (ls->points_count < 2) {
        return ap;
    };

    ap.p = rs_vec2_linear(ls->p[0], ls->p[ls->points_count-1], 0.5);
    ap.azimuth = rs_vec2_azimuth(ls->p[0], ls->p[ls->points_count-1]);

    return ap;

};

rs_angled_point_t rs_linestring_get_middle_angled_point(rs_linestring_t *ls) {

    rs_angled_point_t ap;
    ap.azimuth = 0.0;
    ap.p = rs_vec2(0.0, 0.0);

    if (ls->points_count < 2) {
        return ap;
    };


    float dist = 0.0;
    int result_i = 0;
    for (int i = 1; i < ls->points_count; i++) {
        float new_dist = rs_vec2_distance(ls->p[i], ls->p[i-1]);
        if ( new_dist > dist ) {
            dist = new_dist;
            result_i = i-1;
        };
    };

    ap.p = rs_vec2_linear(ls->p[result_i], ls->p[result_i+1], 0.5);
    ap.azimuth = rs_vec2_azimuth(ls->p[result_i], ls->p[result_i+1]);

    return ap;

};

rs_point_t rs_linestring_get_middle_point(rs_linestring_t *ls) {
    rs_point_t p;
    p = rs_vec2(0.0, 0.0);

    if (ls->points_count < 2) {
        return p;
    };

    for (int i = 0; i < ls->points_count; i++) {
        p = rs_vec2_add(p, ls->p[i]);
    };

    p.x /= ls->points_count;
    p.y /= ls->points_count;

    return p;
};

rs_point_t rs_shape_get_middle_point(rs_shape_t *sh) {


    rs_point_t p;
    p = rs_vec2(0.0, 0.0);

    if (sh->rings_count < 1) {
        return p;
    };

    rs_linestring_t *ls = sh->rings[0];

    return rs_linestring_get_middle_point(ls);

};



int test_orientation(rs_point_t p, rs_point_t q, rs_point_t r)
{
    float val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);


    if (val == 0) return 0;  // colinear
    return (val > 0) ? 1 : 2; // clock or counterclock wise
}


rs_linestring_t *rs_linestring_create_convex_hull(rs_linestring_t *src_ls) {

    // from here: https://www.geeksforgeeks.org/convex-hull-set-1-jarviss-algorithm-or-wrapping/

    rs_linestring_t *ls = rs_linestring_create(src_ls->points_count + 5);

    int n = src_ls->points_count;

    // Find the leftmost point
    int l = 0;
    for (int i = 1; i < src_ls->points_count; i++) {
        if (src_ls->p[i].x < src_ls->p[l].x) {
            l = i;
        };
    };

    int cntr=0;

    // Start from leftmost point, keep moving counterclockwise
    // until reach the start point again.  This loop runs O(h)
    // times where h is number of points in result or output.
    int p = l, q;
    do
    {
        // Add current point to result
        rs_linestring_append_point(ls, src_ls->p[p] );

        // Search for a point 'q' such that orientation(p, x,
        // q) is counterclockwise for all points 'x'. The idea
        // is to keep track of last visited most counterclock-
        // wise point in q. If any point 'i' is more counterclock-
        // wise than q, then update q.
        q = (p+1)%n;
        for (int i = 0; i < n; i++)
        {
           // If i is more counterclockwise than current q, then
           // update q

           if (test_orientation(src_ls->p[p], src_ls->p[i], src_ls->p[q]) == 2)
               q = i;
        }

        // Now q is the most counterclockwise with respect to p
        // Set p as q for next iteration, so that q is added to
        // result 'hull'
        p = q;

        cntr++;


    } while ( (p != l) && (cntr < 100000*n) );  // While we don't come to first point

    return ls;

};


