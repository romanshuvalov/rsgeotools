#ifndef RS_RVT_H
#define RS_RVT_H


#include "rvttypes.h"

/*

tile_i = index in array
tile_ix, tile_iy = (x,y) index in array ( i%side_count, i/side_count )
tile_x, tile_y = index in world, may be negative
tile_px, tile_py = position in world, float, may be negative, px = x * sc, py = y * sc
tile_worldix, tile_worldiy = position in world tile coordinates (like in z/x/y)
tile_z = z_scale

tile_jx, tile_jy = (x,y) index in fractal pak/subpak
tile_j = index in fractal pak/subpak

*/

#define RVT_BUILDING_SIMPLIFICATION_TOLERANCE  (0.51)
#define RVT_BUILDING_CONVEX_HULL_SIMPLIFICATION_TOLERANCE  (0.73)

#define RVT_NO_COLOR    0xFFFFFFFF

#define RVT_GEOM_FLAG_OPEN                  0x0001
#define RVT_GEOM_FLAG_ADD_HM_TO_POINTS      0x0002
#define RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT    0x0004
#define RVT_GEOM_FLAG_USE_VBODATA_COLORS    0x0008
#define RVT_GEOM_FLAG_ADD_HM                0x0010
#define RVT_GEOM_FLAG_FLAT_WALL             0x0020
#define RVT_GEOM_FLAG_BUILDING_COLOR_CODE   0x0040


#define RVT_BRIDGE_HEIGHT  (8.0)



#define OSM_TAG_YES     1
#define OSM_TAG_NO      2
#define OSM_TAG_DESIGNATED   3

#define OSM_TAG_BIT_BUILDING_RESIDENTIAL    0x1000
#define OSM_TAG_BIT_BUILDING_PUBLIC         0x2000
#define OSM_TAG_BIT_BUILDING_INDUSTRIAL     0x4000
#define OSM_TAG_BIT_BUILDING_RUINS          0x8000

enum {

    OSM_TAG_BUILDING_WALL = 16,

    OSM_TAG_BUILDING_APARTMENTS = 0x1000,
    OSM_TAG_BUILDING_RESIDENTIAL,
    OSM_TAG_BUILDING_HOUSE,
    OSM_TAG_BUILDING_DETACHED,
    OSM_TAG_BUILDING_SEMIDETACHED_HOUSE,
    OSM_TAG_BUILDING_SEMI,
    OSM_TAG_BUILDING_HUT,
    OSM_TAG_BUILDING_TERRACE,
    OSM_TAG_BUILDING_DORMITORY,

    OSM_TAG_BUILDING_COMMERCIAL = 0x2000,
    OSM_TAG_BUILDING_RETAIL,
    OSM_TAG_BUILDING_OFFICE,
    OSM_TAG_BUILDING_PUBLIC,

    OSM_TAG_BUILDING_SHOP,
    OSM_TAG_BUILDING_SUPERMARKET,

    OSM_TAG_BUILDING_HOSPITAL,
    OSM_TAG_BUILDING_SCHOOL,
    OSM_TAG_BUILDING_KINDERGARTEN,
    OSM_TAG_BUILDING_UNIVERSITY,
    OSM_TAG_BUILDING_COLLEGE,

    OSM_TAG_BUILDING_CHURCH,
    OSM_TAG_BUILDING_CATHEDRAL,


    OSM_TAG_BUILDING_INDUSTRIAL = 0x4000,
    OSM_TAG_BUILDING_GARAGES,
    OSM_TAG_BUILDING_WAREHOUSE,
    OSM_TAG_BUILDING_SERVICE,
    OSM_TAG_BUILDING_FARM,
    OSM_TAG_BUILDING_HANGAR,
    OSM_TAG_BUILDING_SILO,
    OSM_TAG_BUILDING_ABANDONED,

    OSM_TAG_BUILDING_COLLAPSED = 0x8000,
    OSM_TAG_BUILDING_RUINS,
    OSM_TAG_BUILDING_CONSTRUCTION,

};


#define OSM_TAG_RVT_RELATION_ROLE_OUTLINE   16

enum {
    OSM_TAG_NATURAL_WOOD = 16,
    OSM_TAG_NATURAL_WATER,
    OSM_TAG_NATURAL_BEACH,
    OSM_TAG_NATURAL_SAND,
    OSM_TAG_NATURAL_GRASSLAND,

    OSM_TAG_NATURAL_HEATH,
    OSM_TAG_NATURAL_SCRUB,

    OSM_TAG_NATURAL_TREE,
    OSM_TAG_NATURAL_PEAK,
    OSM_TAG_NATURAL_VOLCANO,
};


enum {
    OSM_TAG_POWER_LINE = 16,
    OSM_TAG_POWER_MINOR_LINE,

