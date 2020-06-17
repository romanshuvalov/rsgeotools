#include "main.h"


#include "rs/rsdebug.h"

#include <math.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h> // for mkdir

#include <stdlib.h>

#include "rvt.h"
#include "rvtapp.h"
#include "rvtutil.h"
#include "rvtgen.h"
#include "rvtloader.h"

#include "loader.h"

#include "tileloader.h"



#include <zlib.h>


rvt_app_t *rvt_app;

void rvt_app_reg_init() {


    rvt_app = (rvt_app_t*) malloc( sizeof(struct rvt_app_t) );
    rs_app_assert_memory( rvt_app, "", __LINE__ );

    memset (rvt_app, 0, sizeof(struct rvt_app_t) );

    rvt_app->sc_z = 1.0 / 4.0;

};




void rvtgen3d_generate() {

    printf("Generating 3D models:\n");

    rvt_app_test_export();

    while (rvt_app->exporting_struct.exporting)  {

        rvt_app_export_process();

        if (rvt_tile_loader_reg.status == RVT_TILE_LOADER_STATUS_LOADING) {
            rvt_tile_loader_check_mainthread();
        };

    };

    printf("Done.\n");

};




void rvtgen3d_init(int argc, char **argv) {



    rs_mem_init();

    rs_mem_pool_init( RVT_MEM_POOL_GD, RVT_GD_MAX_ARRAY_LEN, 4, 1024 );
    rs_mem_pool_init( RVT_MEM_POOL_GPOINTS_ARRAY, RVT_GP_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gpoint_t), 256, 1024 );
    rs_mem_pool_init( RVT_MEM_POOL_GLINES_ARRAY, RVT_GLINE_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gline_t), 256, 1024 );
    rs_mem_pool_init( RVT_MEM_POOL_GTRIANGLES_ARRAY, RVT_GTRIANGLE_MAX_ARRAY_ITEMS_COUNT * sizeof(rvt_gtriangle_t), 256, 1024 );


    rvt_app_reg_init();

    rvt_settings_set_to_defaults();

