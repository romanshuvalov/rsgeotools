#ifndef RVTLOADER_H
#define RVTLOADER_H

#include "rvt.h"

void rvt_load_tile(int tile_z, int tile_x, int tile_y, int subpak_z, int part_i, int tile_ix, int tile_iy, int tile_px, int tile_py, float tile_side_units);

void heightmap_load_tile_to_data(unsigned char **output_super_tile_data, unsigned int *output_super_tile_data_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int vbo_index, int tile_ix, int tile_iy, int tile_x, int tile_y);
void heightmap_load_tile_from_data(unsigned char *super_tile_data, unsigned int super_tile_data_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int vbo_index, int tile_ix, int tile_iy, int tile_x, int tile_y);

void rvt_process_file(unsigned char **pdata, unsigned int *pdata_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int data_type, int part_i);


#endif