    OSM_TAG_POWER_TOWER,
    OSM_TAG_POWER_POLE
};

enum {
    OSM_TAG_LANDUSE_RESIDENTIAL = 16,
    OSM_TAG_LANDUSE_COMMERCIAL,
    OSM_TAG_LANDUSE_INDUSTRIAL,

    OSM_TAG_LANDUSE_QUARRY,

    OSM_TAG_LANDUSE_RESERVOIR,

    OSM_TAG_LANDUSE_FOREST,
    OSM_TAG_LANDUSE_GRASS
};

enum {
    OSM_TAG_RESIDENTIAL_UNKNOWN = 16,
    OSM_TAG_RESIDENTIAL_RURAL,
    OSM_TAG_RESIDENTIAL_URBAN
};

enum {
    OSM_TAG_WATERWAY_RIVERBANK = 16,

    OSM_TAG_WATERWAY_RIVER,
    OSM_TAG_WATERWAY_STREAM,
};

enum {
    OSM_TAG_AEROWAY_RUNWAY = 16,
    OSM_TAG_AEROWAY_TAXIWAY,

    OSM_TAG_AEROWAY_APRON,
    OSM_TAG_AEROWAY_TERMINAL,
};


enum {

    OSM_TAG_RAILWAY_RAIL      = 16,
    OSM_TAG_RAILWAY_TRAM,

    OSM_TAG_RAILWAY_LEVEL_CROSSING

};


enum {
    OSM_TAG_HW_RESIDENTIAL      = 16,
    OSM_TAG_HW_SERVICE,
    OSM_TAG_HW_TERTIARY_LINK,
    OSM_TAG_HW_TERTIARY,
    OSM_TAG_HW_SECONDARY_LINK,
    OSM_TAG_HW_SECONDARY,
    OSM_TAG_HW_PRIMARY_LINK,
    OSM_TAG_HW_PRIMARY,
    OSM_TAG_HW_MOTORWAY_LINK,
    OSM_TAG_HW_MOTORWAY,
    OSM_TAG_HW_TRUNK_LINK,
    OSM_TAG_HW_TRUNK,

    OSM_TAG_HW_TRACK,
    OSM_TAG_HW_FOOTWAY,
    OSM_TAG_HW_PATH,

    OSM_TAG_HW_UNCLASSIFIED,
    OSM_TAG_HW_BRIDLEWAY,

    OSM_TAG_HW_LIVING_STREET,

    OSM_TAG_HW_CYCLEWAY,

    OSM_TAG_HW_PEDESTRIAN,


    OSM_TAG_HW_BUS_STOP,
    OSM_TAG_HW_CROSSING,
    OSM_TAG_HW_TRAFFIC_SIGNALS,

    OSM_TAG_HW_GIVE_WAY,
    OSM_TAG_HW_STOP

};

enum {

    OSM_TAG_DIRECTION_FORWARD = 16,
    OSM_TAG_DIRECTION_BACKWARD

};

enum {

    OSM_TAG_CROSSING_UNCONTROLLED   = 16,
    OSM_TAG_CROSSING_TRAFFIC_SIGNALS

};


#define OSM_TAG_ONEWAY_YES          16


enum {

    OSM_TAG_BIT_SURFACE_PAVED   = 0x0100,

    OSM_TAG_SURFACE_PAVED,
    OSM_TAG_SURFACE_ASPHALT,
    OSM_TAG_SURFACE_CONCRETE,

    OSM_TAG_BIT_SURFACE_UNPAVED = 0x0200,

    OSM_TAG_SURFACE_UNPAVED,
    OSM_TAG_SURFACE_GROUND,
    OSM_TAG_SURFACE_MUD,
    OSM_TAG_SURFACE_DIRT,
    OSM_TAG_SURFACE_SAND,

};


#define OSM_TAG_LIT_YES             16
#define OSM_TAG_LIT_NO              17



#define OSM_TAG_ENTRANCE_YES  16
#define OSM_TAG_ENTRANCE_MAIN  17
#define OSM_TAG_ENTRANCE_STAIRCASE  18


#define OSM_TAG_AMENITY_PARKING      16

// key: leisure
enum {
    OSM_TAG_LEISURE_PITCH = 16,
    OSM_TAG_LEISURE_PLAYGROUND,
    OSM_TAG_LEISURE_PARK,
    OSM_TAG_LEISURE_GARDEN,

};
//#define OSM_TAG_LEISURE_PITCH       16
//#define OSM_TAG_LEISURE_PLAYGROUND  17

// key: barrier
enum {
    OSM_TAG_BARRIER_FENCE = 16,
    OSM_TAG_BARRIER_WALL,
    OSM_TAG_BARRIER_BLOCK
};
//#define OSM_TAG_BARRIER_FENCE       16
//#define OSM_TAG_BARRIER_WALL        17
//#define OSM_TAG_BARRIER_BLOCK       18

