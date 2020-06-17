#include "rvtutil.h"

#include "rvtapp.h"

#include <string.h>
#include <math.h>

#include "rs/rsdebug.h"


int rvt_point_is_in_area(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags, rvt_gtriangle_t **gtriangle_result_p) {

    // Warning, near subtiles can affect
    int point_subtile_i = rvt_get_subtile_i(p.x, p.y);

    int subtile_ix = point_subtile_i % RVT_SUBTILES_SIDE_COUNT;
    int subtile_iy = point_subtile_i / RVT_SUBTILES_SIDE_COUNT;


    int loading_order[ 9*2 ] = { 0,0,   -1,0,  1,0,  0,1,  0,-1,      -1,-1, -1,1,   1,-1,  1,1,  };


    int visible_radius = 1; // Must be 1

    int st_ix_start = subtile_ix - visible_radius;
    int st_iy_start = subtile_iy - visible_radius;


    for (int ai = 0; ai < 9; ai++) {

        int r_i = loading_order[ai*2 + 0] + visible_radius;
        int r_j = loading_order[ai*2 + 1] + visible_radius;

        int st_ix = rs_cyclic_mod_i(st_ix_start + r_j, RVT_SUBTILES_SIDE_COUNT);
        int st_iy = rs_cyclic_mod_i(st_iy_start + r_i, RVT_SUBTILES_SIDE_COUNT);

        int subtile_i = st_ix + RVT_SUBTILES_SIDE_COUNT*st_iy;

        for (int li = 0; li < rvt_app_get_geodata()->gtriangle_list_count[subtile_i][layer_i]; li++) {
            for (int i = 0; i < rvt_app_get_geodata()->gtriangle_items_count[subtile_i][layer_i][li]; i++ ) {

                rvt_gtriangle_t *gtriangle = &rvt_app_get_geodata()->gtriangle[subtile_i][layer_i][li][i];

                if (!(gtriangle->flags & required_flags)) {
                    continue;
                };

                if (gtriangle->flags & exclude_flags) {
                    continue;
                };

                if ( rs_is_point_in_triangle(p, &gtriangle->triangle) ) {
                    if (gtriangle_result_p) {
                        *gtriangle_result_p = (rvt_gtriangle_t*) &(gtriangle->triangle);
                    };
                    return 1;
                };

            };
        };

    };

    return 0;

};



void rvt_gp_add(int layer_i, int dest_tile_i, rs_point_t pos, float azimuth, void *pdata, int flags, float radius) {


    int subtile_i = rvt_get_subtile_i_limited_by_tile_i(pos.x, pos.y, dest_tile_i);

    int ar_i = rvt_app_get_geodata()->gp_list_count[subtile_i][layer_i] - 1;

    if (ar_i == -1) {
        rvt_app_get_geodata()->gp_list_count[subtile_i][layer_i] = 1;
        ar_i = 0;
        rvt_app_get_geodata()->gp_items_count[subtile_i][layer_i][ar_i] = 0;
    };

    int ar_c = rvt_app_get_geodata()->gp_items_count[subtile_i][layer_i][ar_i];

    if (ar_c + 1 > RVT_GP_MAX_ARRAY_ITEMS_COUNT) {
        ar_i = ++rvt_app_get_geodata()->gp_list_count[subtile_i][layer_i];
        if (ar_i >= RVT_GP_MAX_ARRAYS_COUNT) {
            rs_critical_alert_and_halt("Max GP arrays count has been reached. ");
        };
        ar_c = 0;
        rvt_app_get_geodata()->gp_items_count[subtile_i][layer_i][ar_i] = 0;
    };

    if ( rvt_app_get_geodata()->gp[subtile_i][layer_i][ar_i] == NULL ) {
        rvt_app_get_geodata()->gp[subtile_i][layer_i][ar_i] = (rvt_gpoint_t*) rs_mem_alloc_adv( RVT_GP_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gpoint_t), "rvtutil", __LINE__, RVT_MEM_POOL_GPOINTS_ARRAY );
        rs_app_assert_memory( rvt_app_get_geodata()->gp[subtile_i][layer_i][ar_i], "rvtutil", __LINE__ );
        ar_c = 0;
    };

    rvt_gpoint_t *p = &rvt_app_get_geodata()->gp[subtile_i][layer_i][ar_i][ar_c];

    p->pos = pos;
    p->azimuth = azimuth;
    p->pdata = pdata;
    p->flags = flags;
    p->radius = radius;

    rvt_app_get_geodata()->gp_items_count[subtile_i][layer_i][ar_i]++;

};




