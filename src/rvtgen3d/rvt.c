#include "rvt.h"

#include "main.h"

#include "rs/rsgeom.h"

#include "tileloader.h"

#include <math.h>

#include "rvtgen.h"

#include <string.h>

#include "rs/rsmem.h"
#include "rvtexport.h"

#include "rvtapp.h"
#include "rvtutil.h"


struct rvt_reg_t rvt_reg;
struct rvt_settings_t rvt_settings;

void rvt_settings_set_to_defaults() {

    memset(&rvt_settings, 0, sizeof(struct rvt_settings_t));

    rvt_settings.separate_to_tiles = 1;

    rvt_settings.layer_enabled[ 0 ] = 1; // heightmap
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_BUILDING ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_AREA ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_ROAD ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_NATURAL ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_PROP ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_WIRE ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_STRIPES ] = 1;
    rvt_settings.layer_enabled[ 1 + RVT_LAYER_WALLS ] = 1;

    rvt_settings.file_format = RVT_FORMAT_PLY_ASCII;

    rvt_settings.z_up = 0;

    rvt_settings.flat_terrain = rvt_app_is_terrain_flat_by_default();


    rvt_settings.building_decorations = 1;

    rvt_settings.smooth_area_edges = 1;
    rvt_settings.smooth_road_edges = 1;
    rvt_settings.smooth_river_edges = 1;

};



void rvt_set_writing_layer(int layer_index) {
    rvt_reg.rvt_writing_layer = layer_index;
};

void rvt_set_writing_igroup(int igroup) {
    rvt_reg.rvt_writing_igroup = igroup;
};


void rvt_color(float r, float g, float b) {
    rvt_reg.geom_color = rs_vec3(r, g, b);
};

void rvt_color_alt(float r, float g, float b) {
    rvt_reg.geom_color_alt = rs_vec3(r, g, b);
};

void rvt_color_i(uint32_t c) {
    rvt_reg.geom_color.r = 1.0/255.0*( (c >> 16) & 0xFF );
    rvt_reg.geom_color.g = 1.0/255.0*( (c >>  8) & 0xFF );
    rvt_reg.geom_color.b = 1.0/255.0*( (c >>  0) & 0xFF );

    // Prevent over-saturated colors

    float saturation = rs_max( rs_max(rvt_reg.geom_color.r, rvt_reg.geom_color.g), rvt_reg.geom_color.b );
    if (saturation >= 0.01) {
        saturation = 1.0 - rs_min( rs_min(rvt_reg.geom_color.r, rvt_reg.geom_color.g), rvt_reg.geom_color.b ) / saturation;
    };

    float avg = 1.0/3.0*(rvt_reg.geom_color.r + rvt_reg.geom_color.g + rvt_reg.geom_color.b);
    rvt_reg.geom_color = rs_vec3_linear( rvt_reg.geom_color, rs_vec3(avg, avg, avg), 0.6*saturation );

};



void rvt_write_vertex(float x, float y, float z, float w,
                      float nx, float ny, float nz, float nw,
                      float tx, float ty, float tz, float tw,
                      float tc_x, float tc_y, float tc_z, float tc_w,
                      float cr, float cg, float cb, float ca) {

    rvt_reg.rvt_counter_vertices_total++;


    int part_i = rvt_reg.rvt_total_vertices[rvt_reg.rvt_writing_layer] / RVT_VERTICES_PER_PART;

    int rvt_stride = rvt_app_get_stride_by_layer(rvt_reg.rvt_writing_layer);

    if (part_i >= RVT_MAX_PARTS) {
        rs_critical_alert_and_halt("Error, part_i has been reached RVT_MAX_PARTS");
    };

    if (rvt_reg.rvt_vert_data[rvt_reg.rvt_writing_layer][part_i] == NULL) {
        rvt_reg.rvt_vert_data[rvt_reg.rvt_writing_layer][part_i] = (float*) rs_mem_alloc_adv( 4 * rvt_stride * RVT_VERTICES_PER_PART, "rvt", __LINE__, -1 );
        rs_app_assert_memory( rvt_reg.rvt_vert_data[rvt_reg.rvt_writing_layer][part_i], "rvt", __LINE__  );
        rvt_reg.rvt_vert_map[rvt_reg.rvt_writing_layer][part_i] = (int*) rs_mem_alloc_adv( 4 * RVT_VERTICES_PER_PART, "rvt", __LINE__, -1 );
        rs_app_assert_memory( rvt_reg.rvt_vert_map[rvt_reg.rvt_writing_layer][part_i], "rvt", __LINE__  );
        rvt_reg.rvt_vert_parts[rvt_reg.rvt_writing_layer]++;

    };

    int vert_i = (rvt_reg.rvt_total_vertices[rvt_reg.rvt_writing_layer]) % RVT_VERTICES_PER_PART;


    rvt_reg.rvt_vert_map[rvt_reg.rvt_writing_layer][part_i][vert_i] = rvt_reg.rvt_subtile_i;

    float *data = &(rvt_reg.rvt_vert_data[rvt_reg.rvt_writing_layer][part_i][ (vert_i) * rvt_stride]);

    rvt_reg.rvt_total_vertices[rvt_reg.rvt_writing_layer]++;

    rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i]++;

    if (rvt_reg.rvt_writing_layer == RVT_LAYER_BUILDING) {

        data[0] = x;
        data[1] = y;
        data[2] = z;

        data[3] = nx;
        data[4] = ny;
        data[5] = nz;

        data[6] = tc_x;
        data[7] = tc_y;

        data[8] = tx;
        data[9] = ty;
        data[10] = tz;
        data[11] = tw;

        data[12] = cr;
        data[13] = cg;
        data[14] = cb;
        data[15] = ca;


    }
    else if (rvt_stride == 20) {

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        data[4] = nx;
        data[5] = ny;
        data[6] = nz;
        data[7] = nw;

        data[8] = tx;
        data[9] = ty;
        data[10] = tz;
        data[11] = tw;

        data[12] = tc_x;
        data[13] = tc_y;
        data[14] = tc_z;
        data[15] = tc_w;

        data[16] = cr;
        data[17] = cg;
        data[18] = cb;
        data[19] = ca;

    }

    else if (rvt_stride == 16) {

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        data[4] = nx;
        data[5] = ny;
        data[6] = nz;
        data[7] = nw;

        data[8] = tc_x;
        data[9] = tc_y;
        data[10] = tc_z;
        data[11] = tc_w;

        data[12] = cr;
        data[13] = cg;
        data[14] = cb;
        data[15] = ca;
    }

    else if (rvt_stride == 8) {

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        data[4] = cr;
        data[5] = cg;
        data[6] = cb;
        data[7] = ca;

    }

    else if (rvt_stride == 12) {

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        data[4] = nx;
        data[5] = ny;
        data[6] = nz;
        data[7] = nw;

        data[8] = cr;
        data[9] = cg;
        data[10] = cb;
        data[11] = ca;

    };


};



