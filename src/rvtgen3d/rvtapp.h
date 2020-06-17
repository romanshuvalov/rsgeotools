#ifndef RVTAPP_H
#define RVTAPP_H

#include <inttypes.h>

#include "rvt.h"


rvt_vbodata_t* rvt_app_get_vbodata_by_index(int index);

float rvt_app_get_sc();
float rvt_app_get_sc_z();

int rvt_app_is_terrain_flat_by_default();

int rvt_app_get_stride_by_layer(int layer);

int rvt_app_get_conf_index_by_layer(int vbo_layer_index);

int rvt_app_is_exporting();

void rvt_app_exporting_total_bytes_append(uint64_t bytes);


rvt_vbodata_t* rvt_app_get_vbodata_by_index(int index);

int rvt_app_get_hexcode_by_color_string(char *s, int *p_output);


int rvt_app_get_rvt_style();
int rvt_app_get_visstyle_index();

rvt_geodata_struct_t* rvt_app_get_geodata();

void rvt_app_update_history_game_bases();

rvt_gnote_t *rvt_get_gnote_by_point(rvt_gpoint_t *gp);

void rvt_app_create_subtile_raw_vbo(int i);

int rvt_app_are_vertices_per_frame_unlimited();

char *rvt_app_get_lang();
char *rvt_app_get_alt_lang();

void rvt_app_create_terrain_vbo( int vbo_index, unsigned char *data, int data_count, int stride );

rvt_cache_settings_t *rvt_app_get_cache_settings();

rvt_exporting_struct_t* rvt_app_get_exporting_struct();

void rvt_app_sleep_msec(int msec);

void rvt_app_get_date(int *y, int *m, int *d, int *h, int *i, int *s);

#endif