void rvt_gline_add(int layer_i, int dest_tile_i, rs_point_t p1, rs_point_t p2, gd_lines_data_t *pdata, int flags, float thickness) {

    int subtile_i = rvt_get_subtile_i_limited_by_tile_i(p1.x, p1.y, dest_tile_i);

    int ar_i = rvt_app_get_geodata()->gline_list_count[subtile_i][layer_i] - 1;

    if (ar_i == -1) {
        rvt_app_get_geodata()->gline_list_count[subtile_i][layer_i] = 1;
        ar_i = 0;
        rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][ar_i] = 0;
    };

    int ar_c = rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][ar_i];

    if (ar_c + 1 > RVT_GLINE_MAX_ARRAY_ITEMS_COUNT) {
        ar_i = ++rvt_app_get_geodata()->gline_list_count[subtile_i][layer_i];
        if (ar_i >= RVT_GLINE_MAX_ARRAYS_COUNT) {
            rs_critical_alert_and_halt("Max GLINE arrays count has been reached. ");
        };
        ar_c = 0;
        rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][ar_i] = 0;
    };


    if ( rvt_app_get_geodata()->gline[subtile_i][layer_i][ar_i] == NULL ) {
        rvt_app_get_geodata()->gline[subtile_i][layer_i][ar_i] = (rvt_gline_t*) rs_mem_alloc_adv( RVT_GLINE_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gline_t), "rvtutil", __LINE__, RVT_MEM_POOL_GLINES_ARRAY );
        rs_app_assert_memory( rvt_app_get_geodata()->gline[subtile_i][layer_i][ar_i], "rvtutil", __LINE__ );
        ar_c = 0;
    };

    rvt_gline_t *l = &rvt_app_get_geodata()->gline[subtile_i][layer_i][ar_i][ar_c];

    l->p1 = p1;
    l->p2 = p2;
    l->thickness = thickness;
    l->pdata = pdata;
    l->flags = flags;
    l->length = rs_vec2_distance(p1, p2);


    rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][ar_i]++;

};

void rvt_gline_add_shape(int layer_i, int dest_tile_i, float shift_x, float shift_y, rs_shape_t *src_sh, gd_lines_data_t *pdata, int flags, float thickness, int is_already_segmentized) {


    rs_vec2_t shift_p = rs_vec2(shift_x, shift_y);

    rs_shape_t *sh = src_sh;

    for (int ri = 0; ri < sh->rings_count; ri++) {

        for (int i = 1; i < sh->rings[ri]->points_count; i++) {
            rvt_gline_add(layer_i, dest_tile_i, rs_vec2_add(shift_p, sh->rings[ri]->p[i-1]), rs_vec2_add(shift_p, sh->rings[ri]->p[i]), pdata, flags, thickness );
        };

    };


    if (!is_already_segmentized) {
       rs_shape_destroy(sh);
    };


};