void rvt_write_index(int index_value) {


    int part_i = rvt_reg.rvt_total_indices[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup] / RVT_INDICES_PER_PART;

    if (part_i >= RVT_MAX_PARTS) {
        rs_critical_alert_and_halt("Error, part_i has been reached RVT_MAX_PARTS");
    };

    if (rvt_reg.rvt_ind_data[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i] == NULL) {
        rvt_reg.rvt_ind_data[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i] = (uint32_t*) rs_mem_alloc_adv(4 * RVT_INDICES_PER_PART, "rvt", __LINE__, -1 );
        rs_app_assert_memory( rvt_reg.rvt_ind_data[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i], "rvt", __LINE__  );
        rvt_reg.rvt_ind_map[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i] = (int*) rs_mem_alloc_adv( 4 * RVT_INDICES_PER_PART, "rvt", __LINE__, -1 );
        rs_app_assert_memory( rvt_reg.rvt_ind_map[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i], "rvt", __LINE__  );
        rvt_reg.rvt_ind_parts[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup]++;

    };

    int vert_i = (rvt_reg.rvt_total_indices[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup]) % RVT_INDICES_PER_PART;


    rvt_reg.rvt_ind_map[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i][vert_i] = rvt_reg.rvt_subtile_i;

    uint32_t *pindex = &(rvt_reg.rvt_ind_data[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup][part_i][ (vert_i) * 1 ]);

    rvt_reg.rvt_total_indices[rvt_reg.rvt_writing_layer][rvt_reg.rvt_writing_igroup]++;

    *pindex = index_value;


};



void rvt_begin(int tile_i) {

    int tile_ix = tile_i % RVT_TILES_SIDE_COUNT;
    int tile_iy = tile_i / RVT_TILES_SIDE_COUNT;


    memset( &rvt_reg.rvt_vert_map, 0, sizeof(int*) * RVT_MAX_PARTS * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_vert_data, 0, sizeof(float*) * RVT_MAX_PARTS * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_total_vertices, 0, sizeof(int) * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_vert_parts, 0, sizeof(int) * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_vert_current_index, 0, sizeof(int) * RVT_SUBTILES_VBO_LAYERS_COUNT * (RVT_SUBTILES_TOTAL_COUNT+RVT_TILES_TOTAL_COUNT) );

    memset( &rvt_reg.rvt_ind_map, 0, sizeof(int*) * RVT_IGROUPS_COUNT * RVT_MAX_PARTS * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_ind_data, 0, sizeof(uint32_t*) * RVT_IGROUPS_COUNT * RVT_MAX_PARTS * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_total_indices, 0, sizeof(int) * RVT_IGROUPS_COUNT * RVT_SUBTILES_VBO_LAYERS_COUNT );
    memset( &rvt_reg.rvt_ind_parts, 0, sizeof(int) * RVT_IGROUPS_COUNT * RVT_SUBTILES_VBO_LAYERS_COUNT );


    rvt_reg.tile_i = tile_i;
    rvt_reg.tile_ix = tile_ix;
    rvt_reg.tile_iy = tile_iy;

};

#define DEBUGRVTMESHf  DEBUG30f


// Calculate tile position (i or x;y) in pak/subpak by its (x;y or i)

unsigned int conv_xy_to_j(unsigned int x, unsigned int y) {
    unsigned int j = 0;
    for (int i = 0; i < 16; i++) {
        j += ( 1<<(i*2+0) ) * ( (x/(1<<i)) % 2 )
           + ( 1<<(i*2+1) ) * ( (y/(1<<i)) % 2 );
    };
    return j;
};


unsigned int conv_j_to_x(unsigned int j) {
    unsigned int r = 0;
    for (int i = 0; i < 15; i++) {
        r += (1<<i) * ( (j/(1<<(i*2+0))) % 2 );
    };
    return r;
};

unsigned int conv_j_to_y(unsigned int j) {
    unsigned int r = 0;
    for (int i = 0; i < 15; i++) {
        r += (1<<i) * ( (j/(1<<(i*2+1))) % 2 );
    };
    return r;
};




int rvt_get_local_subtile_i(float x, float z) {

    x -= rvt_reg.shift_x;
    z -= rvt_reg.shift_y;

    float sc = rvt_app_get_sc();

    x /= sc;
    z /= sc;

    int ix = rs_clamp_i((int)(x * RVT_SUBTILES_SIDE_COUNT_PER_TILE), 0, RVT_SUBTILES_SIDE_COUNT_PER_TILE-1);
    int iz = rs_clamp_i((int)(z * RVT_SUBTILES_SIDE_COUNT_PER_TILE), 0, RVT_SUBTILES_SIDE_COUNT_PER_TILE-1);

    return iz*RVT_SUBTILES_SIDE_COUNT_PER_TILE + ix;

};

int rvt_get_global_subtile(int local_subtile_i, int tile_ix, int tile_iy) {

    int ix = local_subtile_i % RVT_SUBTILES_SIDE_COUNT_PER_TILE;
    int iz = local_subtile_i / RVT_SUBTILES_SIDE_COUNT_PER_TILE;

    int global_ix = ix + tile_ix*RVT_SUBTILES_SIDE_COUNT_PER_TILE;
    int global_iz = iz + tile_iy*RVT_SUBTILES_SIDE_COUNT_PER_TILE;

    int subtile_i = global_iz*RVT_SUBTILES_SIDE_COUNT + global_ix;

    return subtile_i;

};