//    rvt_app_cache_folders_init();


    rvt_gen_init_default_stages();

    int param_x = -1;
    int param_y = -1;
    int param_w = -1;
    int param_h = -1;


    for (int i = 1; i < argc; i++) {

        char *s = argv[i];
        if ( !strcmp(s, "--flat-terrain") ) {
            DEBUG10f("Flat terrain enabled.\n");
            rvt_app->flags |= RVT_APP_FLAG_FLAT_TERRAIN;
        }


        else if ( strstr(s, "--x=") == s ) {
            sscanf(s+4, "%d", &param_x);
        }
        else if ( strstr(s, "--y=") == s ) {
            sscanf(s+4, "%d", &param_y);
        }
        else if ( strstr(s, "--w=") == s ) {
            sscanf(s+4, "%d", &param_w);
        }
        else if ( strstr(s, "--h=") == s ) {
            sscanf(s+4, "%d", &param_h);
        }

        else if ( strstr(s, "--cache-dir=") == s ) {

            sprintf(rvt_app_get_cache_settings()->cache_folders[ rvt_app_get_cache_settings()->cache_folders_count ], "%s", s+12);

            rvt_app_get_cache_settings()->cache_folders_count++;

        }

        else if ( strstr(s, "--data-dir=") == s ) {

            chdir(s+11);

        }

        else if ( strstr(s, "--output-dir=") == s ) {

            sprintf(rvt_app->output_dir, "%s", s+13);

        }

        else if ( !strcmp(s, "--z-up") ) {
            rvt_settings.z_up = 1;
        }
        else if ( !strcmp(s, "--merge") ) {
            rvt_settings.separate_to_tiles = 0;
        }
        else if ( !strcmp(s, "--disable-edge-smoothing") ) {
            rvt_settings.smooth_road_edges = 0;
            rvt_settings.smooth_river_edges = 0;
            rvt_settings.smooth_area_edges = 0;
        }

        else if ( !strcmp(s, "--obj") ) {
            rvt_settings.file_format = RVT_FORMAT_OBJ;
        }

        else if ( !strcmp(s, "--disable-decorations") ) {
            rvt_settings.building_decorations = 0;
        }

        else if ( !strcmp(s, "--drop-no-outer-ring") ) {
            rvt_app->flags |= RVT_APP_FLAG_DROP_NO_OUTER_RING;
        }

        else if ( !strcmp(s, "--disable-timestamp-folders") ) {
            rvt_app->flags |= RVT_APP_FLAG_DISABLE_TIMESTAMP_FOLDERS;
        };

    };

    if ( (param_x == -1) || (param_y == -1) || (param_w == -1) || (param_h == -1) || (rvt_app_get_cache_settings()->cache_folders_count == 0) ) {

        printf("Usage: rvtgen3d <parameters>\n"
               "  Required parameters:\n"
               "    --x=<top>, --y=<left>, --w=<width>, --h=<height> - rectangle in tile coordinates at 14th scale\n"
               "    --cache-dir=<path> - path to gpak cache, without tailing slash. When multiple parameters passed, first one has higher priority"
               "  Optional parameters:\n"
               "    --data-dir=<path> - path to folder containing color_codes.txt and set of models"
               "    --output-dir=<path> - path to output directory"
               "    --disable-timestamp-folders - by default, separated output folder with unique name will be created. This option disables this feature"
               "    --flat-terrain - disable relief\n"
               "    --z-up - define Z axis as vertical. Default is Y\n"
               "    --merge - merge all tiles into one file. Not recommended for large areas\n"
               "    --disable-edge-smoothing\n"
               "    --obj - set output format to .obj instead of .ply\n"
               "    --disable-decorations - disable building decorations\n"
               "    --drop-no-outer-ring - ignore broken multipolygons which have no outer ring\n"
               );

        rvtgen3d_term();
        exit(-1);
    };

    rvt_app_get_geodata()->start_tile_world_ix = param_x; //10442;
    rvt_app_get_geodata()->start_tile_world_iy = param_y; //5300;

    rvt_app_get_exporting_struct()->exporting_xi_count = param_w; //1;
    rvt_app_get_exporting_struct()->exporting_yi_count = param_h; // 2;

    if (rvt_app->output_dir[0] == '\0') {
        rvt_app->output_dir[0] = '.';
    };


    rs_geom_flags = (rvt_app->flags & RVT_APP_FLAG_DROP_NO_OUTER_RING) ? RS_GEOM_FLAG_DROP_NO_OUTER_RING : 0;

    strcpy(rvt_app->user_lang, "en");
    strcpy(rvt_app->user_alt_lang, "en");


    DEBUG10("--- RVT Constants ---");

    rvt_app->rvt_stride[RVT_LAYER_AREA] = 12;
    rvt_app->rvt_stride[RVT_LAYER_BUILDING] = 16;
    rvt_app->rvt_stride[RVT_LAYER_ROAD] = 12;
    rvt_app->rvt_stride[RVT_LAYER_NATURAL] = 16;
    rvt_app->rvt_stride[RVT_LAYER_PROP] = 16;
    rvt_app->rvt_stride[RVT_LAYER_WIRE] = 12;
    rvt_app->rvt_stride[RVT_LAYER_STRIPES] = 16;
    rvt_app->rvt_stride[RVT_LAYER_WALLS] = 16;

    rvt_app->rvt_vbo_conf_index[RVT_LAYER_AREA] = RS_VBO_CONF_444C;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_BUILDING] = RS_VBO_CONF_USER + RVT_APP_VBO_CONF_BUILDING;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_ROAD] = RS_VBO_CONF_444C;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_NATURAL] = RS_VBO_CONF_USER + RVT_APP_VBO_CONF_16;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_PROP] = RS_VBO_CONF_USER + RVT_APP_VBO_CONF_16;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_WIRE] = RS_VBO_CONF_444C;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_STRIPES] = RS_VBO_CONF_USER + RVT_APP_VBO_CONF_16;
    rvt_app->rvt_vbo_conf_index[RVT_LAYER_WALLS] = RS_VBO_CONF_USER + RVT_APP_VBO_CONF_16;


    rvt_tile_loader_init();

    loader_load_all_vbodata();


};

