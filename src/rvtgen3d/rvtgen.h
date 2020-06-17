#ifndef RVTGEN_H
#define RVTGEN_H

#include "rvt.h"


void rvt_gen_init_default_stages();

void rvt_rural_write_stuff( rs_point_t p, float azimuth, float distance, float shift_forward, float last_shift_forward, float max_side_wall_len, int last, int rand_i );
void rvt_barrier_write_common( int flags, int barrier_osm_value, float y_start, int barrier_height, int wall_type_index, rs_shape_t *p );


void rvt_barrier_wall_area(rs_shape_t *p, int stage_i, gd_area_data_t *data);
void rvt_barrier_wall_line(rs_shape_t *p, int stage_i, gd_lines_data_t *data);

void rvt_road_rural_houses(rs_shape_t *p, int stage_i,  gd_lines_data_t *sdata);

void rvt_road_lamps(rs_shape_t *p, int stage_i, gd_lines_data_t *data);
void rvt_road_footway_trees(rs_shape_t *p, int stage_i, gd_lines_data_t *data);


void rvt_railway_towers(rs_shape_t *p, int stage_i,  gd_lines_data_t *sdata);

void rvt_railway(rs_shape_t *p, int stage_i, gd_lines_data_t *data);

void rvt_road(rs_shape_t *p, int stage_i, gd_lines_data_t *data);
void rvt_river(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata);

void rvt_area(rs_shape_t *p, int stage_i, gd_area_data_t *sdata);

void rvt_bridge(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata);
void rvt_tunnel(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata);

void rvt_building_add_point(rs_shape_t *p, int stage_i, gd_building_data_t *data);


void rvt_building(rs_shape_t *p, int stage_i, gd_building_data_t *sdata, rvt_gnote_t *gnote);

void rvt_crossing(rs_vec2_t *v, int stage_i, gd_point_data_t *sdata);
void rvt_level_crossing(rs_vec2_t *v, int stage_i, gd_point_data_t *data);

void rvt_natural_tree(rs_vec2_t *v, int stage_i, gd_point_data_t *data);
void rvt_building_entrance(rs_vec2_t *v, int stage_i, gd_point_data_t *sdata);
void rvt_barrier_block(rs_vec2_t *v, int stage_i, gd_point_data_t *data);

void rvt_powerline(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata);

void rvt_all_forest(rs_shape_t *unused_shape, int stage_i, gd_area_data_t *unused_data);
void rvt_all_buildings(void *unused_geometry, int stage_i, gd_building_data_t *unused_data);

void rvt_full_tile_naturals(void *geometry, int stage_i, void *data);

void gd_gen_geom(int tile_i);

#endif // RVTGEN_H
