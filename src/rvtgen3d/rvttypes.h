#ifndef RVTTYPES_H
#define RVTTYPES_H

#include <inttypes.h>
#include <stdio.h>

#include "rs/rsmem.h"
#include "rs/rsgeom.h"

#define RVT_SC_HEIGHTMAP_Z  (0.5)


enum {
    RVT_MEM_POOL_GD = RS_MEM_RESERVED_POOLS,
    RVT_MEM_POOL_GPOINTS_ARRAY,
    RVT_MEM_POOL_GLINES_ARRAY,
    RVT_MEM_POOL_GTRIANGLES_ARRAY,
};


typedef struct rvt_vbodata_t {

    unsigned int vertices_count;
    unsigned int flags;
    unsigned int stride;
    unsigned int reserved;

    float *data;

} rvt_vbodata_t;


#define RVT_IGROUPS_COUNT  16

#define RVT_IGROUP_ANGLES_COUNT        12

#define RVT_IGROUP_DEFAULT         0
#define RVT_IGROUP_DETAILS         1
#define RVT_IGROUP_DETAILS_GENERALIZED     2
#define RVT_IGROUP_ALT             3
#define RVT_IGROUP_ANGLE0          4



#define RVT_HEIGHTMAP_TILE_SIDE_SIZE   33 // Must be 33 (32 quads + 1)

typedef struct rvt_heightmap_data_t {

    float data[ RVT_HEIGHTMAP_TILE_SIDE_SIZE*RVT_HEIGHTMAP_TILE_SIDE_SIZE ];

    int status;
    void *phys_shape;
    void *phys_body;

} rvt_heightmap_data_t;



#define RVT_TYPE_HEIGHTMAP      0
#define RVT_TYPE_GEODATA        1
#define RVT_TYPE_GENERALIZED_WORLD     2



typedef struct rvt_export_file_t {

    FILE *fp;

    FILE *fp_temp_v;
    FILE *fp_temp_i;

    char filename_temp_v[200];
    char filename_temp_i[200];

    uint64_t vertices_count;
    uint64_t faces_count;

    int file_format;

    uint64_t total_bytes;

    int v_type;

    int z_up;

    float scale;

} rvt_export_file_t;

#define RVT_FORMAT_PLY_ASCII  (0)
#define RVT_FORMAT_OBJ  (1)


enum {
    RVT_LAYER_BUILDING = 0,
    RVT_LAYER_AREA,
    RVT_LAYER_ROAD,
    RVT_LAYER_NATURAL,
    RVT_LAYER_PROP,
    RVT_LAYER_WIRE,
    RVT_LAYER_STRIPES,
    RVT_LAYER_WALLS,

    RVT_SUBTILES_VBO_LAYERS_COUNT
};


#define RVT_BUILDING_MAX_STRINGS   16

typedef struct gd_general_data_t {
    int content_len;
    int data_type;
    int gen_flags;
} gd_general_data_t;

typedef struct gd_building_data_t {
    int content_len;
    int data_type;
    int gen_flags;

    float cx;
    float cy;
    float base_height;
    int b_levels;

    float b_height;
    float b_min_height;

    int building;
    int building_part;

    int aeroway;

    float roof_height;

    int rvt_relation_role;

    int building_colour;
    int roof_colour;

    int roof_shape;

    float area;

    char *name;
    char *name_user_lang;
    char *name_user_alt_lang;

    char *housenumber;

    char *building_strings[RVT_BUILDING_MAX_STRINGS];


} gd_building_data_t;

typedef struct gd_lines_data_t {
    int content_len;
    int data_type;
    int gen_flags;

    float base_height;

    int hw;
    int railway;
    int waterway;
    int aeroway;
    int power;
    int lanes;
    int lit;
    int barrier;

    int oneway;
    int surface;

    int bridge;
    int tunnel;

    float width;

    char *name;
    char *name_user_lang;
    char *name_user_alt_lang;

} gd_lines_data_t;

typedef struct gd_area_data_t {
    int content_len;
    int data_type;
    int gen_flags;

    float cx;
    float cy;

    int natural;
    int landuse;
    int amenity;
    int leisure;
    int waterway;
    int aeroway;
    int water;
    int hw;

    int surface;

    int barrier;

    int residential;

    char *name;
    char *name_user_lang;
    char *name_user_alt_lang;

} gd_area_data_t;