void rvt_app_destroy_gdata() {

    // Physically free gdata and g*

    for (int t_i = 0; t_i < RVT_TILES_TOTAL_COUNT; t_i++) {
        for (int layer_i = 0; layer_i < RVT_GD_MAX_LAYERS_COUNT; layer_i++) {
            for (int l_i = 0; l_i < RVT_GD_MAX_ARRAYS_COUNT; l_i++) {
                if (rvt_app->geodata.gd_data[t_i][layer_i][l_i] != NULL) {
                    rs_mem_free( rvt_app->geodata.gd_data[t_i][layer_i][l_i] );
                };
            };
        };
    };


    for (int subtile_i = 0; subtile_i < RVT_SUBTILES_TOTAL_COUNT; subtile_i++) {

        for (int current_gp_layer = 0; current_gp_layer < RVT_GP_MAX_LAYERS_COUNT; current_gp_layer++) {
            for (int li = 0; li < RVT_GP_MAX_ARRAYS_COUNT; li++) {
                if (rvt_app->geodata.gp[subtile_i][current_gp_layer][li]) {
                    rs_mem_free(rvt_app->geodata.gp[subtile_i][current_gp_layer][li]);
                    rvt_app->geodata.gp[subtile_i][current_gp_layer][li] = NULL;
                };
                rvt_app->geodata.gp_items_count[subtile_i][current_gp_layer][li] = 0;
            };
            rvt_app->geodata.gp_list_count[subtile_i][current_gp_layer] = 0;
        };

        for (int current_gline_layer = 0; current_gline_layer < RVT_GLINE_MAX_LAYERS_COUNT; current_gline_layer++) {
            for (int li = 0; li < RVT_GLINE_MAX_ARRAYS_COUNT; li++) {
                if (rvt_app->geodata.gline[subtile_i][current_gline_layer][li]) {
                    rs_mem_free(rvt_app->geodata.gline[subtile_i][current_gline_layer][li]);
                    rvt_app->geodata.gline[subtile_i][current_gline_layer][li] = NULL;
                };
                rvt_app->geodata.gline_items_count[subtile_i][current_gline_layer][li] = 0;
            };
            rvt_app->geodata.gline_list_count[subtile_i][current_gline_layer] = 0;
        };

        for (int current_gtriangle_layer = 0; current_gtriangle_layer < RVT_GTRIANGLE_MAX_LAYERS_COUNT; current_gtriangle_layer++) {
            for (int li = 0; li < RVT_GTRIANGLE_MAX_ARRAYS_COUNT; li++) {
                if (rvt_app->geodata.gtriangle[subtile_i][current_gtriangle_layer][li]) {
                    rs_mem_free(rvt_app->geodata.gtriangle[subtile_i][current_gtriangle_layer][li]);
                    rvt_app->geodata.gtriangle[subtile_i][current_gtriangle_layer][li] = NULL;
                };
                rvt_app->geodata.gtriangle_items_count[subtile_i][current_gtriangle_layer][li] = 0;
            };
            rvt_app->geodata.gtriangle_list_count[subtile_i][current_gtriangle_layer] = 0;
        };


    };


};


void rvtgen3d_term() {


    rvt_app->termination_process = 1;



    while (rvt_tile_loader_reg.status != RVT_TILE_LOADER_STATUS_OFF) {
        rvt_tile_loader_check_mainthread();
        rvt_app_sleep_msec(10);
    };


    for (int i = 0; i < RVT_SUBPAK_CACHE_COUNT; i++) {
        if (rvt_app->geodata.subpak_cache[i].data) {
            rs_mem_free(rvt_app->geodata.subpak_cache[i].data);
        };
    };






    for (int i = 0; i < RVT_APP_VBODATA_COUNT; i++) {
        if (rvt_app->vbodata[i].data != NULL) {
            rs_mem_free(rvt_app->vbodata[i].data);
            rvt_app->vbodata[i].data = NULL;
        };
    };



    rvt_app_destroy_all_tiles();

    free(rvt_app);


    DEBUG20("GameTerm Done.");
};