void rvt_gtriangle_add(int layer_i, int dest_tile_i, int flags, float shift_x, float shift_y, rs_triangle_t *src_triangle) {


    int subtile_i = rvt_get_subtile_i_limited_by_tile_i( shift_x + (src_triangle->p[0].x + src_triangle->p[1].x + src_triangle->p[2].x)/3,
                        shift_y + (src_triangle->p[0].y + src_triangle->p[1].y + src_triangle->p[2].y)/3, dest_tile_i );


    int ar_i = rvt_app_get_geodata()->gtriangle_list_count[subtile_i][layer_i] - 1;

    if (ar_i == -1) {
        rvt_app_get_geodata()->gtriangle_list_count[subtile_i][layer_i] = 1;
        ar_i = 0;
        rvt_app_get_geodata()->gtriangle_items_count[subtile_i][layer_i][ar_i] = 0;
    };

    int ar_c = rvt_app_get_geodata()->gtriangle_items_count[subtile_i][layer_i][ar_i];

    if (ar_c + 1 > RVT_GTRIANGLE_MAX_ARRAY_ITEMS_COUNT) {
        ar_i = ++rvt_app_get_geodata()->gtriangle_list_count[subtile_i][layer_i];
        if (ar_i >= RVT_GTRIANGLE_MAX_ARRAYS_COUNT) {
            rs_critical_alert_and_halt("Max GTRIANGLE arrays count has been reached. ");
        };
        ar_c = 0;
        rvt_app_get_geodata()->gtriangle_items_count[subtile_i][layer_i][ar_i] = 0;
    };


    if ( rvt_app_get_geodata()->gtriangle[subtile_i][layer_i][ar_i] == NULL ) {
        rvt_app_get_geodata()->gtriangle[subtile_i][layer_i][ar_i] = (rvt_gtriangle_t*) rs_mem_alloc_adv( RVT_GTRIANGLE_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gtriangle_t), "rvtutil", __LINE__, RVT_MEM_POOL_GTRIANGLES_ARRAY );
        rs_app_assert_memory( rvt_app_get_geodata()->gtriangle[subtile_i][layer_i][ar_i], "rvtutil", __LINE__ );
        ar_c = 0;
    };

    rvt_gtriangle_t *tr = &rvt_app_get_geodata()->gtriangle[subtile_i][layer_i][ar_i][ar_c];

    tr->triangle.p[0].x = src_triangle->p[0].x + shift_x;
    tr->triangle.p[0].y = src_triangle->p[0].y + shift_y;
    tr->triangle.p[1].x = src_triangle->p[1].x + shift_x;
    tr->triangle.p[1].y = src_triangle->p[1].y + shift_y;
    tr->triangle.p[2].x = src_triangle->p[2].x + shift_x;
    tr->triangle.p[2].y = src_triangle->p[2].y + shift_y;

    tr->flags = flags;

    rvt_app_get_geodata()->gtriangle_items_count[subtile_i][layer_i][ar_i]++;

};


void rvt_gtriangle_add_triangle_set(int layer_i, int dest_tile_i, int flags, float shift_x, float shift_y, rs_triangle_set_t *src_tset) {

    for (int ti = 0; ti < src_tset->t_count; ti++) {
        rvt_gtriangle_add(layer_i, dest_tile_i, flags, shift_x, shift_y, &src_tset->t[ti] );
    };

};





int rvt_get_subtile_i_limited_by_tile_i(float x, float z, int dest_tile_i) {

    float tile_x_min = rvt_app_get_sc() * rvt_app_get_geodata()->tile[dest_tile_i].tile_x;
    float tile_x_max = rvt_app_get_sc() * (rvt_app_get_geodata()->tile[dest_tile_i].tile_x + 1);

    float tile_y_min = rvt_app_get_sc() * rvt_app_get_geodata()->tile[dest_tile_i].tile_y;
    float tile_y_max = rvt_app_get_sc() * (rvt_app_get_geodata()->tile[dest_tile_i].tile_y + 1);

    x = rs_clamp(x, tile_x_min+0.01*rvt_app_get_sc(), tile_x_max-0.01*rvt_app_get_sc());
    z = rs_clamp(z, tile_y_min+0.01*rvt_app_get_sc(), tile_y_max-0.01*rvt_app_get_sc());

    int subtile_i = rvt_get_subtile_i(x, z);

    return subtile_i;


};

int rvt_get_tile_i_by_subtile_i(int subtile_i) {

    int subtile_ix = subtile_i % RVT_SUBTILES_SIDE_COUNT;
    int subtile_iy = subtile_i / RVT_SUBTILES_SIDE_COUNT;

    int tile_ix = subtile_ix / RVT_SUBTILES_SIDE_COUNT_PER_TILE;
    int tile_iy = subtile_iy / RVT_SUBTILES_SIDE_COUNT_PER_TILE;

    int tile_i = tile_iy*RVT_TILES_SIDE_COUNT + tile_ix;

    return tile_i;


};



int rvt_get_tile_i(float x, float z) {

    x /= rvt_app_get_sc();
    z /= rvt_app_get_sc();

    x = rs_cyclic_mod_f( x, RVT_TILES_SIDE_COUNT );
    z = rs_cyclic_mod_f( z, RVT_TILES_SIDE_COUNT );

    int tile_x = (int) x;
    int tile_y = (int) z;

    int tile_ix = rs_cyclic_mod_i(tile_x, RVT_TILES_SIDE_COUNT);
    int tile_iy = rs_cyclic_mod_i(tile_y, RVT_TILES_SIDE_COUNT);

    int tile_i = tile_iy*RVT_TILES_SIDE_COUNT + tile_ix;

    return tile_i;


};


