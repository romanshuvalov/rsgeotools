#ifndef TILELOADER_H
#define TILELOADER_H


#include "rvtapp.h"
#include "rvttypes.h"

typedef struct rvt_tile_loader_st_rec_t {

    float *st_data;
    int st_total_vertices;
    int st_stride;
    int *st_ind_data;
    int st_total_indices;
    #ifdef STREETS_GAME
    int st_ind_type;
    int st_geom_type;
    #endif
    int st_vbo_conf_index;

    int st_subtile_i;
    int st_layer_i;

    int st_igroup_start[RVT_IGROUPS_COUNT];
    int st_igroup_indices_count[RVT_IGROUPS_COUNT];

} rvt_tile_loader_st_rec_t;

#define RVT_TILE_LOADER_MAX_ST_RECS_COUNT      32

typedef struct rvt_tile_loader_struct_t {

    int status;
    int progress;

    int hm_ready;
    unsigned char *super_tile_data;
    int super_tile_data_len;

    int st_ready_count;
    rvt_tile_loader_st_rec_t st_rec[RVT_TILE_LOADER_MAX_ST_RECS_COUNT];

    int tile_i;
    int tile_ix;
    int tile_iy;

    int tile_z;
    int tile_x;
    int tile_y;

    int tile_world_ix;
    int tile_world_iy;

    int tile_px;
    int tile_py;

    int current_layer;
    int custom_subtile_i;

    RS_MUTEX_T mutex;


} rvt_tile_loader_struct_t;

extern rvt_tile_loader_struct_t rvt_tile_loader_reg;


#define RVT_TILE_LOADER_STATUS_OFF     0
#define RVT_TILE_LOADER_STATUS_LOADING 1

#define RVT_TILE_LOADER_STATUS_ERROR_CORRUPTED_FILE    8


RS_THREAD_FUNC_T rvt_tile_loader_thread_func();

void rvt_tile_loader_load_tile(int tile_z, int tile_world_ix, int tile_world_iy, int tile_i, int tile_px, int tile_py);
void rvt_tile_loader_init();

void rvt_tile_loader_halt();


void rvt_tile_loader_check_mainthread();


#endif // TILELOADER_H
