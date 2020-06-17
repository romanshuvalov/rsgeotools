#include "tileloader.h"

#include "rvtgen.h"
#include "rvtloader.h"

#ifdef STREETS_GAME
#include "rsbullet.h" // for heightmap
#endif

#include <string.h>

#include "rs/rsdebug.h"

rvt_tile_loader_struct_t rvt_tile_loader_reg;

RS_THREAD_T rvt_tile_loader_thread;

RS_THREAD_FUNC_T rvt_tile_loader_thread_func() {

    int subpak_z;

    // 1. heightmap

    subpak_z = 11;

    unsigned char *super_tile_data;
    unsigned int super_tile_data_len;

    heightmap_load_tile_to_data( &super_tile_data, &super_tile_data_len,
            rvt_tile_loader_reg.tile_z, rvt_tile_loader_reg.tile_world_ix, rvt_tile_loader_reg.tile_world_iy,
            subpak_z, 0 /* !! unused */, rvt_tile_loader_reg.tile_ix, rvt_tile_loader_reg.tile_iy,
            rvt_tile_loader_reg.tile_px, rvt_tile_loader_reg.tile_py);


    RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);
    rvt_tile_loader_reg.super_tile_data = super_tile_data;
    rvt_tile_loader_reg.super_tile_data_len = super_tile_data_len;
    rvt_tile_loader_reg.hm_ready = 1;

    RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);

    // wait until pending heightmap of current layer is created by main thread
    int loop = 1;
    while (loop) {
        rvt_app_sleep_msec(1);
        RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);
        loop = (rvt_tile_loader_reg.hm_ready != 0);
        RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);
    };


    // 2. geodata

    subpak_z = 12;


    for (int i = 0; i < 4; i++) {

        rvt_load_tile(14, rvt_tile_loader_reg.tile_world_ix, rvt_tile_loader_reg.tile_world_iy, subpak_z, i,
                        rvt_tile_loader_reg.tile_ix, rvt_tile_loader_reg.tile_iy, rvt_tile_loader_reg.tile_px, rvt_tile_loader_reg.tile_py,
                        rvt_app_get_sc() );

    };
    gd_gen_geom(rvt_tile_loader_reg.tile_i);

    rvt_app_get_geodata()->tile[rvt_tile_loader_reg.tile_i].status = RVT_TILE_STATUS_LOADED;

    DEBUG20f("gd_gen_geom() done, tile loader done. tile_i %d status is LOADED.\n", rvt_tile_loader_reg.tile_i);

    rvt_tile_loader_reg.status = RVT_TILE_LOADER_STATUS_OFF;
    rvt_app_increase_threads_count(-1); // rs_app.child_threads_count--;
    RS_THREAD_RETURN(0);

};

void rvt_tile_loader_load_tile(int tile_z, int tile_world_ix, int tile_world_iy, int tile_i, int tile_px, int tile_py) {

    RS_THREAD_FUNC_T* func = (RS_THREAD_FUNC_T*) &rvt_tile_loader_thread_func;

    rvt_tile_loader_reg.tile_z = tile_z;
    rvt_tile_loader_reg.tile_world_ix = tile_world_ix;
    rvt_tile_loader_reg.tile_world_iy = tile_world_iy;

    rvt_tile_loader_reg.tile_i = tile_i;
    rvt_tile_loader_reg.tile_ix = tile_i % RVT_TILES_SIDE_COUNT;
    rvt_tile_loader_reg.tile_iy = tile_i / RVT_TILES_SIDE_COUNT;

    rvt_tile_loader_reg.tile_px = tile_px;
    rvt_tile_loader_reg.tile_py = tile_py;

    rvt_tile_loader_reg.status = RVT_TILE_LOADER_STATUS_LOADING;


    RS_MUTEX_INIT(&rvt_tile_loader_reg.mutex);
    RS_THREAD_CREATE(rvt_tile_loader_thread, ( void*(*)(void*) ) func, 0);
    rvt_app_increase_threads_count(1); // rs_app.child_threads_count++;


};

void rvt_tile_loader_init() {

    memset(&rvt_tile_loader_reg, 0, sizeof(rvt_tile_loader_struct_t) );

};

void rvt_tile_loader_halt() {

    return; // Not implemented in this version

};

#ifndef VBO_TERRAIN
    #define VBO_TERRAIN 0
#endif // VBO_TERRAIN

void rvt_tile_loader_check_mainthread() {


    RS_MUTEX_LOCK(&rvt_tile_loader_reg.mutex);

    if (rvt_tile_loader_reg.status == RVT_TILE_LOADER_STATUS_ERROR_CORRUPTED_FILE) {

        rs_show_message("Corrupted file!");
        rvt_tile_loader_reg.status = RVT_TILE_LOADER_STATUS_OFF;

    };

    if (rvt_tile_loader_reg.hm_ready) {

        int subpak_z = 11;
        int vbo_index = VBO_TERRAIN + rvt_tile_loader_reg.tile_i;

        heightmap_load_tile_from_data( rvt_tile_loader_reg.super_tile_data, rvt_tile_loader_reg.super_tile_data_len,
            14, rvt_tile_loader_reg.tile_world_ix, rvt_tile_loader_reg.tile_world_iy, subpak_z,
            vbo_index, rvt_tile_loader_reg.tile_ix,
            rvt_tile_loader_reg.tile_iy,
            rvt_tile_loader_reg.tile_px, rvt_tile_loader_reg.tile_py);

        rvt_app_get_geodata()->hm[rvt_tile_loader_reg.tile_i].status = 1;

        #ifdef STREETS_GAME
            int field_size = 33; // 129;
            rsph_add_heightmap(rvt_tile_loader_reg.tile_i, field_size, rvt_tile_loader_reg.tile_px, rvt_tile_loader_reg.tile_py); //
        #endif

        rvt_app_update_history_game_bases();

        rvt_tile_loader_reg.hm_ready = 0;

    };

    if (rvt_tile_loader_reg.st_ready_count > 0) {


        int max_vertices_per_frame = 15000;
        int max_stages_per_frame = 12;

        if ( rvt_app_are_vertices_per_frame_unlimited() ) {
            max_vertices_per_frame = 99000000;
            max_stages_per_frame = 99999;
        };


        int total_vertices_written = 0;
        int total_indices_written = 0;
        int total_stages_passed = 0;

        for (int i = rvt_tile_loader_reg.st_ready_count-1; (i >= 0) && (i > rvt_tile_loader_reg.st_ready_count-1-max_stages_per_frame); i--) {

            if (!rvt_app_is_exporting()) {
                rvt_app_create_subtile_raw_vbo(i);
            };

            total_stages_passed++;
            total_vertices_written += rvt_tile_loader_reg.st_rec[i].st_total_vertices;
            total_indices_written += rvt_tile_loader_reg.st_rec[i].st_total_indices;
            if (total_vertices_written > max_vertices_per_frame) {
                break;
            };

            if (i > 0) {
                if ( (total_vertices_written + rvt_tile_loader_reg.st_rec[i].st_total_vertices) > max_vertices_per_frame ) {
                    break;
                }
            };


        };

        rvt_tile_loader_reg.st_ready_count = rs_max_i(0, rvt_tile_loader_reg.st_ready_count - total_stages_passed);

    };

    RS_MUTEX_UNLOCK(&rvt_tile_loader_reg.mutex);

};
