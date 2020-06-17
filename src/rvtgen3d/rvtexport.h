#ifndef RS_RVTEXPORT_H
#define RS_RVTEXPORT_H

#include <stdio.h>
#include <inttypes.h>

#include "rvttypes.h"

void rvt_export_file_init(rvt_export_file_t *rvtfile, int file_format, int z_up, int vertex_type, int z, int x, int y, int layer_i);
void rvt_export_file_term(rvt_export_file_t *rvtfile);

void rvt_export_file_start_new_tile(rvt_export_file_t *rvtfile, int z, int x, int y, int layer_index);

#define RVT_EXPORT_V_TYPE_12        0
#define RVT_EXPORT_V_TYPE_16        1
#define RVT_EXPORT_V_TYPE_BUILDING  2

void rvt_export_file_write_vertices(rvt_export_file_t *rvtfile, float *p_vertices, int v_count );
void rvt_export_file_write_indices(rvt_export_file_t *rvtfile, uint32_t *p_indices, int i_count, int v_start_shift );

void rvt_export_file_write( rvt_export_file_t *rvtfile, float *p_vertices, int v_count, uint32_t *p_indices, int i_count );



#endif
