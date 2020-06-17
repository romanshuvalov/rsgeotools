#ifndef RVTUTIL_H
#define RVTUTIL_H

#include <inttypes.h>

#include "rvt.h"
#include "rvttypes.h"

int rvt_point_is_in_area(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags, rvt_gtriangle_t **gtriangle_result_p);


void rvt_gp_add(int layer_i, int dest_tile_i, rs_point_t pos, float azimuth, void *pdata, int flags, float radius);

void rvt_gline_add(int layer_i, int dest_tile_i, rs_point_t p1, rs_point_t p2, gd_lines_data_t *pdata, int flags, float thickness);
void rvt_gline_add_shape(int layer_i, int dest_tile_i, float shift_x, float shift_y, rs_shape_t *src_sh, gd_lines_data_t *pdata, int flags, float thickness, int is_already_segmentized);

void rvt_gtriangle_add(int layer_i, int dest_tile_i, int flags, float shift_x, float shift_y, rs_triangle_t *src_triangle);
void rvt_gtriangle_add_triangle_set(int layer_i, int dest_tile_i, int flags, float shift_x, float shift_y, rs_triangle_set_t *src_tset);


int rvt_get_tile_i_by_subtile_i(int subtile_i);

int rvt_get_subtile_i_limited_by_tile_i(float x, float z, int dest_tile_i);

int rvt_get_tile_i(float x, float z);
int rvt_get_subtile_i(float x, float z);


rvt_gpoint_t* rvt_gp_find_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags);
float rvt_gp_distance_to_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags);

float rvt_gline_find_nearest(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags,  rvt_gline_t **gline_result_p);
float rvt_near_glines_length(rs_vec2_t p, int layer_i, int required_flags, int exclude_flags, float radius);

float rvt_hm_get_height(float x, float y);
float rvt_hm_get_height_adv(float fpx, float fpy, int tile_x, int tile_y);

void rvt_create_wire_vbodata(int vbodata_index, rs_vec3_t p0, rs_vec3_t p1, rs_vec3_t p2, float halfwidth, int segments_count);

#endif