void rvt_app_destroy_tile_geodata(int tile_i) {

    // Make gdata look like clean

    for (int current_gd_layer = 0; current_gd_layer < RVT_GD_MAX_LAYERS_COUNT; current_gd_layer++) {
        for (int a = 0; a < rvt_app->geodata.gd_list_count[tile_i][current_gd_layer]; a++) {
            unsigned char *p = rvt_app->geodata.gd_data[tile_i][current_gd_layer][a];
            for (int i = 0; i < rvt_app->geodata.gd_items_count[tile_i][current_gd_layer][a]; i++ ) {

                if (current_gd_layer == RVT_GD_LAYER_BUILDING) {
                    gd_building_data_t *data = (gd_building_data_t*) p;
                    if (data->name) {
                        rs_mem_free(data->name);
                        data->name = NULL;
                    };
                    if (data->name_user_lang) {
                        rs_mem_free(data->name_user_lang);
                        data->name_user_lang = NULL;
                    };
                    if (data->name_user_alt_lang) {
                        rs_mem_free(data->name_user_alt_lang);
                        data->name_user_alt_lang = NULL;
                    };
                    if (data->housenumber) {
                        rs_mem_free(data->housenumber);
                        data->housenumber = NULL;
                    };
                    for (int bi = 0; bi < RVT_BUILDING_MAX_STRINGS; bi++) {
                        if (data->building_strings[bi]) {
                            rs_mem_free(data->building_strings[bi]);
                            data->building_strings[bi] = NULL;
                        };
                    };
                    p += sizeof(gd_building_data_t);
                    p += data->content_len;
                }
                else if (current_gd_layer == RVT_GD_LAYER_AREA) {
                    gd_area_data_t *data = (gd_area_data_t*) p;
                    if (data->name) {
                        rs_mem_free(data->name);
                        data->name = NULL;
                    };
                    if (data->name_user_lang) {
                        rs_mem_free(data->name_user_lang);
                        data->name_user_lang = NULL;
                    };
                    if (data->name_user_alt_lang) {
                        rs_mem_free(data->name_user_alt_lang);
                        data->name_user_alt_lang = NULL;
                    };
                    p += sizeof(gd_area_data_t);
                    p += data->content_len;
                } else if (current_gd_layer == RVT_GD_LAYER_LINES) {
                    gd_lines_data_t *data = (gd_lines_data_t*) p;
                    if (data->name) {
                        rs_mem_free(data->name);
                        data->name = NULL;
                    };
                    if (data->name_user_lang) {
                        rs_mem_free(data->name_user_lang);
                        data->name_user_lang = NULL;
                    };
                    if (data->name_user_alt_lang) {
                        rs_mem_free(data->name_user_alt_lang);
                        data->name_user_alt_lang = NULL;
                    };
                    p += sizeof(gd_lines_data_t);
                    p += data->content_len;
                } else {    // points
                    gd_point_data_t *data = (gd_point_data_t*) p;
                    if (data->name) {
                        rs_mem_free(data->name);
                        data->name = NULL;
                    };
                    if (data->name_user_lang) {
                        rs_mem_free(data->name_user_lang);
                        data->name_user_lang = NULL;
                    };
                    if (data->name_user_alt_lang) {
                        rs_mem_free(data->name_user_alt_lang);
                        data->name_user_alt_lang = NULL;
                    };
                    p += sizeof(gd_point_data_t);
                    p += data->content_len;
                };
            };
        };
        rvt_app->geodata.gd_list_count[tile_i][current_gd_layer] = 0;
    };


};

void rvt_app_destroy_all_tiles() {
    for (int tile_i = 0; tile_i < RVT_TILES_TOTAL_COUNT; tile_i++) {
        rvt_app_destroy_tile(tile_i);
    };
};

void rvt_app_destroy_subtile_gstuff(int subtile_i) {

    // Make gdata look like clean

    for (int current_gp_layer = 0; current_gp_layer < RVT_GP_MAX_LAYERS_COUNT; current_gp_layer++) {
        for (int li = 0; li < rvt_app->geodata.gp_list_count[subtile_i][current_gp_layer]; li++) {
            rvt_app->geodata.gp_items_count[subtile_i][current_gp_layer][li] = 0;

        };
        rvt_app->geodata.gp_list_count[subtile_i][current_gp_layer] = 0;
    };


    for (int current_gline_layer = 0; current_gline_layer < RVT_GLINE_MAX_LAYERS_COUNT; current_gline_layer++) {
        for (int li = 0; li < rvt_app->geodata.gline_list_count[subtile_i][current_gline_layer]; li++) {

            rvt_app->geodata.gline_items_count[subtile_i][current_gline_layer][li] = 0;

        };
        rvt_app->geodata.gline_list_count[subtile_i][current_gline_layer] = 0;
    };

    for (int current_gtriangle_layer = 0; current_gtriangle_layer < RVT_GTRIANGLE_MAX_LAYERS_COUNT; current_gtriangle_layer++) {
        for (int li = 0; li < rvt_app->geodata.gtriangle_list_count[subtile_i][current_gtriangle_layer]; li++) {
            rvt_app->geodata.gtriangle_items_count[subtile_i][current_gtriangle_layer][li] = 0;

        };
        rvt_app->geodata.gtriangle_list_count[subtile_i][current_gtriangle_layer] = 0;
    };

};