// key: water
enum {
    OSM_TAG_WATER_RESERVOIR = 16,
    OSM_TAG_WATER_LAKE,
};

// key: place
enum {
    OSM_TAG_PLACE_YES = 16, // temporary
};

// key: roof:shape
enum {
    OSM_TAG_ROOF_SHAPE_UNKNOWN = 16,
    OSM_TAG_ROOF_SHAPE_FLAT,
    OSM_TAG_ROOF_SHAPE_DOME,
    OSM_TAG_ROOF_SHAPE_ONION

};

#define RVT_BUILDING_LEVEL_HEIGHT   (2.8)


#define RVT_STRIDE_BUILDING     20
#define RVT_STRIDE_WIRE         8
#define RVT_STRIDE_ROAD         20


#define RVT_MAX_PARTS   4096
#define RVT_VERTICES_PER_PART  8192
#define RVT_INDICES_PER_PART  8192



typedef struct rvt_reg_t {

    int rvt_counter_vertices_total;

    float shift_x;
    float shift_y;


    rs_vec3_t geom_color;
    rs_vec3_t geom_color_alt;

    int rvt_writing_layer;
    int rvt_writing_igroup;


    float *rvt_vert_data[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_MAX_PARTS];
    int *rvt_vert_map[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_MAX_PARTS];
    int rvt_vert_parts[RVT_SUBTILES_VBO_LAYERS_COUNT];
    int rvt_total_vertices[RVT_SUBTILES_VBO_LAYERS_COUNT];
    int rvt_vert_current_index[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_SUBTILES_TOTAL_COUNT + RVT_TILES_TOTAL_COUNT];

    uint32_t *rvt_ind_data[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_IGROUPS_COUNT][RVT_MAX_PARTS];
    int *rvt_ind_map[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_IGROUPS_COUNT][RVT_MAX_PARTS];
    int rvt_ind_parts[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_IGROUPS_COUNT];
    int rvt_total_indices[RVT_SUBTILES_VBO_LAYERS_COUNT][RVT_IGROUPS_COUNT];

    int rvt_subtile_i;

    int tile_i;

    int tile_ix;
    int tile_iy;

    // for debugging only:
    int tile_z;
    int tile_x;
    int tile_y;


    // For exporting:
    int export_to_file;

} rvt_reg_t;

extern struct rvt_reg_t rvt_reg;


typedef struct rvt_settings_t {

    rvt_export_file_t rvt_file_arr[ RVT_SUBTILES_VBO_LAYERS_COUNT + 1 ]; // + 1 for heightmap

    int exporting;

    int file_format;

    int separate_to_tiles;

    int z_up;

    int layer_enabled[ RVT_SUBTILES_VBO_LAYERS_COUNT + 1 ];

    char folder_name[200];

    // Generator settings

    int flat_terrain;

    int building_decorations; // roof & entrances

    int smooth_area_edges;
    int smooth_road_edges;
    int smooth_river_edges;

} rvt_settings_t;

extern struct rvt_settings_t rvt_settings;

void rvt_settings_set_to_defaults();


typedef struct rs_rvt0_file_header_t {
    uint32_t magic_num;
    uint32_t version;
    uint32_t r0;
    uint32_t r1;
} rs_rvt0_file_header_t;

typedef struct rs_rvt0_data_header_t {
    uint32_t layers_count;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
} rs_rvt0_data_header_t;

typedef struct rs_rvt0_layer_header_t {
    uint32_t features_count;
    uint32_t layer_type;
    uint32_t geom_set_count;
    uint32_t r1;
} rs_rvt0_layer_header_t;

typedef struct rs_rvt0_feature_header_t {
    uint32_t feature_flags;
    uint32_t area;
    uint32_t levels;
    union {
        uint32_t rank_value;
        uint32_t landuse_group;
    };

    uint64_t id;
    uint64_t reserved;

} rs_rvt0_feature_header_t;

typedef struct rs_rvt0_geom_header_t {
    uint32_t points_count;
    uint32_t geom_type;
} rs_rvt0_geom_header_t;


#define RVT_X(c)  ( 256.0f/63488.0*(-1024L + (uint16_t) ( ((c)>>0)  & 0xFFFF))  )
#define RVT_Y(c)  ( 256.0f/63488.0*(-1024L + (uint16_t) ( ((c)>>16) & 0xFFFF))  )
#define RVT_PACK_XY(x,y)   (  ((uint16_t)(1024.0+(x))) | (((uint32_t)(1024.0+(y)))<<16)  )



