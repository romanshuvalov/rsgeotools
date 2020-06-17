#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "loader.h"
#include "tileloader.h"

#include "main.h"

#include "rvt.h"
#include "rvtapp.h"
#include "rvtgen.h"
#include "rvtloader.h"


void loader_load_all_vbodata() {


    char fnbuf[256];

    loader_create_vbodata( VBODATA_BUILDING_ROOF_PART_1, "models/building-roof-part1.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BUILDING_ROOF_PART_2, "models/building-roof-part2.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BUILDING_CONSTRUCTION_PART, "models/building-construction-part.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BUILDING_ENTRANCE_1, "models/building-entrance-1.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_ROOF_SHAPE_DOME, "models/roof-shape-dome.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_ROOF_SHAPE_ONION, "models/roof-shape-onion.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_ROOF_RURAL_01, "models/roof-rural-01.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_ROOF_RURAL_02, "models/roof-rural-02.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_POWER_TOWER_METAL, "models/powertower1.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_POWER_TOWER, "models/powertower2.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_TREE1, "models/tree1.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_TREEPACK1, "models/treepack1.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BUSH1, "models/bush1.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_BRIDGE_END1, "models/bridge-end.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BRIDGE_GARBAGE1, "models/bridge-garbage.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_BRIDGE_BASE1, "models/bridge-base.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_HOUSE01, "models/house01.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_HOUSE02, "models/house02.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_HOUSE03, "models/house03.ply" , RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_HOUSE04, "models/house04.ply" , RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_STREET_LAMP2, "models/street-lamp-2.ply", RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_STREET_LAMP1_FOOTWAY, "models/street-lamp-footway.ply", RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_RAILWAY_TOWER, "models/railway-tower.ply", RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_TRAFFIC_LIGHT, "models/traffic-light.ply", RVT_APP_LOADER_FORMAT_PLY);

    loader_create_vbodata( VBODATA_ROAD_SIGN_GIVE_WAY, "models/road-sign-give-way.ply", RVT_APP_LOADER_FORMAT_PLY);
    loader_create_vbodata( VBODATA_ROAD_SIGN_STOP, "models/road-sign-stop.ply", RVT_APP_LOADER_FORMAT_PLY);


};



// Can be called in any thread, but it's not thread-safe
void loader_create_vbodata(int vbodata_index, char* filename, int format_type) {

    rvt_vbodata_t *pvbodata = &rvt_app->vbodata[vbodata_index];
    if (pvbodata->data != NULL) {
        rs_mem_free(pvbodata->data);
        pvbodata->data = NULL;
        pvbodata->vertices_count = 0;
    };

    if (format_type == RVT_APP_LOADER_FORMAT_PLY) {
        loader_create_vbodata_ply(vbodata_index, filename);
    }
    else {
        // unknown format_type
    };

};

#ifndef RS_VBO_MAX_VERTICES
    #define RS_VBO_MAX_VERTICES 65536
#endif


void loader_create_vbodata_ply(int vbodata_index, char* filename) {


    rvt_vbodata_t *pvbodata = &rvt_app->vbodata[vbodata_index];


    float *data = (float*) rs_mem_alloc(RS_VBO_MAX_VERTICES * (16) * 4, RS_MEM_POOL_AUTO); //  POS1 NORM1 UV00 RGBA
    rs_app_assert_memory( data, "", __LINE__ );
    float *data_ptr = data;

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Missing file %s.\nYou may want to set --data-dir parameter.\nAborted\n", filename);
        exit(-1);
    };

    rs_vec3_t *verts = (rs_vec3_t*) rs_mem_alloc( sizeof(rs_vec3_t) * RS_VBO_MAX_VERTICES, RS_MEM_POOL_AUTO );
    rs_vec3_t *norms = (rs_vec3_t*) rs_mem_alloc( sizeof(rs_vec3_t) * RS_VBO_MAX_VERTICES, RS_MEM_POOL_AUTO );
    rs_vec2_t *uvs = (rs_vec2_t*) rs_mem_alloc( sizeof(rs_vec2_t) * RS_VBO_MAX_VERTICES, RS_MEM_POOL_AUTO );
    rs_vec4_t *colors = (rs_vec4_t*) rs_mem_alloc( sizeof(rs_vec4_t) * RS_VBO_MAX_VERTICES, RS_MEM_POOL_AUTO );
    rs_vec4i_t *colors_int = (rs_vec4i_t*) rs_mem_alloc( sizeof(rs_vec4i_t) * RS_VBO_MAX_VERTICES, RS_MEM_POOL_AUTO );


    char s[256];
    int d[9];
    int c;

    int vertscount = 0;
    int facescount = 0;
    int output_vertices_count = 0;


    while (!feof(fp)) {

        if (!fgets(s, 256, fp)) {
            continue;
        };


        if (s[0] == '#') {
            continue;
        };

        if (isspace(s[0])) {
            continue;
        };

        if (s[0] == 0) {
            continue;
        };

        if (strlen(s) > 22) { // vertex data (hacky, but it works)

            c = sscanf(s, "%f %f %f %f %f %f %f %f %d %d %d", &verts[vertscount].x, &verts[vertscount].y, &verts[vertscount].z,
                   &norms[vertscount].x, &norms[vertscount].y, &norms[vertscount].z,
                   &uvs[vertscount].x, &uvs[vertscount].y,
                   &colors_int[vertscount].x, &colors_int[vertscount].y, &colors_int[vertscount].z
                    );

            if (c != 11) {
                continue;
            };

            colors[vertscount].x = (float) colors_int[vertscount].x / 255.0;
            colors[vertscount].y = (float) colors_int[vertscount].y / 255.0;
            colors[vertscount].z = (float) colors_int[vertscount].z / 255.0;
            colors[vertscount].w = 1.0;

            vertscount++;
            continue;
        }

        else {

            c = sscanf(s, "%d %d %d %d", &d[0], &d[1], &d[2], &d[3]);

            if (c != 4) {
                continue;
            };


            if (d[0] != 3) {
                rs_critical_alert_and_halt_sprintf("Model %s is not triangulated", filename );
                return;
            };

            int i;

            for (i = 1; i < 4; i++) {

                float *v = &data[output_vertices_count * 16];

                v[0] = verts[d[i]].x;
                v[1] = verts[d[i]].y;
                v[2] = verts[d[i]].z;
                v[3] = 1.0;

                v[4] = norms[d[i]].x;
                v[5] = norms[d[i]].y;
                v[6] = norms[d[i]].z;
                v[7] = 1.0;

                v[ 8] = uvs[d[i]].x;
                v[ 9] = uvs[d[i]].y;
                v[10] = 0.0;
                v[11] = 0.0;

                v[12] = colors[d[i]].x;
                v[13] = colors[d[i]].y;
                v[14] = colors[d[i]].z;
                v[15] = 1.0;

                output_vertices_count++;

            };

            facescount++;
            continue;
        };

    };


    int total_vertices = output_vertices_count;


    int data_len = 4 * 16 * total_vertices;
    pvbodata->data = (float*) rs_mem_alloc( data_len, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( pvbodata->data, "loader", __LINE__ );

    memcpy(pvbodata->data, data, data_len);

    pvbodata->stride = 16;
    pvbodata->vertices_count = total_vertices;

    rs_mem_free(colors_int);
    rs_mem_free(colors);
    rs_mem_free(uvs);
    rs_mem_free(norms);
    rs_mem_free(verts);

    rs_mem_free(data);


};