void gd_append(int tile_i, int layer_i, int hdr_len, unsigned char *content_header, int data_len, unsigned char *content_data) {


    int ar_i = rvt_app->geodata.gd_list_count[tile_i][layer_i] - 1;

    if (ar_i == -1) {
        rvt_app->geodata.gd_list_count[tile_i][layer_i] = 1;
        ar_i = 0;
        rvt_app->geodata.gd_items_len[tile_i][layer_i][ar_i] = 0;
        rvt_app->geodata.gd_items_count[tile_i][layer_i][ar_i] = 0;
    };

    int ar_p = rvt_app->geodata.gd_items_len[tile_i][layer_i][ar_i];

    if (ar_p + hdr_len + data_len > RVT_GD_MAX_ARRAY_LEN) {
        ar_i = rvt_app->geodata.gd_list_count[tile_i][layer_i]++;
        if (ar_i >= RVT_GD_MAX_ARRAYS_COUNT) {
            rs_critical_alert_and_halt("Max GD arrays count is reached. ");
        };
        rvt_app->geodata.gd_items_len[tile_i][layer_i][ar_i] = 0;
        ar_p = 0;
        rvt_app->geodata.gd_items_count[tile_i][layer_i][ar_i] = 0;
    };


    if ( rvt_app->geodata.gd_data[tile_i][layer_i][ar_i] == NULL ) {
        rvt_app->geodata.gd_data[tile_i][layer_i][ar_i] = rs_mem_alloc( RVT_GD_MAX_ARRAY_LEN, RVT_MEM_POOL_GD );
        ar_p = 0;
    };

    unsigned char *p = rvt_app->geodata.gd_data[tile_i][layer_i][ar_i] + ar_p;



    memcpy( p, content_header, hdr_len );
    memcpy( p + hdr_len, content_data, data_len);
    rvt_app->geodata.gd_items_count[tile_i][layer_i][ar_i]++;
    rvt_app->geodata.gd_items_len[tile_i][layer_i][ar_i] += hdr_len + data_len;

};




void rvt_app_destroy_tile(int tile_i) {


    int tile_ix = tile_i % RVT_TILES_SIDE_COUNT;
    int tile_iy = tile_i / RVT_TILES_SIDE_COUNT;

    if (rvt_tile_loader_reg.status == RVT_TILE_LOADER_STATUS_LOADING) {
        rvt_tile_loader_halt();
    };

    if (rvt_app_get_geodata()->tile[tile_i].status) {

        rvt_app_get_geodata()->tile[tile_i].status = RVT_TILE_STATUS_NONE;


        rvt_app_destroy_tile_geodata( tile_i );

        rvt_app->geodata.gd_list_count[tile_i][RVT_GD_LAYER_BUILDING] = 0;
        rvt_app->geodata.gd_list_count[tile_i][RVT_GD_LAYER_AREA] = 0;
        rvt_app->geodata.gd_list_count[tile_i][RVT_GD_LAYER_LINES] = 0;
        rvt_app->geodata.gd_list_count[tile_i][RVT_GD_LAYER_POINTS] = 0;
        // lists are freed once by GameTerm() by checking for NULL

    };


    for (int i = 0; i < RVT_SUBTILES_SIDE_COUNT_PER_TILE; i++) {
        for (int j = 0; j < RVT_SUBTILES_SIDE_COUNT_PER_TILE; j++) {
            int si_x = RVT_SUBTILES_SIDE_COUNT_PER_TILE*tile_ix + j;
            int si_z = RVT_SUBTILES_SIDE_COUNT_PER_TILE*tile_iy + i;
            int subtile_i = si_z*RVT_SUBTILES_SIDE_COUNT + si_x;

            rvt_app_destroy_subtile_gstuff( subtile_i );

            rvt_app->geodata.gflags[subtile_i] = 0;

        };
    };


    rvt_app_get_geodata()->tile[tile_i].flags = 0;

    if (rvt_app_get_geodata()->hm[tile_i].status) {
        rvt_app_get_geodata()->hm[tile_i].status = 0;
        rvt_app_get_geodata()->hm[tile_i].phys_shape = NULL;
        rvt_app_get_geodata()->hm[tile_i].phys_body = NULL;
    };


};




