#include "rvtexport.h"

#include "rvt.h"

#include <unistd.h> // for unlink

void rvt_export_file_init(rvt_export_file_t *rvtfile, int file_format, int z_up, int v_type, int z, int x, int y, int layer_i) {

    const char *ext = (file_format == RVT_FORMAT_OBJ) ? "obj" : "ply";

    char filename[200];
    if ( (z+x+y) == 0 ) {
        sprintf(filename, "%s/layer%d.%s", rvt_settings.folder_name, layer_i, ext);
    }
    else {
        sprintf(filename, "%s/tile_%d_%d_%d_layer%d.%s", rvt_settings.folder_name, z, x, y, layer_i, ext );
    };

    rvtfile->fp = fopen(filename, "w+");


    if (file_format != RVT_FORMAT_OBJ) {

        sprintf(rvtfile->filename_temp_v, "%s.v.tmp", filename);
        rvtfile->fp_temp_v = fopen(rvtfile->filename_temp_v, "w+");

        sprintf(rvtfile->filename_temp_i, "%s.i.tmp", filename);
        rvtfile->fp_temp_i = fopen(rvtfile->filename_temp_i, "w+");

    };


    rvtfile->v_type = v_type;

    rvtfile->file_format = file_format;

    if (file_format == RVT_FORMAT_OBJ) {

        rvtfile->total_bytes += fprintf(rvtfile->fp, "# Generated using Generation Streets by Roman Shuvalov, streets.romanshuvalov.com\n");
        rvtfile->total_bytes += fprintf(rvtfile->fp, "# Geodata (c) OpenStreetMap Contributors, www.openstreetmap.org/copyright\n");

        rvtfile->total_bytes += fprintf(rvtfile->fp, "# Vertex format: PosX, PosY, PosZ, ColorR, ColorG, ColorB, FlagsA, FlagsB\n\n");

    }
    else { // if (file_format == RVT_FORMAT_PLY_ASCII) {
        // PLY

        rvtfile->total_bytes += fprintf(rvtfile->fp,
            "ply\n"
            "format ascii 1.0\n"
            "comment Generated using Generation Streets by Roman Shuvalov, streets.romanshuvalov.com\n"
            "comment Geodata (c) OpenStreetMap Contributors, www.openstreetmap.org/copyright\n"
            );

    };

    rvtfile->z_up = z_up;

    rvtfile->vertices_count = 0;
    rvtfile->faces_count = 0;

    rvtfile->total_bytes = 0;

    rvtfile->scale = 1.0/64.0;

};

void rvt_export_file_term(rvt_export_file_t *rvtfile) {


    if (rvtfile->file_format == RVT_FORMAT_OBJ) {

        rvtfile->total_bytes += fprintf(rvtfile->fp, "\n# Total vertices: %d\n", rvtfile->vertices_count);
        rvtfile->total_bytes += fprintf(rvtfile->fp, "# Total faces: %d\n", rvtfile->faces_count);
        rvtfile->total_bytes += fprintf(rvtfile->fp, "# End\n");


    }

    else { // if (rvtfile->file_format == RVT_FORMAT_PLY_ASCII)  {

        if ( rvtfile->v_type == RVT_EXPORT_V_TYPE_12 ) {


            rvtfile->total_bytes += fprintf(rvtfile->fp,
                "element vertex %d\n"
                "property float x\n"
                "property float y\n"
                "property float z\n"
                "property float nx\n"
                "property float ny\n"
                "property float nz\n"
                "property float red\n"
                "property float green\n"
                "property float blue\n"
                "element face %d\n"
                "property list uchar uint vertex_indices\n"
                "end_header\n",
                rvtfile->vertices_count, rvtfile->faces_count
                );

        }

        else {

            rvtfile->total_bytes += fprintf(rvtfile->fp,
                "element vertex %d\n"
                "property float x\n"
                "property float y\n"
                "property float z\n"
                "property float nx\n"
                "property float ny\n"
                "property float nz\n"
                "property float nw\n"
                "property float s\n"
                "property float t\n"
                "property float red\n"
                "property float green\n"
                "property float blue\n"
                "property float alpha\n"
                "element face %d\n"
                "property list uchar uint vertex_indices\n"
                "end_header\n",
                rvtfile->vertices_count, rvtfile->faces_count
                );

        };


        char buf[BUFSIZ];

        size_t size;

        rewind(rvtfile->fp_temp_v);
        rewind(rvtfile->fp_temp_i);

        while (size = fread(buf, 1, BUFSIZ, rvtfile->fp_temp_v)) {
            fwrite(buf, 1, size, rvtfile->fp);
        };

        while (size = fread(buf, 1, BUFSIZ, rvtfile->fp_temp_i)) {
            fwrite(buf, 1, size, rvtfile->fp);
        };

        fclose(rvtfile->fp_temp_v);
        fclose(rvtfile->fp_temp_i);

        unlink( rvtfile->filename_temp_v );
        unlink( rvtfile->filename_temp_i );

    };

    fclose(rvtfile->fp);

    rvtfile->fp = NULL;

};