typedef struct gd_point_data_t {
    int content_len;
    int data_type;

    int power;
    int hw;
    int railway;
    int entrance;
    int amenity;
    int leisure;

    int crossing;
    int direction;

    int natural;
    int barrier;

    int place;

    char *name;
    char *name_user_lang;
    char *name_user_alt_lang;

} gd_point_data_t;


enum {
    RVT_GD_LAYER_BUILDING = 0,
    RVT_GD_LAYER_AREA,
    RVT_GD_LAYER_LINES,
    RVT_GD_LAYER_POINTS,
    RVT_GD_MAX_LAYERS_COUNT
};



#define RVT_TILES_SIDE_COUNT   5 // should be 5, minimum 4, max 8 (or increase vbo count)
#define RVT_TILES_TOTAL_COUNT  (RVT_TILES_SIDE_COUNT*RVT_TILES_SIDE_COUNT)

#define RVT_SUBTILES_SIDE_COUNT_PER_TILE   6
#define RVT_SUBTILES_COUNT_PER_TILE    (RVT_SUBTILES_SIDE_COUNT_PER_TILE*RVT_SUBTILES_SIDE_COUNT_PER_TILE)

// Warning, total subtiles side count must be higher than 2*visible_radius when drawing
#define RVT_SUBTILES_SIDE_COUNT    (RVT_SUBTILES_SIDE_COUNT_PER_TILE*RVT_TILES_SIDE_COUNT)
#define RVT_SUBTILES_TOTAL_COUNT   (RVT_SUBTILES_SIDE_COUNT*RVT_SUBTILES_SIDE_COUNT)


#define RVT_GD_MAX_ARRAYS_COUNT    32
#define RVT_GD_MAX_ARRAY_LEN       (1024*1024)

#define RVT_GP_MAX_ARRAYS_COUNT    64
#define RVT_GP_MAX_ARRAY_ITEMS_COUNT  256

#define RVT_GLINE_MAX_ARRAYS_COUNT    64
#define RVT_GLINE_MAX_ARRAY_ITEMS_COUNT  256

#define RVT_GTRIANGLE_MAX_ARRAYS_COUNT    128
#define RVT_GTRIANGLE_MAX_ARRAY_ITEMS_COUNT  512



// ------------ internal geopoints -----------

typedef struct rvt_gpoint_t {

    rs_point_t pos;
    float azimuth;
    float radius;
    int flags;

    void *pdata;

} rvt_gpoint_t;

#define RVT_GP_FLAG_DATA_POINT     0x01
#define RVT_GP_FLAG_DATA_LINE      0x02
#define RVT_GP_FLAG_DATA_AREA      0x04
#define RVT_GP_FLAG_DATA_BUILDING  0x08

#define RVT_GP_FLAG_BUILDING_PHYS_LOADED   0x10
#define RVT_GP_FLAG_GENERATED_BUILDING     0x20
#define RVT_GP_FLAG_LARGE_BUILDING         0x40

#define RVT_GP_FLAG_NODATA                 0x80

#define RVT_GP_FLAG_ANY    0xFFFF
#define RVT_GP_FLAG_NONE   0x0000


enum {

    RVT_GP_LAYER_ROOF_PART1,
    RVT_GP_LAYER_STREET_LAMP,
    RVT_GP_LAYER_BUILDING,
    RVT_GP_LAYER_BUILDING_ENTRANCE,
    RVT_GP_LAYER_POWER_TOWER,
    RVT_GP_LAYER_BUS_STOP,

    RVT_GP_LAYER_VP,

    RVT_GP_MAX_LAYERS_COUNT
};


// --------- internal lines ----------

typedef struct rvt_gline_t {

    rs_point_t p1;
    rs_point_t p2;
    float azimuth1;
    float azimuth2;
    float thickness;
    float length;
    int flags;

    gd_lines_data_t *pdata;

} rvt_gline_t;



enum {
    RVT_GLINE_LAYER_ROAD = 0,
    RVT_GLINE_LAYER_PATH,   // including footways
    RVT_GLINE_LAYER_BARRIER,
    RVT_GLINE_LAYER_RAILWAY,
    RVT_GLINE_LAYER_RIVER,

    RVT_GLINE_MAX_LAYERS_COUNT
};

#define RVT_GLINE_FLAG_NONE    0x0000
#define RVT_GLINE_FLAG_ANY     0xFFFF

#define RVT_GLINE_FLAG_HW_RESIDENTIAL  0x0002  // also living_street
#define RVT_GLINE_FLAG_HW_TERTIARY     0x0004
#define RVT_GLINE_FLAG_HW_SECONDARY_OR_HIGHER     0x0008