void rvt_app_color_recs_init() {

    char fn_buf[256];

    char buf[64];
    char color_name[1024];
    int color_code = 7;

    FILE *fp = fopen( "color_codes.txt", "r" );

    if (fp) {

        while (!feof(fp)) {
            fgets(buf, 64, fp);

            if (buf[0] == '#') {
                continue;
            };

            int res = sscanf(buf, "%s %x", color_name, &color_code );

            rvt_app->color_recs[rvt_app->color_recs_count].hexcode = color_code;


            strncpy(rvt_app->color_recs[rvt_app->color_recs_count].s, color_name, RVT_APP_COLOR_REC_STRING_LEN-1);
            rvt_app->color_recs[rvt_app->color_recs_count].s[RVT_APP_COLOR_REC_STRING_LEN-1] = '\0';

            if (res == 2) {
                rvt_app->color_recs_count++;
            };

            if (rvt_app->color_recs_count > RVT_APP_MAX_COLOR_RECS-1) {
                break;
            };

        };

        fclose(fp);

    }
    else {
        printf("Warning, file color_codes.txt is missing.");
    }

};



void rvt_app_export_process() {

    if (!rvt_app_get_exporting_struct()->exporting) {
        return;
    };

    if (rvt_tile_loader_reg.status == RVT_TILE_LOADER_STATUS_LOADING) {
        return; // loader is busy
    };




    rs_vec2_t latlon = rs_geoconv_tc2degrees(14, rvt_app_get_geodata()->start_tile_world_ix, rvt_app_get_geodata()->start_tile_world_iy);

    rvt_app->sc = rvt_calc_sc( latlon );

    int tile_x = (int) 0;
    int tile_y = (int) 0;


    int tile_ix = rs_cyclic_mod_i(tile_x, RVT_TILES_SIDE_COUNT);
    int tile_iy = rs_cyclic_mod_i(tile_y, RVT_TILES_SIDE_COUNT);

    rvt_app->current_tile_ix = tile_ix;
    rvt_app->current_tile_iy = tile_iy;
    rvt_app->current_tile_i = tile_iy*RVT_TILES_SIDE_COUNT + tile_ix;

    rvt_app->current_subtile_i = 0;



    if (!rvt_app_get_exporting_struct()->exporting_abort) {

        if ( rvt_app_get_exporting_struct()->exporting_yi < rvt_app_get_exporting_struct()->exporting_yi_count ) {

            int st_ix = rs_cyclic_mod_i(rvt_app_get_exporting_struct()->exporting_xi, RVT_TILES_SIDE_COUNT);
            int st_iy = rs_cyclic_mod_i(rvt_app_get_exporting_struct()->exporting_yi, RVT_TILES_SIDE_COUNT);
            int tile_i = st_ix + RVT_TILES_SIDE_COUNT*st_iy;


            tile_ix = st_ix;
            tile_iy = st_iy;

            int st_x = tile_x + rvt_app_get_exporting_struct()->exporting_xi;
            int st_y = tile_y + rvt_app_get_exporting_struct()->exporting_yi;


            if ( (!rvt_app_get_geodata()->tile[tile_i].status)
                || (  (rvt_app_get_geodata()->tile[tile_i].tile_x != st_x) || (rvt_app_get_geodata()->tile[tile_i].tile_y != st_y) )  )  {

                if (rvt_app_get_exporting_struct()->exporting_xi_count > 4) {
                    if (rvt_app_get_exporting_struct()->exporting_xi == 0) {
                        rvt_app_destroy_all_tiles(); // Previous stripe of tiles must not affect new stripe
                    };
                };

//                #error make usage



                int mbytes10 = (int) (rvt_app_get_exporting_struct()->exporting_total_bytes / 1024 * 10 / 1024);
                if (mbytes10 > 0) {
                    float mbytes = mbytes10 / 10.0;

                    if (mbytes > 1024.0) {
                        printf("%.1f GB written.\n", mbytes/1024.0 );
                    }
                    else {
                        printf("%.1f MB written.\n", mbytes);
                    };
                };

                printf("Generating %d of %d... \n", 1+(rvt_app_get_exporting_struct()->exporting_yi*rvt_app_get_exporting_struct()->exporting_xi_count+rvt_app_get_exporting_struct()->exporting_xi),
                        rvt_app_get_exporting_struct()->exporting_xi_count*rvt_app_get_exporting_struct()->exporting_yi_count );



                rvt_app_destroy_tile(tile_i);


                rvt_app_get_geodata()->tile[tile_i].status = RVT_TILE_STATUS_LOADING;
                rvt_app_get_geodata()->tile[tile_i].tile_x = st_x;
                rvt_app_get_geodata()->tile[tile_i].tile_y = st_y;

                int tile_world_ix = st_x + rvt_app_get_geodata()->start_tile_world_ix;
                int tile_world_iy = st_y + rvt_app_get_geodata()->start_tile_world_iy;

                DEBUG10f("\n\nExporter:\n%d %d\n\n", tile_world_ix, tile_world_iy );

                rvt_tile_loader_load_tile(14, tile_world_ix, tile_world_iy, tile_i, st_x, st_y);

                rvt_app_get_exporting_struct()->exporting_xi++;
                if (rvt_app_get_exporting_struct()->exporting_xi == rvt_app_get_exporting_struct()->exporting_xi_count) {
                    rvt_app_get_exporting_struct()->exporting_xi = 0;
                    rvt_app_get_exporting_struct()->exporting_yi++;
                };


                return; // loader is now busy. return.

            }

        };

    };


    if (!rvt_settings.separate_to_tiles) {
        for (int i = 0; i < RVT_SUBTILES_VBO_LAYERS_COUNT+1; i++) { // heightmap is 0, geodata starts from 1
            if (rvt_settings.rvt_file_arr[i].fp != NULL) {

                DEBUG10f("Exporting: file term %d...\n", i);
                rvt_export_file_term(&rvt_settings.rvt_file_arr[i]);

            };
        };
    };


    rvt_app_get_exporting_struct()->exporting = 0;

    rvt_app_get_exporting_struct()->exporting_abort = 0;


    int mbytes10 = (int) (rvt_app_get_exporting_struct()->exporting_total_bytes / 1024 * 10 / 1024);
    float mbytes = mbytes10 / 10.0;

    if (mbytes > 1024.0) {
        printf("%.1f GB total.\n", mbytes/1024.0 );
    }
    else {
        printf("%.1f MB total.\n", mbytes);
    };

};