int rvt_get_subtile_i(float x, float z) {


    x /= rvt_app_get_sc();
    z /= rvt_app_get_sc();


    x = rs_cyclic_mod_f( x*RVT_SUBTILES_SIDE_COUNT_PER_TILE, RVT_SUBTILES_SIDE_COUNT );
    z = rs_cyclic_mod_f( z*RVT_SUBTILES_SIDE_COUNT_PER_TILE, RVT_SUBTILES_SIDE_COUNT );

    int subtile_x = (int) x;
    int subtile_y = (int) z;

    int subtile_ix = rs_cyclic_mod_i(subtile_x, RVT_SUBTILES_SIDE_COUNT);
    int subtile_iy = rs_cyclic_mod_i(subtile_y, RVT_SUBTILES_SIDE_COUNT);

    int subtile_i = subtile_iy*RVT_SUBTILES_SIDE_COUNT + subtile_ix;

    return subtile_i;


};




rvt_gpoint_t* rvt_gp_find_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags) {

    int point_subtile_i = rvt_get_subtile_i(p.x, p.y);

    float distance_sqr = RS_SQR(11000.0);
    rvt_gpoint_t *result = NULL;

    int subtile_ix = point_subtile_i % RVT_SUBTILES_SIDE_COUNT;
    int subtile_iy = point_subtile_i / RVT_SUBTILES_SIDE_COUNT;


    int loading_order[ 9*2 ] = { 0,0,   -1,0,  1,0,  0,1,  0,-1,      -1,-1, -1,1,   1,-1,  1,1,  };

    int visible_radius = 1; // Must be 1

    int st_ix_start = subtile_ix - visible_radius;
    int st_iy_start = subtile_iy - visible_radius;


    for (int ai = 0; ai < 9; ai++) {

        int r_i = loading_order[ai*2 + 0] + visible_radius;
        int r_j = loading_order[ai*2 + 1] + visible_radius;

        int st_ix = rs_cyclic_mod_i(st_ix_start + r_j, RVT_SUBTILES_SIDE_COUNT);
        int st_iy = rs_cyclic_mod_i(st_iy_start + r_i, RVT_SUBTILES_SIDE_COUNT);

        int subtile_i = st_ix + RVT_SUBTILES_SIDE_COUNT*st_iy;

        for (int li = 0; li < rvt_app_get_geodata()->gp_list_count[subtile_i][layer_i]; li++) {
            for (int i = 0; i < rvt_app_get_geodata()->gp_items_count[subtile_i][layer_i][li]; i++ ) {

                rvt_gpoint_t *gpoint = &rvt_app_get_geodata()->gp[subtile_i][layer_i][li][i];

                if (!(gpoint->flags & required_flags)) {
                    continue;
                };

                if (gpoint->flags & exclude_flags) {
                    continue;
                };

                float current_distance_sqr = rs_vec2_distance_sqr(p, gpoint->pos) - gpoint->radius;
                if (current_distance_sqr < distance_sqr) {
                    distance_sqr = current_distance_sqr;
                    result = gpoint;
                };
            };
        };


    };

    return result; // may return NULL
};

float rvt_gp_distance_to_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags) {

    rvt_gpoint_t *gp = rvt_gp_find_nearest(p, layer_i, required_flags, exclude_flags);
    if (gp != NULL) {
        return rs_vec2_distance(p, gp->pos) - gp->radius;
    };
    return 11000.0; // not found

};


float distance_to_gline(rs_vec2_t p, rvt_gline_t *gline) {

    rs_vec2_t v = gline->p1;
    rs_vec2_t w = gline->p2;

    const float len_sqr = rs_vec2_distance_sqr(v, w);
    if (len_sqr < 0.000001) {
        return rs_vec2_distance(p, v);
    };

    float t = rs_clamp( rs_vec2_dot( rs_vec2(p.x-v.x, p.y-v.y), rs_vec2(w.x-v.x, w.y-v.y) )/len_sqr, 0.0, 1.0 );
    rs_vec2_t v_projection = rs_vec2_add(v, rs_vec2_mult(rs_vec2_sub(w, v), t) );

    return rs_vec2_distance(p, v_projection);

};