#define RVT_GLINE_FLAG_RAILWAY     0x0010
#define RVT_GLINE_FLAG_WATERWAY    0x0020


// -------- internal area-triangles --------

typedef struct rvt_gtriangle_t {

    rs_triangle_t triangle;
    float area; // TODO: check if this parameter is needed
    int type_mask;
    int flags;


} rvt_gtriangle_t;




enum {
    RVT_GTRIANGLE_LAYER_WATER = 0,
    RVT_GTRIANGLE_LAYER_LANDUSE,
    RVT_GTRIANGLE_LAYER_BUILDINGS,
    RVT_GTRIANGLE_LAYER_GREEN_NATURALS,
    RVT_GTRIANGLE_LAYER_ASPHALT,
    RVT_GTRIANGLE_MAX_LAYERS_COUNT
};

#define RVT_GTRIANGLE_FLAG_DEFAULT             0x0001
#define RVT_GTRIANGLE_FLAG_RESIDENTIAL_UNKNOWN     0x0010
#define RVT_GTRIANGLE_FLAG_RESIDENTIAL_RURAL       0x0020
#define RVT_GTRIANGLE_FLAG_RESIDENTIAL_URBAN       0x0040
#define RVT_GTRIANGLE_FLAG_RESIDENTIAL_ANY         0x0070

#define RVT_GTRIANGLE_FLAG_FOREST      0x0100
#define RVT_GTRIANGLE_FLAG_HEATH       0x0200
#define RVT_GTRIANGLE_FLAG_SCRUB       0x0400
#define RVT_GTRIANGLE_FLAG_GRASSLAND   0x0800

#define RVT_GTRIANGLE_FLAG_ANY     0xFFFF
#define RVT_GTRIANGLE_FLAG_NONE    0x0000



#define RVT_GEN_MAX_STAGES      4

#define RVT_GEN_MAX_FUNCS_PER_LAYER    256

#define RVT_GEN_FUNC_TYPE_NULL  0
#define RVT_GEN_FUNC_TYPE_C     1
#define RVT_GEN_FUNC_TYPE_LUA   2

typedef void RVTGENFUNC(void *geometry_data_pointer, int stage_i, void *data);
typedef RVTGENFUNC *PRVTGENFUNC;

typedef struct rvt_gen_func_rec_t {
    int func_type;
    int lua_func_index;
    PRVTGENFUNC func;
} rvt_gen_func_rec_t;

typedef struct rvt_gen_stage_t {
    rvt_gen_func_rec_t func_recs[RVT_GD_MAX_LAYERS_COUNT+1][RVT_GEN_MAX_FUNCS_PER_LAYER];
    int func_recs_count[RVT_GD_MAX_LAYERS_COUNT+1];
} rvt_gen_stage_t;


typedef struct rvt_tile_t {

    int status;
    int flags;

    int tile_x;
    int tile_y;

} rvt_tile_t;


#define RVT_TILE_STATUS_NONE       0
#define RVT_TILE_STATUS_LOADING    1
#define RVT_TILE_STATUS_NOT_AVAILABLE  2
#define RVT_TILE_STATUS_LOADING_HM_LOADED    3
#define RVT_TILE_STATUS_LOADED     4

#define RVT_TILE_FLAG_LOGIC_OBJS_LOADED        0x0001


typedef struct rvt_subtile_t {

    int fov_status;
    int distance;
    float adaptive_distance;

} rvt_subtile_t;


#define RVT_GD_GEN_FLAG_DONE   0x0001

#define RVT_BASE_STYLE_DEFAULT  0
#define RVT_BASE_STYLE_MAP_EDITOR   1

#define RVT_BUILDING_S_TYPE    0
#define RVT_BUILDING_S_MATERIAL    1
#define RVT_BUILDING_S_COLOUR      2
#define RVT_BUILDING_S_ROOF_COLOUR 3

#define RVT_MAX_CACHE_FOLDERS      32

typedef struct rvt_cache_settings_t {

    #ifdef STREETS_GAME
        int custom_g_server;
        char custom_g_server_addr[256];
        char custom_g_server_path[256];
        int custom_g_server_port;

        int custom_h_server;
        char custom_h_server_addr[256];
        char custom_h_server_path[256];
        int custom_h_server_port;

        int custom_g_cache;
        char custom_g_cache_path[256];

        int custom_h_cache;
        char custom_h_cache_path[256];
    #endif

    // Cache folders
    char cache_folders[RVT_MAX_CACHE_FOLDERS][256];
    int cache_folders_count;

} rvt_cache_settings_t;