void rvt_export_file_start_new_tile(rvt_export_file_t *rvtfile, int z, int x, int y, int layer_index) {

    if (rvtfile->file_format == RVT_FORMAT_OBJ) {

        rvtfile->total_bytes += fprintf(rvtfile->fp, "\no tile_%d_%d_%d_layer%d\n\n", z, x, y, layer_index);

    };

};

void rvt_export_file_write_vertices(rvt_export_file_t *rvtfile, float *p_vertices, int v_count ) {

    if (rvtfile->v_type == RVT_EXPORT_V_TYPE_BUILDING) {

        int stride = 16;

        for (int vi = 0; vi < v_count; vi++) {

            if (rvtfile->file_format == RVT_FORMAT_OBJ) {

                rvtfile->total_bytes += fprintf(rvtfile->fp, "v %.6f %.6f %.6f %.2f %.2f %.2f %.4f %.4f\n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            p_vertices[ vi*stride + 12], p_vertices[ vi*stride + 13], p_vertices[ vi*stride + 14],

                            p_vertices[ vi*stride + 15], p_vertices[ vi*stride + 11]
                             );

                rvtfile->total_bytes += fprintf(rvtfile->fp, "vn %.3f %.3f %.3f\n",
                            p_vertices[ vi*stride + 3],
                            p_vertices[ vi*stride + 4]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 5]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 4]*(rvtfile->z_up)
                                                );
                rvtfile->total_bytes += fprintf(rvtfile->fp, "vt %.2f %.2f\n\n", p_vertices[ vi*stride + 6], p_vertices[ vi*stride + 7] );

            }
            else if (rvtfile->file_format == RVT_FORMAT_PLY_ASCII) {

                rvtfile->total_bytes += fprintf(rvtfile->fp_temp_v, "%.6f %.6f %.6f %.3f %.3f %.3f %.1f %.3f %.3f %.3f %.3f %.3f %.1f\n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            p_vertices[ vi*stride + 3],
                            p_vertices[ vi*stride + 4]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 5]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 4]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 11],

                            p_vertices[ vi*stride + 6], p_vertices[ vi*stride + 7],

                            p_vertices[ vi*stride + 12], p_vertices[ vi*stride + 13],
                            p_vertices[ vi*stride + 14], p_vertices[ vi*stride + 15]

                             );

            };

            rvtfile->vertices_count++;

        };

    }

    else if (rvtfile->v_type == RVT_EXPORT_V_TYPE_16) {

        int stride = 16;

        for (int vi = 0; vi < v_count; vi++) {

            if (rvtfile->file_format == RVT_FORMAT_OBJ) {

                rvtfile->total_bytes += fprintf(rvtfile->fp, "v %.6f %.6f %.6f %.2f %.2f %.2f \n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            p_vertices[ vi*stride + 12], p_vertices[ vi*stride + 13], p_vertices[ vi*stride + 14]
                             );
                rvtfile->total_bytes += fprintf(rvtfile->fp, "vn %.3f %.3f %.3f\n",
                            p_vertices[ vi*stride + 4],
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 6]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 6]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 5]*(rvtfile->z_up)
                                                );
                rvtfile->total_bytes += fprintf(rvtfile->fp, "vt %.2f %.2f\n\n", p_vertices[ vi*stride + 8], p_vertices[ vi*stride + 9] );

            }
            else if (rvtfile->file_format == RVT_FORMAT_PLY_ASCII) {


                rvtfile->total_bytes += fprintf(rvtfile->fp_temp_v, "%.6f %.6f %.6f %.3f %.3f %.3f %.1f %.3f %.3f %.3f %.3f %.3f %.1f\n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            p_vertices[ vi*stride + 4],
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 6]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 6]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 5]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 7],

                            p_vertices[ vi*stride + 8], p_vertices[ vi*stride + 9],

                            p_vertices[ vi*stride + 12], p_vertices[ vi*stride + 13],
                            p_vertices[ vi*stride + 14], rvtfile->scale*p_vertices[ vi*stride + 15]

                             );

            };

            rvtfile->vertices_count++;

        };

    }
    else if (rvtfile->v_type == RVT_EXPORT_V_TYPE_12) {

        int stride = 12;

        for (int vi = 0; vi < v_count; vi++) {

            if (rvtfile->file_format == RVT_FORMAT_OBJ) {

                rvtfile->total_bytes += fprintf(rvtfile->fp, "v %.6f %.6f %.6f %.2f %.2f %.2f \n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            rvtfile->scale*p_vertices[ vi*stride + 8], rvtfile->scale*p_vertices[ vi*stride + 9], rvtfile->scale*p_vertices[ vi*stride + 10]
                             );
                rvtfile->total_bytes += fprintf(rvtfile->fp, "vn %.3f %.3f %.3f\n",
                            p_vertices[ vi*stride + 4],
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 6]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 6]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 5]*(rvtfile->z_up)
                                                );
    //            fprintf(rvtfile->fp, "vt %.2f %.2f\n\n", p_vertices[ vi*stride + 8], p_vertices[ vi*stride + 9] );

            }
            else if (rvtfile->file_format == RVT_FORMAT_PLY_ASCII) {

                rvtfile->total_bytes += fprintf(rvtfile->fp_temp_v, "%.6f %.6f %.6f %.3f %.3f %.3f %.3f %.3f %.3f \n",
                            rvtfile->scale*p_vertices[ vi*stride + 0],
                            rvtfile->scale*p_vertices[ vi*stride + 1]*(1-rvtfile->z_up) - rvtfile->scale*p_vertices[ vi*stride + 2]*rvtfile->z_up,
                            rvtfile->scale*p_vertices[ vi*stride + 2]*(1-rvtfile->z_up) + rvtfile->scale*p_vertices[ vi*stride + 1]*rvtfile->z_up,

                            p_vertices[ vi*stride + 4],
                            p_vertices[ vi*stride + 5]*(1-rvtfile->z_up) - p_vertices[ vi*stride + 6]*(rvtfile->z_up),
                            p_vertices[ vi*stride + 6]*(1-rvtfile->z_up) + p_vertices[ vi*stride + 5]*(rvtfile->z_up),

                            p_vertices[ vi*stride + 8], p_vertices[ vi*stride + 9], p_vertices[ vi*stride + 10]

                             );

            };



            rvtfile->vertices_count++;

        };

    }
    else {
        // unknown vertex type
    };

};