float rvt_near_glines_length(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags, float radius) {

    float total_length = 0.0;

    // Near subtiles can affect
    int point_subtile_i = rvt_get_subtile_i(p.x, p.y);

    int subtile_ix = point_subtile_i % RVT_SUBTILES_SIDE_COUNT;
    int subtile_iy = point_subtile_i / RVT_SUBTILES_SIDE_COUNT;


    int loading_order[ 9*2 ] = { 0,0,   -1,0,  1,0,  0,1,  0,-1,      -1,-1, -1,1,   1,-1,  1,1,  };


    int visible_radius = 1; // Must be 1

    int st_ix_start = subtile_ix - visible_radius;
    int st_iy_start = subtile_iy - visible_radius;

    for (int ai = 0; ai < 9; ai++) { // 9 or 1

        int r_i = loading_order[ai*2 + 0] + visible_radius;
        int r_j = loading_order[ai*2 + 1] + visible_radius;

        int st_ix = rs_cyclic_mod_i(st_ix_start + r_j, RVT_SUBTILES_SIDE_COUNT);
        int st_iy = rs_cyclic_mod_i(st_iy_start + r_i, RVT_SUBTILES_SIDE_COUNT);

        int subtile_i = st_ix + RVT_SUBTILES_SIDE_COUNT*st_iy;


        for (int li = 0; li < rvt_app_get_geodata()->gline_list_count[subtile_i][layer_i]; li++) {
            for (int i = 0; i < rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][li]; i++ ) {

                rvt_gline_t *gline = &rvt_app_get_geodata()->gline[subtile_i][layer_i][li][i];

                if (!(gline->flags & required_flags)) {
                    continue;
                };

                if (gline->flags & exclude_flags) {
                    continue;
                };

                float current_distance = distance_to_gline(p, gline) - gline->thickness;
                if (current_distance < radius) {
                    total_length += gline->length;
                };

            };
        };

    };

    return total_length;


};

float rvt_gline_find_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags, rvt_gline_t **gline_result_p) {

    // Near subtiles can affect
    int point_subtile_i = rvt_get_subtile_i(p.x, p.y);

    float distance_sqr = RS_SQR(10000.0);
    rvt_gline_t *result = NULL;

    int subtile_ix = point_subtile_i % RVT_SUBTILES_SIDE_COUNT;
    int subtile_iy = point_subtile_i / RVT_SUBTILES_SIDE_COUNT;


    int loading_order[ 9*2 ] = { 0,0,   -1,0,  1,0,  0,1,  0,-1,      -1,-1, -1,1,   1,-1,  1,1,  };


    int visible_radius = 1; // Must be 1

    int st_ix_start = subtile_ix - visible_radius;
    int st_iy_start = subtile_iy - visible_radius;


    for (int ai = 0; ai < 9; ai++) { // 9 or 1

        int r_i = loading_order[ai*2 + 0] + visible_radius;
        int r_j = loading_order[ai*2 + 1] + visible_radius;

        int st_ix = rs_cyclic_mod_i(st_ix_start + r_j, RVT_SUBTILES_SIDE_COUNT);
        int st_iy = rs_cyclic_mod_i(st_iy_start + r_i, RVT_SUBTILES_SIDE_COUNT);

        int subtile_i = st_ix + RVT_SUBTILES_SIDE_COUNT*st_iy;

        for (int li = 0; li < rvt_app_get_geodata()->gline_list_count[subtile_i][layer_i]; li++) {
            for (int i = 0; i < rvt_app_get_geodata()->gline_items_count[subtile_i][layer_i][li]; i++ ) {

                rvt_gline_t *gline = &rvt_app_get_geodata()->gline[subtile_i][layer_i][li][i];

                if (!(gline->flags & required_flags)) {
                    continue;
                };

                if (gline->flags & exclude_flags) {
                    continue;
                };

                float current_distance_sqr = distance_to_gline(p, gline) - gline->thickness; // - gline->thickness  //  rs_vec2_distance_sqr(p, gpoint->pos) - gpoint->radius;
                if (current_distance_sqr < distance_sqr) {
                    distance_sqr = current_distance_sqr;
                    result = gline;
                };


            };
        };

    };

    if (gline_result_p) {
        *gline_result_p = result;
    };

    if (distance_sqr < 0.0) {
        return 0.0;
    };

    return sqrtf(distance_sqr); // TODO: Something wrong is here, distance is actually not squared

};