void rvt_end(int custom_subtile_i, int tile_ix, int tile_iy) {

    for (int vbo_layer_index = 0; vbo_layer_index < RVT_SUBTILES_VBO_LAYERS_COUNT; vbo_layer_index++) {


        int rvt_stride = rvt_app_get_stride_by_layer(vbo_layer_index);
        int rvt_vbo_conf_index = rvt_app_get_conf_index_by_layer(vbo_layer_index);


        DEBUGRVTMESHf("=== RVT END (layer index %d) ===\n" );


        rvt_reg.export_to_file = 0;
        if (rvt_app_is_exporting()) {
            if (rvt_settings.layer_enabled[vbo_layer_index] ) {
                rvt_reg.export_to_file = 1;
            };
        };

        // ============ 1. Vertices ========

        int subtile_total_vertices[RVT_SUBTILES_COUNT_PER_TILE+1];
        memset(subtile_total_vertices, 0, sizeof(int)*(RVT_SUBTILES_COUNT_PER_TILE+1) );

        for (int vi = 0; vi < rvt_reg.rvt_total_vertices[vbo_layer_index]; vi++) {
            int part_i = vi / RVT_VERTICES_PER_PART;
            int vert_i = vi % RVT_VERTICES_PER_PART;
            int ti = rvt_reg.rvt_vert_map[vbo_layer_index][part_i][vert_i];
            subtile_total_vertices[ ti ]++;
        };



        float *data[RVT_SUBTILES_COUNT_PER_TILE+1];
        int data_pointer[RVT_SUBTILES_COUNT_PER_TILE+1];
        memset(data_pointer, 0, sizeof(int) * (RVT_SUBTILES_COUNT_PER_TILE+1) );

        for (int ti = 0; ti < RVT_SUBTILES_COUNT_PER_TILE+1; ti++) {
            if (subtile_total_vertices[ti]) {
                data[ti] = (float*) rs_mem_alloc_adv( 4 * rvt_stride * subtile_total_vertices[ti], "rvt", __LINE__, RS_MEM_POOL_AUTO );
                rs_app_assert_memory_adv( data[ti], "rvt", __LINE__, 4 * rvt_stride * subtile_total_vertices[ti] );
            }
            else {
                data[ti] = NULL;
            };
        };


        for (int vi = 0; vi < rvt_reg.rvt_total_vertices[vbo_layer_index]; vi++) {
            int part_i = vi / RVT_VERTICES_PER_PART;
            int vert_i = vi % RVT_VERTICES_PER_PART;
            int ti = rvt_reg.rvt_vert_map[vbo_layer_index][part_i][vert_i];
            memcpy( &data[ti] [ rvt_stride * data_pointer[ti] ], &rvt_reg.rvt_vert_data[vbo_layer_index][part_i][ rvt_stride * vert_i ], 4 * rvt_stride );
            data_pointer[ti]++;
        };


        int rvt_export_v_type = RVT_EXPORT_V_TYPE_16;
        if (vbo_layer_index == RVT_LAYER_BUILDING) {
            rvt_export_v_type = RVT_EXPORT_V_TYPE_BUILDING;
        };
        if ( (vbo_layer_index==RVT_LAYER_AREA) || (vbo_layer_index==RVT_LAYER_ROAD) || (vbo_layer_index==RVT_LAYER_WIRE) ) {
            rvt_export_v_type = RVT_EXPORT_V_TYPE_12;
        };


        for (int p = 0; p < rvt_reg.rvt_vert_parts[vbo_layer_index]; p++) {

            rs_mem_free(rvt_reg.rvt_vert_data[vbo_layer_index][p]);
            rs_mem_free(rvt_reg.rvt_vert_map[vbo_layer_index][p]);

        };

        // ============ 2. Indices ========


        int subtile_total_indices[RVT_SUBTILES_COUNT_PER_TILE+1];
        memset(subtile_total_indices, 0, sizeof(int)*(RVT_SUBTILES_COUNT_PER_TILE+1) );

        for (int gi = 0; gi < RVT_IGROUPS_COUNT; gi++) {
            for (int ii = 0; ii < rvt_reg.rvt_total_indices[vbo_layer_index][gi]; ii++) {
                int part_i = ii / RVT_INDICES_PER_PART;
                int vert_i = ii % RVT_INDICES_PER_PART;
                int ti = rvt_reg.rvt_ind_map[vbo_layer_index][gi][part_i][vert_i];
                subtile_total_indices[ ti ]++;
            };
        };


        uint32_t* ind_data[RVT_SUBTILES_COUNT_PER_TILE+1];
        int ind_data_pointer[RVT_SUBTILES_COUNT_PER_TILE+1];
        memset(ind_data_pointer, 0, sizeof(int) * (RVT_SUBTILES_COUNT_PER_TILE+1) );

        for (int ti = 0; ti < RVT_SUBTILES_COUNT_PER_TILE+1; ti++) {
            if (subtile_total_indices[ti]) {
                ind_data[ti] = (uint32_t*) rs_mem_alloc_adv( 4 * 1 * subtile_total_indices[ti], "rvt", __LINE__, RS_MEM_POOL_AUTO );
                rs_app_assert_memory_adv( ind_data[ti], "rvt", __LINE__ , 4 * 1 * subtile_total_indices[ti] );
                }
            else {
                ind_data[ti] = NULL;
            };
        };


        uint32_t igroup_start[RVT_SUBTILES_COUNT_PER_TILE+1][RVT_IGROUPS_COUNT];
        uint32_t igroup_count[RVT_SUBTILES_COUNT_PER_TILE+1][RVT_IGROUPS_COUNT];
        uint32_t igroup_total_counter[RVT_SUBTILES_COUNT_PER_TILE+1];

        memset( igroup_start, 0, 4 * (RVT_SUBTILES_COUNT_PER_TILE+1) * RVT_IGROUPS_COUNT );
        memset( igroup_count, 0, 4 * (RVT_SUBTILES_COUNT_PER_TILE+1) * RVT_IGROUPS_COUNT );
        memset( igroup_total_counter, 0, 4 * (RVT_SUBTILES_COUNT_PER_TILE+1)  );


        for (int gi = 0; gi < RVT_IGROUPS_COUNT; gi++) {

            for (int i = 0; i < RVT_SUBTILES_COUNT_PER_TILE+1; i++) {
                igroup_start[i][gi] = igroup_total_counter[i];
            };

            for (int ii = 0; ii < rvt_reg.rvt_total_indices[vbo_layer_index][gi]; ii++) {
                int part_i = ii / RVT_INDICES_PER_PART;
                int vert_i = ii % RVT_INDICES_PER_PART;
                int ti = rvt_reg.rvt_ind_map[vbo_layer_index][gi][part_i][vert_i];

                memcpy( &ind_data[ti] [ 1 * ind_data_pointer[ti] ], &rvt_reg.rvt_ind_data[vbo_layer_index][gi][part_i][ 1 * vert_i ], 4 * 1 );
                ind_data_pointer[ti]++;

                igroup_count[ti][gi]++;
            };

            for (int i = 0; i < RVT_SUBTILES_COUNT_PER_TILE+1; i++) {
                igroup_total_counter[i] += igroup_count[i][gi];
            };
        };

        for (int gi = 0; gi < RVT_IGROUPS_COUNT; gi++) {
            for (int p = 0; p < rvt_reg.rvt_ind_parts[vbo_layer_index][gi]; p++) {

                if ( rvt_reg.rvt_ind_data[vbo_layer_index][gi][p] ) {
                    rs_mem_free( rvt_reg.rvt_ind_data[vbo_layer_index][gi][p] );
                };

                if ( rvt_reg.rvt_ind_map[vbo_layer_index][gi][p] ) {
                    rs_mem_free( rvt_reg.rvt_ind_map[vbo_layer_index][gi][p] );
                };

            };
        };


        if (rvt_reg.export_to_file) {

            int z_up = rvt_settings.z_up;

            int layer_i = vbo_layer_index + 1; // heightmap is 0, geodata starts from 1

            if (rvt_settings.separate_to_tiles) {
                rvt_export_file_init( &rvt_settings.rvt_file_arr[layer_i], rvt_settings.file_format, z_up, rvt_export_v_type,
                    rvt_tile_loader_reg.tile_z, rvt_tile_loader_reg.tile_world_ix, rvt_tile_loader_reg.tile_world_iy, layer_i );
            }
            else {
                if (rvt_settings.rvt_file_arr[layer_i].fp == NULL) {
                    rvt_export_file_init(&rvt_settings.rvt_file_arr[layer_i], rvt_settings.file_format, z_up, rvt_export_v_type, 0, 0, 0, layer_i);
                };
            };

            rvt_export_file_start_new_tile( &rvt_settings.rvt_file_arr[layer_i], rvt_tile_loader_reg.tile_z,
                                                rvt_tile_loader_reg.tile_world_ix, rvt_tile_loader_reg.tile_world_iy, layer_i );


            for (int ti = 0; ti < RVT_SUBTILES_COUNT_PER_TILE+1; ti++) {
                if (ind_data[ti] != NULL) {

                    rvt_export_file_write(&rvt_settings.rvt_file_arr[layer_i],
                            data[ti], subtile_total_vertices[ti],
                            ind_data[ti], subtile_total_indices[ti] );

                };
            };

            rvt_app_exporting_total_bytes_append(rvt_settings.rvt_file_arr[layer_i].total_bytes);

            rvt_settings.rvt_file_arr[layer_i].total_bytes = 0;

            if (rvt_settings.separate_to_tiles) {
                rvt_export_file_term( &rvt_settings.rvt_file_arr[layer_i] );
            };

        };



        // ============ 3. Creating VBO ========

        for (int i = 0; i < RVT_SUBTILES_COUNT_PER_TILE + 1; i++) {

            if (subtile_total_vertices[i] == 0) {
                continue;
            };


            DEBUGRVTMESHf("Processing subtile %d...\n", i);


            int subtile_i = rvt_get_global_subtile(i, tile_ix, tile_iy);
            if (i == RVT_SUBTILES_COUNT_PER_TILE) {
                // Last = static
                subtile_i = RVT_SUBTILES_TOTAL_COUNT + (tile_iy*RVT_TILES_SIDE_COUNT + tile_ix);
            };

            int layer_i = vbo_layer_index;

                int st_recs_limit = RVT_TILE_LOADER_MAX_ST_RECS_COUNT;
                int loop = 1;
                while (loop) {
                    rvt_app_sleep_msec(1);
                    RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);
                    loop = (rvt_tile_loader_reg.st_ready_count >= st_recs_limit);
                    RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);
                };


                RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);

                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_data = data[i];
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_total_vertices = subtile_total_vertices[i];
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_stride = rvt_stride;
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_ind_data = ind_data[i];
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_total_indices = subtile_total_indices[i];
                #ifdef STREETS_GAME
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_ind_type = RS_VBO_UNSIGNED_INT;
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_geom_type = RS_VBO_TRIANGLES;
                #endif
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_vbo_conf_index = rvt_vbo_conf_index;


                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_subtile_i = subtile_i;
                rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_layer_i = layer_i;

                for (int gi = 0; gi < RVT_IGROUPS_COUNT; gi++) {
                    rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_igroup_start[gi] = igroup_start[i][gi];
                    rvt_tile_loader_reg.st_rec[rvt_tile_loader_reg.st_ready_count].st_igroup_indices_count[gi] = igroup_count[i][gi];
                };

                rvt_tile_loader_reg.st_ready_count++;

                RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);

        };

        // wait until all pending VBOs of current layer is created
        int loop = 1;
        while (loop) {
            rvt_app_sleep_msec(1);
            RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);
            loop = (rvt_tile_loader_reg.st_ready_count != 0);
            RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);
        };


        for (int ti = 0; ti < RVT_SUBTILES_COUNT_PER_TILE+1; ti++) {
            if (data[ti] != NULL) {
                rs_mem_free(data[ti]);
            };
            if (subtile_total_indices[ti]) {
                rs_mem_free(ind_data[ti]);
            }
        };

    };



};


