#include "rvtapp.h"

#include "main.h"

#include <string.h>

#include <unistd.h>

float rvt_app_get_sc() {
    return rvt_app->sc;
};

float rvt_app_get_sc_z() {
    return rvt_app->sc_z;
};

int rvt_app_is_terrain_flat_by_default() {
    return (rvt_app->flags & RVT_APP_FLAG_FLAT_TERRAIN) ? 1 : 0;
};

int rvt_app_get_stride_by_layer(int layer) {
    return rvt_app->rvt_stride[layer];
};

int rvt_app_get_conf_index_by_layer(int vbo_layer_index) {
    return rvt_app->rvt_vbo_conf_index[vbo_layer_index];
};

int rvt_app_is_exporting() {
    return rvt_app_get_exporting_struct()->exporting;
};


void rvt_app_exporting_total_bytes_append(uint64_t bytes) {
    rvt_app_get_exporting_struct()->exporting_total_bytes += bytes;
};

rvt_vbodata_t* rvt_app_get_vbodata_by_index(int index) {
    return &rvt_app->vbodata[index];
};

int rvt_app_get_hexcode_by_color_string(char *s, int *p_output) {

    for (int i = 0; i < rvt_app->color_recs_count; i++) {
        if (!strcmp(s, rvt_app->color_recs[i].s)) {
            *p_output = rvt_app->color_recs[i].hexcode;
            return 1;
        };
    };

    return 0;
};

int rvt_app_get_rvt_style() {
    return rvt_app->rvt_style;
};

int rvt_app_get_visstyle_index() {
    return rvt_app->visstyle_index;
};

rvt_geodata_struct_t* rvt_app_get_geodata() {
    return &rvt_app->geodata;
};

void rvt_app_update_history_game_bases() {
    // Not used in this tool
};

rvt_gnote_t *rvt_get_gnote_by_point(rvt_gpoint_t *gp) {
    // Not used in this tool
    return NULL;
};

void rvt_app_create_subtile_raw_vbo(int stage_i) {
    // Not used in this tool
};

int rvt_app_are_vertices_per_frame_unlimited() {

    return ( (!rvt_app->tiles_initial_loading_is_passed) || (rvt_app->termination_process) || (rvt_app_is_exporting()) );

};

char *rvt_app_get_lang() {
    return rvt_app->user_lang;
};

char *rvt_app_get_alt_lang() {
    return rvt_app->user_alt_lang;
};

void rvt_app_create_terrain_vbo( int vbo_index, unsigned char *data, int data_count, int stride ) {
    // Not used in this tool
};


rvt_cache_settings_t *rvt_app_get_cache_settings() {

    return &rvt_app->rvt_cache_settings;

};

rvt_exporting_struct_t* rvt_app_get_exporting_struct() {
    return &rvt_app->exporting_struct;
};

void rvt_app_sleep_msec(int msec) {

    usleep(msec*1000);

};

void rvt_app_get_date(int *y, int *m, int *d, int *h, int *i, int *s) {

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);

    *y = aTime->tm_year + 1900; // Year is # years since 1900
    *m = aTime->tm_mon + 1; // Month is 0 - 11, adding 1 to get a 1-12
    *d = aTime->tm_mday;

    *h = aTime->tm_hour;
    *i = aTime->tm_min;
    *s = aTime->tm_sec;

};