typedef struct rvt_subpak_cache_t {
    unsigned int z;
    unsigned int x;
    unsigned int y;
    unsigned int len;

    unsigned int parts_count;
    unsigned int parts_mask;
    unsigned int tiles_count_per_subpak;
    unsigned int dest_z;

    unsigned char *data;
} rvt_subpak_cache_t;

#define RVT_SUBPAK_CACHE_COUNT     8


typedef struct rvt_geodata_struct_t {

    rvt_gen_stage_t rvt_gen_stages[RVT_GEN_MAX_STAGES];

    struct rvt_tile_t tile[RVT_TILES_TOTAL_COUNT];
    struct rvt_subtile_t subtile[RVT_SUBTILES_TOTAL_COUNT];

    int start_tile_world_ix;
    int start_tile_world_iy;

    // Subpak cache for tile loader
    rvt_subpak_cache_t subpak_cache[RVT_SUBPAK_CACHE_COUNT];
    int subpak_cache_counter;

    // Heightmap
    rvt_heightmap_data_t hm[RVT_TILES_TOTAL_COUNT];

    // Source Geodata

    unsigned char *gd_data[RVT_TILES_TOTAL_COUNT][RVT_GD_MAX_LAYERS_COUNT][RVT_GD_MAX_ARRAYS_COUNT];
    int gd_list_count[RVT_TILES_TOTAL_COUNT][RVT_GD_MAX_LAYERS_COUNT];
    int gd_items_len[RVT_TILES_TOTAL_COUNT][RVT_GD_MAX_LAYERS_COUNT][RVT_GD_MAX_ARRAYS_COUNT];
    int gd_items_count[RVT_TILES_TOTAL_COUNT][RVT_GD_MAX_LAYERS_COUNT][RVT_GD_MAX_ARRAYS_COUNT];


    // GeoPoints
    struct rvt_gpoint_t *gp[RVT_SUBTILES_TOTAL_COUNT][RVT_GP_MAX_LAYERS_COUNT][RVT_GP_MAX_ARRAYS_COUNT];
    int gp_list_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GP_MAX_LAYERS_COUNT];
    int gp_items_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GP_MAX_LAYERS_COUNT][RVT_GP_MAX_ARRAYS_COUNT];


    // GeoLines
    struct rvt_gline_t *gline[RVT_SUBTILES_TOTAL_COUNT][RVT_GLINE_MAX_LAYERS_COUNT][RVT_GLINE_MAX_ARRAYS_COUNT];
    int gline_list_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GLINE_MAX_LAYERS_COUNT];
    int gline_items_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GLINE_MAX_LAYERS_COUNT][RVT_GLINE_MAX_ARRAYS_COUNT];

    // GeoTriangles (areas)
    struct rvt_gtriangle_t *gtriangle[RVT_SUBTILES_TOTAL_COUNT][RVT_GTRIANGLE_MAX_LAYERS_COUNT][RVT_GTRIANGLE_MAX_ARRAYS_COUNT];
    int gtriangle_list_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GTRIANGLE_MAX_LAYERS_COUNT];
    int gtriangle_items_count[RVT_SUBTILES_TOTAL_COUNT][RVT_GTRIANGLE_MAX_LAYERS_COUNT][RVT_GTRIANGLE_MAX_ARRAYS_COUNT];

    //
    unsigned int gflags[RVT_SUBTILES_TOTAL_COUNT];

} rvt_geodata_struct_t;



#ifdef STREETS_GAME

    #define RVT_GNOTE_MAX_TYPE_STRING_LEN   50
    #define RVT_GNOTE_MAX_STRING_LEN   20

    typedef struct rvt_gnote_t {

        int status;
        int timestamp;

        int type;

        double lat;
        double lon;
        int b_levels;
        char b_type[RVT_GNOTE_MAX_TYPE_STRING_LEN];
        char b_material[RVT_GNOTE_MAX_STRING_LEN];
        char b_colour[RVT_GNOTE_MAX_STRING_LEN];
        char roof_colour[RVT_GNOTE_MAX_STRING_LEN];

        rs_vec2_t global_pos;

    } rvt_gnote_t;

#else

    typedef int rvt_gnote_t;

#endif


typedef struct rvt_exporting_struct_t {

    int exporting;
    int exporting_abort;

    int exporting_xi;
    int exporting_xi_count;
    int exporting_yi;
    int exporting_yi_count;

    uint64_t exporting_total_bytes;
    uint64_t exporting_total_vertices;

} rvt_exporting_struct_t;


#endif