void rvt_app_test_export() {


    if (rvt_app->flags & RVT_APP_FLAG_DISABLE_TIMESTAMP_FOLDERS) {
        sprintf( rvt_settings.folder_name, "%s", rvt_app->output_dir);
    }
    else {
        int y, m, d, h, i, s;
        rvt_app_get_date( &y, &m, &d, &h, &i, &s );
        sprintf( rvt_settings.folder_name, "%s/%04d%02d%02d-%02d%02d%02d", rvt_app->output_dir, y, m, d, h, i, s);
    };

    rvt_app_get_exporting_struct()->exporting_total_bytes = 0;
    rvt_app_get_exporting_struct()->exporting_total_vertices = 0;

    mkdir(rvt_settings.folder_name, 0777);

    for (int i = 0; i < 9; i++) {
        rvt_settings.layer_enabled[i] = 1;
    };

    rvt_app_destroy_all_tiles();
    rvt_app->tiles_are_ready = 0;

    rvt_app_get_exporting_struct()->exporting_xi = 0;
    rvt_app_get_exporting_struct()->exporting_yi = 0;

    rvt_app_get_exporting_struct()->exporting = 1;


};


int main(int argc, char** argv) {

    rvtgen3d_init(argc, argv);

    rvtgen3d_generate();

    rvtgen3d_term();

    return 0;

};