float calc_barycentric(rs_vec3_t p1, rs_vec3_t p2, rs_vec3_t p3, float x, float z) {
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);

    float l1 = ((p2.z - p3.z) * (x - p3.x) + (p3.x - p2.x) * (z - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (x - p3.x) + (p1.x - p3.x) * (z - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;

    float res = l1 * p1.y + l2 * p2.y + l3 * p3.y;

    return res;

};


float rvt_hm_get_height_adv(float fpx, float fpz, int tile_x, int tile_y) {


    int tile_ix = rs_cyclic_mod_i(tile_x, RVT_TILES_SIDE_COUNT);
    int tile_iy = rs_cyclic_mod_i(tile_y, RVT_TILES_SIDE_COUNT);


    int tile_i = tile_iy * RVT_TILES_SIDE_COUNT + tile_ix;

    fpx *= RVT_HEIGHTMAP_TILE_SIDE_SIZE-1;
    fpz *= RVT_HEIGHTMAP_TILE_SIDE_SIZE-1;


    int px = (int) fpx;
    int pz = (int) fpz;


    float kx = fpx - px;
    float kz = fpz - pz;

    int i_z1 = rs_clamp_i(pz, 0, RVT_HEIGHTMAP_TILE_SIDE_SIZE-1);
    int i_z2 = rs_clamp_i(pz+1, 0, RVT_HEIGHTMAP_TILE_SIDE_SIZE-1);

    int i_x1 = rs_clamp_i(px, 0, RVT_HEIGHTMAP_TILE_SIDE_SIZE-1);
    int i_x2 = rs_clamp_i(px+1, 0, RVT_HEIGHTMAP_TILE_SIDE_SIZE-1);

    float h_00 = rvt_app_get_geodata()->hm[tile_i].data[ i_z1*RVT_HEIGHTMAP_TILE_SIDE_SIZE + i_x1 ];
    float h_10 = rvt_app_get_geodata()->hm[tile_i].data[ i_z1*RVT_HEIGHTMAP_TILE_SIDE_SIZE + i_x2 ];

    float h_01 = rvt_app_get_geodata()->hm[tile_i].data[ i_z2*RVT_HEIGHTMAP_TILE_SIDE_SIZE + i_x1 ];
    float h_11 = rvt_app_get_geodata()->hm[tile_i].data[ i_z2*RVT_HEIGHTMAP_TILE_SIDE_SIZE + i_x2 ];

    if (kx+kz < 1.0) {
        return calc_barycentric( rs_vec3(0.0, h_00, 0.0), rs_vec3(1.0, h_10, 0.0), rs_vec3(0.0, h_01, 1.0), kx, kz );
    }
    else {
        return calc_barycentric( rs_vec3(1.0, h_10, 0.0), rs_vec3(0.0, h_01, 1.0), rs_vec3(1.0, h_11, 1.0), kx, kz );
    };



};

float rvt_hm_get_height(float x, float y) {


    int tile_i = rvt_get_tile_i(x, y);

    int hm_tile_ix = tile_i % RVT_TILES_SIDE_COUNT;
    int hm_tile_iy = tile_i / RVT_TILES_SIDE_COUNT;

    float fpx = rs_cyclic_mod_f( (x / rvt_app_get_sc()) , 1.0);
    float fpy = rs_cyclic_mod_f( (y / rvt_app_get_sc()) , 1.0);

    return rvt_hm_get_height_adv(fpx, fpy, hm_tile_ix, hm_tile_iy);

};



void rvt_create_wire_vbodata(int vbodata_index, rs_vec3_t p0, rs_vec3_t p1, rs_vec3_t p2, float halfwidth, int segments_count) {

    rs_vec3_t v0, v1;

    int stride = 12; // pos, dir, additional data

    float approx_line_length = rs_vec3_distance(p0, p1) + rs_vec3_distance(p1, p2);
    float uv_length = approx_line_length / (2.0 * halfwidth);


    float *tri_data = (float*) rs_mem_alloc( (stride) * 6 * segments_count * sizeof(float), RS_MEM_POOL_AUTO );

    for (int i = 0; i < segments_count; i++) {

        int i_next = i + 1;

        v0 = rs_vec3_bezier2(p0, p1, p2, 1.0 * i / segments_count);
        v1 = rs_vec3_bezier2(p0, p1, p2, 1.0 * i_next / segments_count);

        rs_vec3_t v0v1 = rs_vec3_sub(v1, v0);

        rs_vec3_t v_dir0 = rs_vec3_add( v0v1, rs_vec3_sub(v0, rs_vec3_bezier2(p0, p1, p2, 1.0 * (i-1) / segments_count) ) );
        rs_vec3_t v_dir1 = rs_vec3_add( v0v1, rs_vec3_sub(rs_vec3_bezier2(p0, p1, p2, 1.0 * (i+2) / segments_count), v1) );

        if (i == 0) {
            v_dir0 = v0v1;
        }
        else if (i == segments_count - 1) {
            v_dir1 = v0v1;
        };

        v_dir0 = rs_vec3_normalize(v_dir0);
        v_dir1 = rs_vec3_normalize(v_dir1);

        int node_i;

        // First Triangle

        node_i = 6*i + 0;

        tri_data[ (node_i)*(stride) + 0 ] = v0.x;
        tri_data[ (node_i)*(stride) + 1 ] = v0.y;
        tri_data[ (node_i)*(stride) + 2 ] = v0.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir0.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir0.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir0.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = 1.0; // right
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i / segments_count * uv_length; // texcoord

        node_i = 6*i + 1;

        tri_data[ (node_i)*(stride) + 0 ] = v0.x;
        tri_data[ (node_i)*(stride) + 1 ] = v0.y;
        tri_data[ (node_i)*(stride) + 2 ] = v0.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir0.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir0.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir0.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = -1.0; // left
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i / segments_count * uv_length; // texcoord

        node_i = 6*i + 2;

        tri_data[ (node_i)*(stride) + 0 ] = v1.x;
        tri_data[ (node_i)*(stride) + 1 ] = v1.y;
        tri_data[ (node_i)*(stride) + 2 ] = v1.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir1.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir1.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir1.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = -1.0; // left
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i_next / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i_next / segments_count * uv_length; // texcoord

        // Second Triangle

        node_i = 6*i + 3;

        tri_data[ (node_i)*(stride) + 0 ] = v1.x;
        tri_data[ (node_i)*(stride) + 1 ] = v1.y;
        tri_data[ (node_i)*(stride) + 2 ] = v1.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir1.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir1.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir1.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = -1.0; // left
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i_next / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i_next / segments_count * uv_length; // texcoord

        node_i = 6*i + 4;

        tri_data[ (node_i)*(stride) + 0 ] = v1.x;
        tri_data[ (node_i)*(stride) + 1 ] = v1.y;
        tri_data[ (node_i)*(stride) + 2 ] = v1.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir1.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir1.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir1.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = 1.0; // right
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i_next / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i_next / segments_count * uv_length; // texcoord

        node_i = 6*i + 5;

        tri_data[ (node_i)*(stride) + 0 ] = v0.x;
        tri_data[ (node_i)*(stride) + 1 ] = v0.y;
        tri_data[ (node_i)*(stride) + 2 ] = v0.z;
        tri_data[ (node_i)*(stride) + 3 ] = 1.0;

        tri_data[ (node_i)*(stride) + 4 ] = v_dir0.x;
        tri_data[ (node_i)*(stride) + 5 ] = v_dir0.y;
        tri_data[ (node_i)*(stride) + 6 ] = v_dir0.z;
        tri_data[ (node_i)*(stride) + 7 ] = 1.0;

        tri_data[ (node_i)*(stride) + 8 ] = 1.0; // right
        tri_data[ (node_i)*(stride) + 9 ] = halfwidth;
        tri_data[ (node_i)*(stride) + 10 ] = 1.0 * i / segments_count; // pos
        tri_data[ (node_i)*(stride) + 11 ] = 1.0 * i / segments_count * uv_length; // texcoord


    };


    int total_vertices = 6 * segments_count;

    // pos: XYZ1
    // norm: (dirXYZ)1
    // COLOR: (r/l) (halfwidth) (0) (0)


    rvt_vbodata_t *pvbodata = rvt_app_get_vbodata_by_index(vbodata_index);

    int data_len = sizeof(float) * stride * total_vertices;
    pvbodata->data = (float*) rs_mem_alloc( data_len, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( pvbodata->data, "wire vbodata", __LINE__ );

    memcpy(pvbodata->data, tri_data, data_len);

    pvbodata->stride = stride;
    pvbodata->vertices_count = total_vertices;


    rs_mem_free(tri_data);

};