void rvt_export_file_write_indices(rvt_export_file_t *rvtfile, uint32_t *p_indices, int i_count, int v_start_shift ) {

    if ( (i_count%3) != 0 ) {
        fprintf(rvtfile->fp, "# error: i_count is %d\n\n", i_count);
        return;
    };

    int istart = rvtfile->vertices_count - v_start_shift;

    if (rvtfile->file_format == RVT_FORMAT_OBJ) {
        istart += 1; // indices in PLY start from 0, indices in OBJ start from 1
    };


    for (int ii = 0; ii < i_count; ii+=3) {


        if (rvtfile->file_format == RVT_FORMAT_OBJ) {


            if (rvtfile->v_type == RVT_EXPORT_V_TYPE_12) {

                rvtfile->total_bytes += fprintf(rvtfile->fp, "f %d//%d %d//%d %d//%d\n",

                             istart + p_indices[ ii + 0],
                             istart + p_indices[ ii + 0],

                             istart + p_indices[ ii + 1],
                             istart + p_indices[ ii + 1],

                             istart + p_indices[ ii + 2],
                             istart + p_indices[ ii + 2]

                              );
            }
            else {


                rvtfile->total_bytes += fprintf(rvtfile->fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",

                             istart + p_indices[ ii + 0],
                             istart + p_indices[ ii + 0],
                             istart + p_indices[ ii + 0],

                             istart + p_indices[ ii + 1],
                             istart + p_indices[ ii + 1],
                             istart + p_indices[ ii + 1],

                             istart + p_indices[ ii + 2],
                             istart + p_indices[ ii + 2],
                             istart + p_indices[ ii + 2]

                              );
            }

        }


        else if (rvtfile->file_format == RVT_FORMAT_PLY_ASCII) {

            rvtfile->total_bytes += fprintf(rvtfile->fp_temp_i, "3 %d %d %d\n",
                         istart + p_indices[ ii + 0],
                         istart + p_indices[ ii + 1],
                         istart + p_indices[ ii + 2]
                          );
        };


        rvtfile->faces_count++;

    };



};

void rvt_export_file_write( rvt_export_file_t *rvtfile, float *p_vertices, int v_count, uint32_t *p_indices, int i_count ) {

    rvt_export_file_write_vertices(rvtfile, p_vertices, v_count );
    rvt_export_file_write_indices(rvtfile, p_indices, i_count, v_count );

}