typedef struct render_point_t {
    float x;
    float y;
    float angle;
    float dist_to_next;
    int flags;
} render_point_t;




void rvt_write_stripe_linestring( int geom_flags, float y_start, float stripe_halfwidth, float stripe_center_shift, float tc_v1, float tc_v2, int disable_k_end, float ca_left, float ca_right, float y_left, rs_linestring_t *ring) {

    // exclude last point for linestrings
    int last_point_n = (geom_flags & RVT_GEOM_FLAG_OPEN) ? 1 : 0;


    int points_count = ring->points_count;


    render_point_t *pts = (render_point_t*) rs_mem_alloc_adv( sizeof(render_point_t) * points_count, "rvt", __LINE__, -1 );
    rs_app_assert_memory_adv( pts, "rvt", __LINE__, sizeof(render_point_t) * points_count );


    for (int i = 0; i < ring->points_count; i++) {
        pts[i].x = ring->p[i].x;
        pts[i].y = ring->p[i].y;
    }


    for (int j = 0; j < points_count - last_point_n; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;



        pts[j].flags = 0;
        if (geom_flags & RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT) {

            rs_vec3_t v1 = rs_vec3( pts[j].x, 0.0, pts[j].y );
            rs_vec3_t v2 = rs_vec3( pts[jn].x, 0.0, pts[jn].y );

            const float th1 = 0.999 * rvt_app_get_sc();
            const float th0 = 0.001 * rvt_app_get_sc();
            if ( (v1.x > th1) && (v2.x > th1) ) {
                pts[j].flags = 1;
            };
            if ( (v1.x < th0) && (v2.x < th0) ) {
                pts[j].flags = 1;
            };
            if ( (v1.z > th1) && (v2.z > th1) ) {
                pts[j].flags = 1;
            };
            if ( (v1.z < th0) && (v2.z < th0) ) {
                pts[j].flags = 1;
            };

        };

        pts[j].dist_to_next = sqrtf( RS_SQR(pts[jn].x - pts[j].x) + RS_SQR(pts[jn].y - pts[j].y) );


        float x1 = pts[jn].x - pts[j].x;
        float y1 = pts[jn].y - pts[j].y;
        float x2 = -(pts[jp].x - pts[j].x);
        float y2 = -(pts[jp].y - pts[j].y);

        float an = atan2( y1, x1 );
        float ap = atan2( y2, x2 );

        pts[j].angle = an - ap;
        if (pts[j].angle < 0.0) {
            pts[j].angle += 2.0*M_PI;
        };

    };

    for (int j = 0; j < points_count - last_point_n; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;

        if (pts[j].flags & 1) {
            pts[j].angle = 0;
            pts[jn].angle = 0;
        };

    };


    float cr = rvt_reg.geom_color.x;
    float cg = rvt_reg.geom_color.y;
    float cb = rvt_reg.geom_color.z;


    float uvk = (1.0/stripe_halfwidth) * 0.5*(tc_v2 - tc_v1);


    for (int j = 0; j < points_count - last_point_n; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;


        rs_vec3_t v1 = rs_vec3( pts[j].x, 0.0, pts[j].y );
        rs_vec3_t v2 = rs_vec3( pts[jn].x, 0.0, pts[jn].y );

        if (geom_flags & RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT) {
            if (pts[j].flags) {
                continue;
            };
        };

        if (geom_flags & RVT_GEOM_FLAG_ADD_HM_TO_POINTS) {
            v1.y = rvt_hm_get_height_adv(v1.x/rvt_app_get_sc(), v1.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
            v2.y = rvt_hm_get_height_adv(v2.x/rvt_app_get_sc(), v2.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
        };


        v1.x += rvt_reg.shift_x;
        v1.z += rvt_reg.shift_y;
        v2.x += rvt_reg.shift_x;
        v2.z += rvt_reg.shift_y;

        rs_vec3_t v_norm = rs_vec3(0.0, -1.0, 0.0);

        float distance = pts[j].dist_to_next;
        float uv_distance = distance;


        float c_angle = atan2( v2.z - v1.z, v2.x - v1.x );
        float s_angle = c_angle + M_PI_2;

        float halfangle_v1 = 0.5* pts[j].angle;
        float halfangle_v2 = -0.5* pts[jn].angle;

        float k_end_v1 = 1.0;
        float k_end_v2 = 1.0;
        float k_end_single = 0.0;

        if ( (last_point_n) ) {
            if (j == 0) {
                halfangle_v1 = 0.0;
                k_end_v1 = 0.0;
            };
            if (j == (points_count - last_point_n - 1) ) {
                halfangle_v2 = 0.0;
                k_end_v2 = 0.0;
            };
        };

        if ( points_count - last_point_n == 1 ) {
            k_end_single = 1.0;
            k_end_v1 = 0.0;
            k_end_v2 = 1.0;
        };

        if (disable_k_end) {
            k_end_single = 0.0;
            k_end_v1 = 1.0;
            k_end_v2 = 1.0;
        };


        float c_tan_limits = 4.0;  // mitre

        float c_tan1 = tan(halfangle_v1);

        c_tan1 = rs_clamp(c_tan1, -c_tan_limits, c_tan_limits);

        float c_tan2 = tan(halfangle_v2);
        c_tan2 = rs_clamp(c_tan2, -c_tan_limits, c_tan_limits);

        rvt_reg.rvt_subtile_i = rvt_get_local_subtile_i( (v1.x+v2.x)/2, (v1.z+v2.z)/2 );
        if ( RS_SQR(v1.x-v2.x) + RS_SQR(v1.z-v2.z) > RS_SQR(1.0*rvt_app_get_sc()/RVT_SUBTILES_SIDE_COUNT_PER_TILE) ) {
            rvt_reg.rvt_subtile_i = RVT_SUBTILES_COUNT_PER_TILE; // static
        };


        int v_index = rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i];

        // Quad: 4 vertices, 6 indices
        rvt_write_index(v_index + 0);
        rvt_write_index(v_index + 1);
        rvt_write_index(v_index + 2);
        rvt_write_index(v_index + 2);
        rvt_write_index(v_index + 1);
        rvt_write_index(v_index + 3);

        rvt_write_vertex(v1.x - cos(s_angle)*(stripe_halfwidth+stripe_center_shift) - cos(c_angle)*c_tan1*(stripe_halfwidth+stripe_center_shift), v1.y + y_start + y_left,  v1.z - sin(s_angle)*(stripe_halfwidth+stripe_center_shift) - sin(c_angle)*c_tan1*(stripe_halfwidth+stripe_center_shift), 1.0,        v_norm.x, v_norm.y, v_norm.z, (float)(0.0),     0.0, 1.0, 0.0, 0.0,      (-c_tan1*stripe_halfwidth)*uvk ,              tc_v1, k_end_v1, k_end_single, cr, cg, cb, ca_left );
        rvt_write_vertex(v2.x - cos(s_angle)*(stripe_halfwidth+stripe_center_shift) - cos(c_angle)*c_tan2*(stripe_halfwidth+stripe_center_shift), v2.y + y_start + y_left,  v2.z - sin(s_angle)*(stripe_halfwidth+stripe_center_shift) - sin(c_angle)*c_tan2*(stripe_halfwidth+stripe_center_shift), 1.0,        v_norm.x, v_norm.y, v_norm.z, (float)(0.0),     0.0, 1.0, 0.0, 0.0,      (-c_tan2*stripe_halfwidth + uv_distance)*uvk, tc_v1, k_end_v2, k_end_single, cr, cg, cb, ca_left );
        rvt_write_vertex(v1.x + cos(s_angle)*(stripe_halfwidth-stripe_center_shift) + cos(c_angle)*c_tan1*(stripe_halfwidth-stripe_center_shift), v1.y + y_start,           v1.z + sin(s_angle)*(stripe_halfwidth-stripe_center_shift) + sin(c_angle)*c_tan1*(stripe_halfwidth-stripe_center_shift), 1.0,        v_norm.x, v_norm.y, v_norm.z, (float)(0.0),     0.0, 1.0, 0.0, 0.0,      (c_tan1*stripe_halfwidth)*uvk,                tc_v2, k_end_v1, k_end_single, cr, cg, cb, ca_right );
        rvt_write_vertex(v2.x + cos(s_angle)*(stripe_halfwidth-stripe_center_shift) + cos(c_angle)*c_tan2*(stripe_halfwidth-stripe_center_shift), v2.y + y_start,           v2.z + sin(s_angle)*(stripe_halfwidth-stripe_center_shift) + sin(c_angle)*c_tan2*(stripe_halfwidth-stripe_center_shift), 1.0,        v_norm.x, v_norm.y, v_norm.z, (float)(0.0),     0.0, 1.0, 0.0, 0.0,      (c_tan2*stripe_halfwidth + uv_distance)*uvk,  tc_v2, k_end_v2, k_end_single, cr, cg, cb, ca_right );

    };

    rs_mem_free(pts);

};





void rvt_write_wall_linestring( int geom_flags, float y_start, float y_height, float v_base, float v_top, float uv_scale, float ca_component, rs_linestring_t *ring) {


    if (geom_flags & RVT_GEOM_FLAG_FLAT_WALL) {
        v_top = 0.0;
        v_base = (y_height) * 4.0 / 1.667 / 7.5;
    };


    // exclude last point for linestrings
    int last_point_n = (geom_flags & RVT_GEOM_FLAG_OPEN) ? 1 : 0;

    int points_count = ring->points_count;

    render_point_t *pts = (render_point_t*) rs_mem_alloc_adv( sizeof(render_point_t) * points_count, "rvt", __LINE__, -1 );
    rs_app_assert_memory( pts, "rvt", __LINE__ );

    for (int i = 0; i < ring->points_count; i++) {
        pts[i].x = ring->p[i].x;
        pts[i].y = ring->p[i].y;
    }

    float avg_side_len = 0.0;

    int sides_count = 0;

    for (int j = 0; j < points_count - last_point_n; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;

        pts[j].dist_to_next = sqrtf( RS_SQR(pts[jn].x - pts[j].x) + RS_SQR(pts[jn].y - pts[j].y) );
        avg_side_len += pts[j].dist_to_next;

        float x1 = pts[jn].x - pts[j].x;
        float y1 = pts[jn].y - pts[j].y;
        float x2 = -(pts[jp].x - pts[j].x);
        float y2 = -(pts[jp].y - pts[j].y);

        float an = atan2( y1, x1 );
        float ap = atan2( y2, x2 );

        pts[j].angle = an - ap;
        if (pts[j].angle < -M_PI) {
            pts[j].angle += 2.0*M_PI;
        };
        if (pts[j].angle > M_PI) {
            pts[j].angle -= 2.0*M_PI;
        };

        if ( fabs(an) > 0.1 ) {
            sides_count++;
        };

    };


    avg_side_len /= sides_count;

    float wall_height = y_height;

    float cr = 0.5;
    float cg = 0.5;
    float cb = 0.5;


    cr = rvt_reg.geom_color.x;
    cg = rvt_reg.geom_color.y;
    cb = rvt_reg.geom_color.z;


    float uvk = 1.0 / uv_scale;

    for (int j = 0; j < points_count - last_point_n; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;

        rs_vec3_t v1 = rs_vec3( pts[j].x, 0.0, pts[j].y );
        rs_vec3_t v2 = rs_vec3( pts[jn].x, 0.0, pts[jn].y );

        if (geom_flags & RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT) {
            const float th1 = 0.999 * rvt_app_get_sc();
            const float th0 = 0.001 * rvt_app_get_sc();
            if ( (v1.x > th1) && (v2.x > th1) ) {
                continue;
            };
            if ( (v1.x < th0) && (v2.x < th0) ) {
                continue;
            };
            if ( (v1.z > th1) && (v2.z > th1) ) {
                continue;
            };
            if ( (v1.z < th0) && (v2.z < th0) ) {
                continue;
            };
        };

        if (geom_flags & RVT_GEOM_FLAG_ADD_HM_TO_POINTS) {
            v1.y = rvt_hm_get_height_adv(v1.x/rvt_app_get_sc(), v1.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
            v2.y = rvt_hm_get_height_adv(v2.x/rvt_app_get_sc(), v2.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
        };

        v1.x += rvt_reg.shift_x;
        v1.z += rvt_reg.shift_y;
        v2.x += rvt_reg.shift_x;
        v2.z += rvt_reg.shift_y;


        rs_vec3_t v_norm = rs_vec3_normalize( rs_vec3_cross( rs_vec3(0.0, -1.0, 0.0), rs_vec3_sub(v2, v1) ) );

        float distance = pts[j].dist_to_next;
        float uv_distance = floor(0.5+1.667*distance) / 7.5;


        int flat_wall = 0;
        float angle_threshold = 65.0*M_PI/180.0;
        float angle_upper_threshold = 153.0 * M_PI / 180.0;

        if ( (distance < 0.75*avg_side_len)
                && ((pts[j].angle)  < -angle_threshold)
                && ((pts[jn].angle) < -angle_threshold)
            ) {
            flat_wall = 1;
        };

        if (geom_flags & RVT_GEOM_FLAG_FLAT_WALL) {
            flat_wall = 1;
        };

        if (flat_wall) {
            uv_distance = 1.667*distance / 7.5;
        };


        float y_current = y_start;
        float y_a[3];
        y_a[0] = wall_height;

        int tangent_w[3] = { 0, 0, 0 };

        for (int k = 0; k < 1; k++) {


            if (flat_wall) {
                tangent_w[k] |= 4;
            }

            rvt_reg.rvt_subtile_i = rvt_get_local_subtile_i( (v1.x+v2.x)/2, (v1.z+v2.z)/2 );
            if ( rs_vec3_distance_sqr(v1, v2) > 12.0*rvt_app_get_sc()/RVT_SUBTILES_SIDE_COUNT_PER_TILE ) {
                rvt_reg.rvt_subtile_i = RVT_SUBTILES_COUNT_PER_TILE;
            };

            int v_index = rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i];

            if ( rvt_reg.rvt_writing_igroup >= RVT_IGROUP_ANGLE0 ) {

                float iangle = atan2( v2.z - v1.z, v2.x - v1.x ) - M_PI_2;

                iangle = rs_clamp_angle( iangle + 2.0*M_PI );

                int iangle_i = (int)( 1.0 / (2.0*M_PI) * iangle * RVT_IGROUP_ANGLES_COUNT );

                iangle_i %= RVT_IGROUP_ANGLES_COUNT;

                rvt_set_writing_igroup( RVT_IGROUP_ANGLE0 + iangle_i );
            };

            // Quad: 4 vertices, 6 indices

            rvt_write_index(v_index + 0);
            rvt_write_index(v_index + 1);
            rvt_write_index(v_index + 2);
            rvt_write_index(v_index + 2);
            rvt_write_index(v_index + 1);
            rvt_write_index(v_index + 3);

            rvt_write_vertex(v1.x, v1.y + y_current, v1.z, 1.0,                 v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      0.0,             v_base, 0.0, 0.0,    cr, cg, cb, ca_component );
            rvt_write_vertex(v2.x, v2.y + y_current, v2.z, 1.0,                 v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      uv_distance*uvk, v_base, 0.0, 0.0,    cr, cg, cb, ca_component );
            rvt_write_vertex(v1.x, v1.y + y_current + y_a[k], v1.z, 1.0,        v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      0.0,             v_top,  0.0, 0.0,    cr, cg, cb, ca_component );

            rvt_write_vertex(v2.x, v2.y + y_current + y_a[k], v2.z, 1.0,        v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      uv_distance*uvk, v_top,  0.0, 0.0,    cr, cg, cb, ca_component );

            y_current += y_a[k];

        };


    };

    rs_mem_free(pts);


};

void rvt_write_wire( rs_vec3_t p1, rs_vec3_t p2, float wire_thickness, float wire_height, int flags ) {

    if (flags & RVT_GEOM_FLAG_ADD_HM) {
        p1.y += rvt_hm_get_height_adv(p1.x/rvt_app_get_sc(), p1.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
        p2.y += rvt_hm_get_height_adv(p2.x/rvt_app_get_sc(), p2.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
    };


    float ap_distance = rs_vec3_distance( p2, p1 );
    float scale = ap_distance;

    float azimuth = atan2( p2.z - p1.z, p2.x - p1.x );

    rs_vec3_t p_delta = rs_vec3_sub(p2, p1);
    rs_vec3_t p_middle = rs_vec3(p_delta.x / 2.0, p_delta.y / 2.0 - 1.0*wire_height, p_delta.z / 2.0);


    rvt_create_wire_vbodata(VBODATA_TEMP_WIRE, rs_vec3(0.0, 0.0, 0.0), p_middle, p_delta, 0.03, 12 );

    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS, p1, rs_vec3(1.0, 1.0, 1.0), 0.0, 0.0, 0.0, VBODATA_TEMP_WIRE );

    rs_mem_free( rvt_app_get_vbodata_by_index(VBODATA_TEMP_WIRE)->data );
    rvt_app_get_vbodata_by_index(VBODATA_TEMP_WIRE)->data = NULL;


};


void rvt_write_stripe( int geom_flags, float y_start, float stripe_halfwidth, float stripe_center_shift, float tc_v1, float tc_v2, int disable_k_end, float ca_left, float ca_right, float y_left, rs_shape_t *p ) {


    if (p->rings_count < 1) {
        DEBUG10f("No rings. Empty polygon. Skipping\n" );
        return;
    };

    rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, 22.0);

    for (int r = 0; r < p_segmentized->rings_count; r++) {
        rvt_write_stripe_linestring( geom_flags, y_start, stripe_halfwidth, stripe_center_shift, tc_v1, tc_v2, disable_k_end, ca_left, ca_right, y_left, p_segmentized->rings[r] );

    };

    rs_shape_destroy(p_segmentized);


};


void rvt_write_wall( int geom_flags, float y_start, float y_height, float v_base, float v_top, float uv_scale, float ca_component, rs_shape_t *p ) {


    if (p->rings_count < 1) {
        // TODO: check if empty polygon can cause problems
        DEBUG10f("No rings. Empty polygon. Skipping\n" );
        return;
    };

    for (int r = 0; r < p->rings_count; r++) {

        rvt_write_wall_linestring( geom_flags, y_start, y_height, v_base, v_top, uv_scale, ca_component, p->rings[r] );

    };


};

void rvt_write_roof( float roof_start, float roof_height, rs_shape_t *p ) {

    if (p->rings_count < 1) {
        DEBUG10f("No rings. Empty polygon. Skipping\n" );
        return;
    };


    // only for 0th ring

    rs_linestring_t *ring = p->rings[0];

    float v_top = 0.0;
    float v_base = (roof_height) * 4.0 / 1.667 / 7.5;


    rs_point_t middle_point = rs_linestring_get_middle_point(ring);
    middle_point.x += rvt_reg.shift_x;
    middle_point.y += rvt_reg.shift_y;


    int points_count = ring->points_count;

    render_point_t *pts = (render_point_t*) rs_mem_alloc_adv( sizeof(render_point_t) * points_count, "rvt", __LINE__, -1 );
    rs_app_assert_memory( pts, "rvt", __LINE__ );


    for (int i = 0; i < ring->points_count; i++) {
        pts[i].x = ring->p[i].x;
        pts[i].y = ring->p[i].y;
    };



    for (int j = 0; j < points_count; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;

        pts[j].dist_to_next = sqrtf( RS_SQR(pts[jn].x - pts[j].x) + RS_SQR(pts[jn].y - pts[j].y) );

    };


    float wall_height = roof_height;

    float cr = 0.5;
    float cg = 0.5;
    float cb = 0.5;

    cr = rvt_reg.geom_color.x;
    cg = rvt_reg.geom_color.y;
    cb = rvt_reg.geom_color.z;

    float ca_component = 1.0;


    float uvk = 1.0;



    for (int j = 0; j < points_count; j++) {

        int jn = (j+1) % points_count;
        int jp = (j+points_count-1) % points_count;

        rs_vec3_t v1 = rs_vec3( pts[j].x, 0.0, pts[j].y );
        rs_vec3_t v2 = rs_vec3( pts[jn].x, 0.0, pts[jn].y );


        v1.x += rvt_reg.shift_x;
        v1.z += rvt_reg.shift_y;
        v2.x += rvt_reg.shift_x;
        v2.z += rvt_reg.shift_y;


        rs_vec3_t v_norm = rs_vec3_normalize( rs_vec3_cross( rs_vec3(0.0, -1.0, 0.0), rs_vec3_sub(v2, v1) ) );

        float distance = pts[j].dist_to_next;
        float uv_distance = floor(0.5+1.667*distance) / 7.5;


        int flat_wall = 0;


        flat_wall = 1;

        if (flat_wall) {
            uv_distance = 1.667*distance / 7.5;
        };

        float u_top = 0.5*uv_distance;
        v_base = 0.0;
        v_top = 1.667*rs_vec3_distance( rs_vec3_linear(v1, v2, 0.5), rs_vec3(middle_point.x, wall_height, middle_point.y) ) / 7.5;




        float y_current = roof_start;
        float y_a[3];
        y_a[0] = wall_height;


        int tangent_w[3] = { 0, 0, 0 };

        for (int k = 0; k < 1; k++) {

            if (flat_wall) {
                tangent_w[k] |= 4;
            }

            rvt_reg.rvt_subtile_i = rvt_get_local_subtile_i( (v1.x+v2.x)/2, (v1.z+v2.z)/2 );

            int v_index = rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i];


            rvt_write_index(v_index + 0);
            rvt_write_index(v_index + 1);
            rvt_write_index(v_index + 2);

            rvt_write_vertex(v1.x, v1.y + y_current, v1.z, 1.0,                                     v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      0.0,             v_base, 0.0, 0.0,    cr, cg, cb, ca_component );
            rvt_write_vertex(v2.x, v2.y + y_current, v2.z, 1.0,                                     v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      uv_distance*uvk, v_base, 0.0, 0.0,    cr, cg, cb, ca_component );
            rvt_write_vertex(middle_point.x, 0.0 + y_current + y_a[k], middle_point.y, 1.0,         v_norm.x, 0.0, v_norm.z, 0.0,     0.0, 1.0, 0.0, (float)(tangent_w[k]),      u_top,             v_top,  0.0, 0.0,    cr, cg, cb, ca_component );

            y_current += y_a[k];

        };


    };

    rs_mem_free(pts);

};

void rvt_write_vbodata( int geom_flags, rs_vec3_t pos, rs_vec3_t scale, float angle, float uv2, float uv3, int index ) {

    if (geom_flags & RVT_GEOM_FLAG_ADD_HM) {
        pos.y += rvt_hm_get_height_adv(pos.x/rvt_app_get_sc(), pos.z/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
    };

    rvt_vbodata_t *pvbodata = rvt_app_get_vbodata_by_index(index);

    rvt_reg.rvt_subtile_i = rvt_get_local_subtile_i( rvt_reg.shift_x + pos.x, rvt_reg.shift_y + pos.z );

    rs_mx_t mx;
    rs_mx_identity(mx);
    rs_mx_translate(mx, pos.x + rvt_reg.shift_x, pos.y, pos.z + rvt_reg.shift_y);
    rs_mx_rotate(mx, angle, 0.0, -1.0, 0.0);
    rs_mx_scale_adv(mx, scale.x, scale.y, scale.z);

    rs_mx_t mx_norm;
    rs_mx_identity(mx_norm);
    rs_mx_rotate(mx_norm, angle, 0.0, -1.0, 0.0);

    int color_i = 12;
    if (pvbodata->stride == 12) {
        color_i = 8;
    };

    for (unsigned int i = 0; i < pvbodata->vertices_count; i++) {

        float *v = &pvbodata->data[ i*pvbodata->stride ];


        rs_vec4_t vc = rs_vec4(v[0], v[1], v[2], 1.0);
        vc = rs_mx_mult_vec(mx, vc);

        rs_vec4_t vc_norm = rs_vec4(v[4], v[5], v[6], 1.0);
        vc_norm = rs_mx_mult_vec(mx_norm, vc_norm);

        rs_vec4_t v_color = rs_vec4(rvt_reg.geom_color.x, rvt_reg.geom_color.y, rvt_reg.geom_color.z, v[color_i+3]);
        if (geom_flags & RVT_GEOM_FLAG_USE_VBODATA_COLORS) {
            v_color = *(rs_vec4_t*)(&v[color_i]);
        };


        float tangent_w = 1.0; // 1.0 for building roof
        float uv_scale = 1.0;
        if ( (geom_flags & RVT_GEOM_FLAG_BUILDING_COLOR_CODE) && ( v[color_i+0] < 0.5 ) ) {
            tangent_w = 0.0;  // wall with windows
            uv_scale = 0.25;
            v_color = rs_vec4(rvt_reg.geom_color_alt.x, rvt_reg.geom_color_alt.y, rvt_reg.geom_color_alt.z, v[color_i+3]);
        };


        int v_index = rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i];
        rvt_write_index(v_index);



        rvt_write_vertex( vc.x, vc.y, vc.z, 1.0,
                          vc_norm.x, vc_norm.y, vc_norm.z, 0.0,
                          1.0, 0.0, 0.0, tangent_w,
                          uv_scale*v[8], uv_scale*v[9], uv2, uv3,
                          v_color.x, v_color.y, v_color.z, v_color.w );


    };

};


void rvt_write_triangles( int add_hm_height, float y_start, float y_height, rs_triangle_set_t *tset ) {

    float wall_height = y_start + y_height;


    for (int ti = 0; ti < tset->t_count; ti++) {

        rs_triangle_t *tr = &(tset->t[ti]);

        for (int i = 0; i < 3; i++) {

            rs_vec3_t v0 = rs_vec3( tr->p[0].x, 0.0, tr->p[0].y );
            rs_vec3_t v1 = rs_vec3( tr->p[1].x, 0.0, tr->p[1].y );
            rs_vec3_t v2 = rs_vec3( tr->p[2].x, 0.0, tr->p[2].y );


            v0.x += rvt_reg.shift_x;
            v0.z += rvt_reg.shift_y;
            v1.x += rvt_reg.shift_x;
            v1.z += rvt_reg.shift_y;
            v2.x += rvt_reg.shift_x;
            v2.z += rvt_reg.shift_y;

            float uv_scale = 0.25;

            float hm_height0 = 0.0;
            float hm_height1 = 0.0;
            float hm_height2 = 0.0;
            if (add_hm_height) {
                hm_height0 = rvt_hm_get_height( (v0.x), (v0.z) );
                hm_height1 = rvt_hm_get_height( (v1.x), (v1.z) );
                hm_height2 = rvt_hm_get_height( (v2.x), (v2.z) );
            };

            rvt_reg.rvt_subtile_i = rvt_get_local_subtile_i( (v0.x+v1.x+v2.x)/3, (v0.z+v1.z+v2.z)/3 );

            float perimeter = rs_vec3_distance(v0, v1) + rs_vec3_distance(v1, v2) + rs_vec3_distance(v0, v2);
            if (perimeter > 3.0*rvt_app_get_sc()/RVT_SUBTILES_SIDE_COUNT_PER_TILE ) {
                rvt_reg.rvt_subtile_i = RVT_SUBTILES_COUNT_PER_TILE;
            };

            int v_index = rvt_reg.rvt_vert_current_index[rvt_reg.rvt_writing_layer][rvt_reg.rvt_subtile_i];

            rvt_write_index(v_index + 0);
            rvt_write_index(v_index + 1);
            rvt_write_index(v_index + 2);



            float tangent_w = 1.0; // (v_param.x==1) for roof

            // Horizontal Triangle
            rvt_write_vertex( v0.x, v0.y + wall_height + hm_height0, v0.z, 1.0,   0.0, 1.0, 0.0, 0.0,     1.0, 0.0, 0.0, tangent_w,      uv_scale*v0.x, uv_scale*v0.z, 0.0, 0.0,      rvt_reg.geom_color.x, rvt_reg.geom_color.y, rvt_reg.geom_color.z, 1.0 );
            rvt_write_vertex( v2.x, v2.y + wall_height + hm_height2, v2.z, 1.0,   0.0, 1.0, 0.0, 0.0,     1.0, 0.0, 0.0, tangent_w,      uv_scale*v2.x, uv_scale*v2.z, 0.0, 0.0,      rvt_reg.geom_color.x, rvt_reg.geom_color.y, rvt_reg.geom_color.z, 1.0 );
            rvt_write_vertex( v1.x, v1.y + wall_height + hm_height1, v1.z, 1.0,   0.0, 1.0, 0.0, 0.0,     1.0, 0.0, 0.0, tangent_w,      uv_scale*v1.x, uv_scale*v1.z, 0.0, 0.0,      rvt_reg.geom_color.x, rvt_reg.geom_color.y, rvt_reg.geom_color.z, 1.0 );


        };
    };

};


void rs_geoconv_tc2proj(int zoom_level, double x, double y, double *output_x, double *output_y) {

    double px = ((x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI) * 20037508.342789 / M_PI;
    double py = ( M_PI - y*2.0*M_PI/( pow(2.0, zoom_level) ) ) * 20037508.342789 / M_PI;

    *output_x = px;
    *output_y = py;

};

void rs_geoconv_proj2tc(int zoom_level, double x, double y, double *output_x, double *output_y) {

    double tx =  pow(2.0, zoom_level) * (  1.0 + x/20037508.342789 ) / 2.0;
    double ty = -pow(2.0, zoom_level) * ( -1.0 + y/20037508.342789 ) / 2.0;

    *output_x = tx;
    *output_y = ty;

};

rs_vec2_t rs_geoconv_latlon2tc(double lat, double lon) {

    int zoom_level = 0;

    double x;
    double y;

    lon *= M_PI/180.0;
    lat *= M_PI/180.0;

    x = 0.5 / M_PI * pow(2.0, zoom_level) * (M_PI + lon);
    y = 0.5 / M_PI * pow(2.0, zoom_level) * ( M_PI - log( tan(M_PI/4.0 + lat/2.0) ) );

    return rs_vec2( (float)x, (float)y );

};

rs_vec2_t rs_geoconv_tc2degrees(int zoom_level, double x, double y) {

    double lon;
    double lat;

    lon = (x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI;
    lat = 2.0 * atan(exp(M_PI - y*2.0*M_PI/pow(2.0, zoom_level))) - M_PI/2.0;

    lon *= 180.0 / M_PI;
    lat *= 180.0 / M_PI;

    return rs_vec2(lat, lon);

};

int osm_value_is_non_zero(char *s) {
    if ( s[0] ) {
        if ( strcmp(s, "no") ) { // is not "no"
            if ( strcmp(s, "0") ) { // is not "0"
                return 1;
            };
        };
    };
    return 0;
};

float rvt_calc_sc( rs_vec2_t latlon ) {

    // Side of the z14-tile size at equator is 40 075 696 / 16384 = 2446.02636719 meters
    // 4 meters in 1 unit
    float sc = 2446.02636719 / 4.0;

    float merc_scale_factor = cos(M_PI*latlon.x/180.0);

    return sc * merc_scale_factor;

};

uint32_t rvt_get_building_type_from_string(char *s_value) {


    if (!strcmp(s_value, "apartments")) {
        return OSM_TAG_BUILDING_APARTMENTS;
    }
    else if (!strcmp(s_value, "residential")) {
        return OSM_TAG_BUILDING_RESIDENTIAL;
    }
    else if (!strcmp(s_value, "house")) {
        return OSM_TAG_BUILDING_HOUSE;
    }
    else if (!strcmp(s_value, "detached")) {
        return OSM_TAG_BUILDING_DETACHED;
    }
    else if (!strcmp(s_value, "semidetached_house")) {
        return OSM_TAG_BUILDING_SEMIDETACHED_HOUSE;
    }
    else if (!strcmp(s_value, "semi")) {
        return OSM_TAG_BUILDING_SEMI;
    }
    else if (!strcmp(s_value, "hut")) {
        return OSM_TAG_BUILDING_HUT;
    }
    else if (!strcmp(s_value, "terrace")) {
        return OSM_TAG_BUILDING_TERRACE;
    }
    else if (!strcmp(s_value, "dormitory")) {
        return OSM_TAG_BUILDING_DORMITORY;
    }





    else if (!strcmp(s_value, "commercial")) {
        return OSM_TAG_BUILDING_COMMERCIAL;
    }
    else if (!strcmp(s_value, "retail")) {
        return OSM_TAG_BUILDING_RETAIL;
    }
    else if (!strcmp(s_value, "office")) {
        return OSM_TAG_BUILDING_OFFICE;
    }
    else if (!strcmp(s_value, "public")) {
        return OSM_TAG_BUILDING_PUBLIC;
    }
    else if (!strcmp(s_value, "shop")) {
        return OSM_TAG_BUILDING_SHOP;
    }
    else if (!strcmp(s_value, "supermarket")) {
        return OSM_TAG_BUILDING_SUPERMARKET;
    }

    else if (!strcmp(s_value, "hospital")) {
        return OSM_TAG_BUILDING_HOSPITAL;
    }
    else if (!strcmp(s_value, "school")) {
        return OSM_TAG_BUILDING_SCHOOL;
    }
    else if (!strcmp(s_value, "kindergarten")) {
        return OSM_TAG_BUILDING_KINDERGARTEN;
    }
    else if (!strcmp(s_value, "university")) {
        return OSM_TAG_BUILDING_UNIVERSITY;
    }
    else if (!strcmp(s_value, "college")) {
        return OSM_TAG_BUILDING_COLLEGE;
    }

    else if (!strcmp(s_value, "church")) {
        return OSM_TAG_BUILDING_CHURCH;
    }
    else if (!strcmp(s_value, "cathedral")) {
        return OSM_TAG_BUILDING_CATHEDRAL;
    }






    else if (!strcmp(s_value, "industrial")) {
        return OSM_TAG_BUILDING_INDUSTRIAL;
    }
    else if (!strcmp(s_value, "garages")) {
        return OSM_TAG_BUILDING_GARAGES;
    }
    else if (!strcmp(s_value, "warehouse")) {
        return OSM_TAG_BUILDING_WAREHOUSE;
    }
    else if (!strcmp(s_value, "service")) {
        return OSM_TAG_BUILDING_SERVICE;
    }
    else if (!strcmp(s_value, "farm")) {
        return OSM_TAG_BUILDING_FARM;
    }
    else if (!strcmp(s_value, "hangar")) {
        return OSM_TAG_BUILDING_HANGAR;
    }
    else if (!strcmp(s_value, "silo")) {
        return OSM_TAG_BUILDING_SILO;
    }


    else if (!strcmp(s_value, "collapsed")) {
        return OSM_TAG_BUILDING_COLLAPSED;
    }
    else if (!strcmp(s_value, "ruins")) {
        return OSM_TAG_BUILDING_RUINS;
    }
    else if (!strcmp(s_value, "abandoned")) {
        return OSM_TAG_BUILDING_ABANDONED;
    }
    else if (!strcmp(s_value, "construction")) {
        return OSM_TAG_BUILDING_CONSTRUCTION;
    }


    else if (!strcmp(s_value, "wall")) {
        return OSM_TAG_BUILDING_WALL;
    };

    return OSM_TAG_YES;

};

uint32_t rvt_get_colour_from_string(char *s) {

    uint32_t c;

    if (s[0] == '#' ) {
        int res = sscanf(s+1, "%x", &c);
        if (res == 0) {
            return RVT_NO_COLOR;
        };
        if (strlen(s+1) == 3) {
            int cr = (c & 0xF00) >> 8;
            int cg = (c & 0x0F0) >> 4;
            int cb = c & 0x00F;
            c = (cr<<20) | (cr<<16) | (cg<<12) | (cg<<8) | (cb<<4) | (cb<<0);
        };
        return (c & 0x00FFFFFF);
    }
    else if (s[0] == '\0') {
        return RVT_NO_COLOR;
    }
    else {

        uint32_t hexcode;
        if ( rvt_app_get_hexcode_by_color_string(s, &hexcode) ) {
            return hexcode;
        };

    };

    return RVT_NO_COLOR;

};

int rvt_calc_building_levels_by_shape(rs_shape_t *p) {

    float area = rs_shape_area(p);

    float sc2 = 1.0 / rvt_app_get_sc() * rvt_app_get_sc();

    if (area < 11.0*sc2) {
        return 1;
    };

    if (area < 41.0*sc2) {
        return 2;
    };

    if (area < 210.0*sc2) {
        return 4;
    };

    return 5;

};