typedef struct gpak_file_header_t {

    uint32_t magic_num0;
    uint32_t magic_num1;
    uint32_t version;           // must be 2
    uint32_t flags;

    uint64_t timestamp_planet_osm;
    uint64_t timestamp_pak_creation;

    uint32_t pak_z;
    uint32_t pak_x;
    uint32_t pak_y;
    uint32_t reserved0;

    uint32_t subpak_z;         // tiles are compressed in subpaks
    uint32_t subpak_count;     // must be 4^(subtile_z - pak_z)
    uint32_t dest_z;           // actual tiles
    uint32_t dest_count_per_subpak; // must be 4^(dest_z - subtile_z)

    uint32_t parts_count;       // typically 5 parts for version=2: heightmap, layer:building, layer:area, layer:lines, layer:points
    uint32_t parts_mask;
    uint32_t parts_reserved0;
    uint32_t parts_reserved1;

    uint32_t hm_format;
    uint32_t hm_image_size;
    uint32_t hm_reserved0;
    uint32_t hm_reserved1;

    uint32_t gd_format;
    uint32_t gd_reserved0;
    uint32_t gd_reserved1;
    uint32_t gd_reserved2;

    uint32_t extensions_total_bytes;
    uint32_t extensions_flags;
    uint32_t extensions_reserved0;
    uint32_t extensions_reserved1;

} gpak_file_header_t;



typedef struct subpak_map_record_t {

    uint64_t bytes_shift;

    uint32_t bytes_len;
    uint32_t uncompressed_bytes_len;
    uint32_t compression_type;      // 0 uncompressed, 1 gzip, 2 bzip2
    uint32_t flags;


} subpak_map_record_t;

typedef struct tile_map_record_t {

    uint32_t bytes_shift;
    uint32_t bytes_len;
    uint32_t flags;
    uint32_t reserved0;

} tile_map_record_t;


#define RS_COMPRESS_TYPE_NONE       0
#define RS_COMPRESS_TYPE_ZLIB       1
#define RS_COMPRESS_TYPE_BZIP2      2


#define RS_WKB_POINT        1
#define RS_WKB_LINESTRING   2
#define RS_WKB_POLYGON      3
#define RS_WKB_MULTIPOINT   4
#define RS_WKB_MULTILINESTRING  5
#define RS_WKB_MULTIPOLYGON 6
#define RS_WKB_GEOMETRY_COLLECTION  7




#define RS_SMV_LINES     0
#define RS_SMV_TRIANGLES 1

void rvt_begin(int tile_i);
void rvt_end(int custom_subtile_i, int tile_ix, int tile_iy);

void rs_geoconv_tc2proj(int zoom_level, double x, double y, double *output_x, double *output_y);
void rs_geoconv_proj2tc(int zoom_level, double x, double y, double *output_x, double *output_y);

rs_vec2_t rs_geoconv_latlon2tc(double lat, double lon);
rs_vec2_t rs_geoconv_tc2degrees(int zoom_level, double x, double y);

int osm_value_is_non_zero(char *s);

void rvt_color(float r, float g, float b);
void rvt_color_alt(float r, float g, float b);
void rvt_color_i(uint32_t c);

void rvt_write_stripe_linestring( int geom_flags, float y_start, float stripe_halfwidth, float stripe_center_shift, float tc_v1, float tc_v2, int disable_k_end, float ca_left, float ca_right, float y_left, rs_linestring_t *ring);
void rvt_write_stripe( int geom_flags, float y_start, float stripe_halfwidth, float stripe_center_shift, float tc_v1, float tc_v2, int disable_k_end, float ca_left, float ca_right, float y_left, rs_shape_t *p );

void rvt_write_wall_linestring( int geom_type, float y_start, float y_height, float v_base, float v_top, float uv_scale, float ca_component, rs_linestring_t *ring);
void rvt_write_wall( int geom_type, float y_start, float y_height, float v_base, float v_top, float uv_scale, float ca_component, rs_shape_t *p );

void rvt_write_roof( float roof_start, float roof_height, rs_shape_t *p );


void rvt_write_vbodata( int add_hm_height, rs_vec3_t pos, rs_vec3_t scale, float angle, float uv2, float uv3, int index );

void rvt_write_wire( rs_vec3_t p1, rs_vec3_t p2, float wire_thickness, float wire_height, int flags );

void rvt_write_triangles( int add_hm_height, float y_start, float y_height, rs_triangle_set_t *tset );



void rvt_set_writing_layer(int layer_index);
void rvt_set_writing_igroup(int igroup);

unsigned int conv_xy_to_j(unsigned int x, unsigned int y);
unsigned int conv_j_to_x(unsigned int j);
unsigned int conv_j_to_y(unsigned int j);

float rvt_calc_sc( rs_vec2_t latlon );

int rvt_calc_building_levels_by_shape(rs_shape_t *p);

uint32_t rvt_get_colour_from_string(char *s);
uint32_t rvt_get_building_type_from_string(char *s);


#endif
