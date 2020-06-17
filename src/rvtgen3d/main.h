#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED


#include "rs/rsdebug.h"

#include "rs/rsthread.h"

#include "rs/rsgeom.h"
#include "rs/rsmx.h"

#include "rs/rsmem.h"

#include "rvtexport.h"
#include "rvttypes.h"


#ifndef RS_VBO_UNSIGNED_SHORT
    #define RS_VBO_UNSIGNED_SHORT 1
#endif

#ifndef RS_VBO_CONF_444C
    #define RS_VBO_CONF_444C    2
#endif

#ifndef RS_VBO_CONF_USER
    #define RS_VBO_CONF_USER    16
#endif


#define RVT_APP_VBO_CONF_20         0
#define RVT_APP_VBO_CONF_16         1
#define RVT_APP_VBO_CONF_BUILDING   2


// ----------


enum {
    VBODATA_TEMP = 0,
    VBODATA_TEMP_WIRE,
    VBODATA_RESERVED1,
    VBODATA_RESERVED2,


    VBODATA_BUILDING_ROOF_PART_1,
    VBODATA_BUILDING_ROOF_PART_2,
    VBODATA_BUILDING_CONSTRUCTION_PART,

    VBODATA_BUILDING_ENTRANCE_1,

    VBODATA_ROOF_SHAPE_DOME,
    VBODATA_ROOF_SHAPE_ONION,

    VBODATA_ROOF_RURAL_01,
    VBODATA_ROOF_RURAL_02,

    VBODATA_POWER_TOWER,
    VBODATA_POWER_TOWER_METAL,
    VBODATA_MINOR_TOWER,

    VBODATA_TREE1,
    VBODATA_TREEPACK1,
    VBODATA_BUSH1,

    VBODATA_BRIDGE_END1,
    VBODATA_BRIDGE_GARBAGE1,
    VBODATA_BRIDGE_BASE1,

    VBODATA_STREET_LAMP2,
    VBODATA_STREET_LAMP1_FOOTWAY,
    VBODATA_RAILWAY_TOWER,

    VBODATA_TRAFFIC_LIGHT,

    VBODATA_ROAD_SIGN_GIVE_WAY,
    VBODATA_ROAD_SIGN_STOP,

    VBODATA_HOUSE01,
    VBODATA_HOUSE02,
    VBODATA_HOUSE03,
    VBODATA_HOUSE04,

    RVT_APP_VBODATA_COUNT

};


// Colors

#define RVT_APP_COLOR_REC_STRING_LEN   28

typedef struct rs_color_rec_t {
    char s[RVT_APP_COLOR_REC_STRING_LEN];
    uint32_t hexcode;
} rs_color_rec_t;

#define RVT_APP_MAX_COLOR_RECS    1024


void gd_append(int tile_i, int layer_i, int hdr_len, unsigned char *content_header, int data_len, unsigned char *content_data);


#define RVT_APP_FLAG_FLAT_TERRAIN      0x01
#define RVT_APP_FLAG_DROP_NO_OUTER_RING    0x02
#define RVT_APP_FLAG_DISABLE_TIMESTAMP_FOLDERS  0x04

#define VISSTYLE_INDEX_DEFAULT      (1)
#define VISSTYLE_INDEX_WINTER       (2)


// Game Registry

typedef struct rvt_app_t {

    int flags;

    int termination_process;

    struct rvt_exporting_struct_t exporting_struct;

    int current_tile_ix;
    int current_tile_iy;
    int current_tile_i;
    int current_subtile_i;

    char user_lang[3]; // e.g. 'ru\0'
    char user_alt_lang[3];

    // RVT Constants
    int rvt_stride[RVT_SUBTILES_VBO_LAYERS_COUNT];
    int rvt_vbo_conf_index[RVT_SUBTILES_VBO_LAYERS_COUNT];

    // GeoData
    struct rvt_geodata_struct_t geodata;

    rvt_gpoint_t *highlighted_building_point;
    int highlighted_building_tile_i;

    rvt_gpoint_t *editor_building_point;
    int editor_building_tile_i;

    float sc;
    float sc_z;
    float sc_heightmap_z;


    // Color codes
    rs_color_rec_t color_recs[RVT_APP_MAX_COLOR_RECS];
    int color_recs_count;

    int tiles_are_ready; // Everything is loaded.
    int tiles_initial_loading_is_passed;
    int tiles_loader_progress;


    rvt_vbodata_t vbodata[RVT_APP_VBODATA_COUNT];

    int visstyle_index;

    rvt_cache_settings_t rvt_cache_settings;
    char output_dir[255];

    int rvt_style;


} rvt_app_t;


extern rvt_app_t *rvt_app;


void rvt_app_reg_init();

void GameProcess();


void rvtgen3d_init(int argc, char **argv);
void rvtgen3d_generate();
void rvtgen3d_term();

void rvt_app_destroy_tile(int tile_i);

void rvt_app_destroy_tile_geodata(int tile_i);
void rvt_app_destroy_subtile_gstuff(int subtile_i);

void rvt_app_clear_tile_cache();

void rvt_app_destroy_all_tiles();

void rvt_app_street_labels_update(int manually);


void rvt_app_export_process();


void rvt_app_test_export();

#endif // MAIN_H_INCLUDED
