#include "rvtloader.h"

#include "main.h"

#include "rvtapp.h"
#include "rvtutil.h"

#include "rs/rsgeom.h"
#include "rs/rsmem.h"

#include "loader.h"

#ifdef STREETS_GAME
    #include "rvtnetloader.h"
#endif

#include <math.h>
#include <zlib.h>
#include <bzlib.h>
#include <string.h>

#include "rs/rsnoise.h"


unsigned char *rvt_data;
unsigned char *rvt_data_pointer;


void read_byte(unsigned char *b) {
    memcpy(b, rvt_data_pointer, 1);
    rvt_data_pointer+=1;
};

void read_us(unsigned short *us) {
    memcpy(us, rvt_data_pointer, 2);
    rvt_data_pointer+=2;
};

void read_int(unsigned int *i) {
    memcpy(i, rvt_data_pointer, 4);
    rvt_data_pointer+=4;
};


void read_double(double *d) {
    memcpy(d, rvt_data_pointer, 8);
    rvt_data_pointer+=8;
};

void read_buf(unsigned char *buf, unsigned long len) {
    memcpy(buf, rvt_data_pointer, len);
    rvt_data_pointer+=len;
};




#define DEBUGRVTf  DEBUG30f

#define DEBUGRVT20f  DEBUG30f

#define DEBUGRVTSTATf   DEBUG10f





void rvt_process_file(unsigned char **pdata, unsigned int *pdata_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int data_type, int part_i/*, int tile_ix, int tile_iy*/) {

    DEBUGRVTSTATf("rvt_process_file() start (%d/%d/%d, type %d part %d)\n", tile_z, tile_world_ix, tile_world_iy, data_type, part_i );

    *pdata = NULL;
    *pdata_len = 0;

    char c_part;
    int part_bit;

    if (data_type == RVT_TYPE_HEIGHTMAP) {
        c_part = 'h';
        part_bit = 0;
    }
    else if (data_type == RVT_TYPE_GEODATA) {
        c_part = 'g';
        part_bit = 1 << (part_i+1);
    }
    #ifdef STREETS_GAME
    else if (data_type == RVT_TYPE_GENERALIZED_WORLD) {
        c_part = 'w';
        part_bit = 1 << (part_i+5);
    }
    #endif
    else {
        rs_critical_alert_and_halt_sprintf("Not implemented (data_type = %d) (%d/%d/%d)\n", data_type, tile_z, tile_world_ix, tile_world_iy);
        return;
    };


    int parts_count = 1;    // 'h'
    int parts_mask = 0b00000001; // 'h'
    if (c_part == 'g') {
        parts_count = 4;
        parts_mask = 0b00011110;
    };

    unsigned int data_size;
    unsigned int dest_data_size;
    unsigned long dest_data_size_long;


    int tile_subpak_z_diff = tile_z - subpak_z;
    int subpak_world_ix = tile_world_ix >> tile_subpak_z_diff;
    int subpak_world_iy = tile_world_iy >> tile_subpak_z_diff;


    // Tile position in subpak
    int tile_jx = tile_world_ix - (subpak_world_ix << tile_subpak_z_diff);
    int tile_jy = tile_world_iy - (subpak_world_iy << tile_subpak_z_diff);
    int tile_j = conv_xy_to_j(tile_jx, tile_jy);

    // 1. Search in cache

    DEBUGRVTSTATf("Searching in cache for %d/%d/%d (subpak_z %d) part_bit %08x: \n", tile_z, subpak_world_ix, subpak_world_iy, subpak_z, part_bit);

    for (int i = 0; i < RVT_SUBPAK_CACHE_COUNT; i++) {

        rvt_subpak_cache_t *spc = &rvt_app_get_geodata()->subpak_cache[i];

        if ( spc->data != NULL ) {

            DEBUGRVTSTATf("(%d/%d/%d %d %02x)... ",spc->z, spc->x, spc->y, spc->dest_z, spc->parts_mask );

            if ( (spc->dest_z == tile_z) && (spc->z == subpak_z) && (spc->x == subpak_world_ix) && (spc->y == subpak_world_iy) && (spc->parts_mask & part_bit ) ) {

                DEBUGRVTSTATf("(!!!)... ");

                if ( part_i >= spc->parts_count ) {
                    // We don't have this part_i, probably out of bounds
                    return;
                };

                tile_map_record_t *tmr = (tile_map_record_t*) spc->data;

                unsigned int bytes_shift = tmr[tile_j*spc->parts_count + (part_i) ].bytes_shift;
                unsigned int bytes_len =   tmr[tile_j*spc->parts_count + (part_i) ].bytes_len;

                if (bytes_len == 0) {
                    DEBUGRVTSTATf("No data for %d/%d/%d (subpak in cache)\n", tile_z, tile_world_ix, tile_world_iy );
                    return;
                };


                DEBUGRVTSTATf("Found in cache: %d/%d/%d, bytes_shift %d, bytes_len %d (spc->len %d, spc->parts_count %d, spc->parts_mask %d)\n", spc->z, spc->x, spc->y,
                    bytes_shift, bytes_len,
                    spc->len, spc->parts_count, spc->parts_mask );

                *pdata_len = bytes_len;
                *pdata = spc->data + bytes_shift;

                return;

            };
        };

    };



    // 2. Search in previously downloaded files

    int downloaded = 0;

    char s_filename[200];
    s_filename[0] = '\0';

    char *custom_cache_folder = NULL;
    #ifdef STREETS_GAME
    if ( (c_part == 'g') && (rvt_app_get_cache_settings()->custom_g_cache) ) {
        custom_cache_folder = rvt_app_get_cache_settings()->custom_g_cache_path;
    };
    if ( (c_part == 'h') && (rvt_app_get_cache_settings()->custom_h_cache) ) {
        custom_cache_folder = rvt_app_get_cache_settings()->custom_h_cache_path;
    };
    #endif

    int z_diff;
    int pak_world_ix, pak_world_iy;
    int subpak_z_diff;

    FILE *fp = NULL;

    if (1) {

        for (int pz = subpak_z; pz >= 0; pz--) {

            int pak_is_found = 0;

            z_diff = tile_z - pz;
            pak_world_ix = tile_world_ix >> z_diff;
            pak_world_iy = tile_world_iy >> z_diff;
            subpak_z_diff = subpak_z - pz;

            char s_folder[255];

            for (int fi = -1; fi < rvt_app_get_cache_settings()->cache_folders_count; fi++) {

                if (fi == -1) {
                    if (!custom_cache_folder) {
                        continue;
                    };
                    strcpy(s_folder, custom_cache_folder);
                }
                else {
                    sprintf(s_folder, "%s/%c", rvt_app_get_cache_settings()->cache_folders[fi], c_part);
                };

                if (c_part == 'w') {
                    sprintf(s_filename, "%s/%d_%d_%d_%c_z%d.gpak", s_folder, pz, pak_world_ix, pak_world_iy, c_part, tile_z );
                }
                else {
                    sprintf(s_filename, "%s/%d_%d_%d_%c.gpak", s_folder, pz, pak_world_ix, pak_world_iy, c_part );
                };

                fp = fopen(s_filename, "rb");
                if (fp) {
                    pak_is_found = 1;
                    #ifdef STREETS_GAME
                    if (fi == 0) {
                        downloaded = 1; // file is in "_downloaded" folder, it's safe to delete it in case of trouble
                    }
                    #endif
                    break;
                }
                else {
                    DEBUGRVTSTATf("Not found, looking for next...\n");
                };

            };

            if (pak_is_found) {
                break;
            };

        };

    };


    // 3. Download tile if not found (not implemented in this tool)

    #ifdef STREETS_GAME
    if (!fp) {
        rvt_download_tile(&fp, tile_z, tile_world_ix, tile_world_iy, c_part, subpak_z, &pak_world_ix, &pak_world_iy, &subpak_z_diff);
    };
    #endif


    if (!fp) {

        // Subpak doesn't exist.
        // Creating empty subpak cache record:

        DEBUGRVTSTATf("Subpak doesn't exist, creating empty subpak cache record.\n");

        rvt_subpak_cache_t *spc = &rvt_app_get_geodata()->subpak_cache[rvt_app_get_geodata()->subpak_cache_counter];
        if (spc->data != NULL) {
            rs_mem_free(spc->data);
        };

        spc->data = NULL;
        spc->len = 0;
        spc->z = subpak_z;
        spc->x = subpak_world_ix;
        spc->y = subpak_world_iy;
        spc->parts_count = parts_count;
        spc->parts_mask = parts_mask;
        spc->tiles_count_per_subpak = 1 << (tile_z - subpak_z);
        spc->dest_z = tile_z;
        rvt_app_get_geodata()->subpak_cache_counter = (rvt_app_get_geodata()->subpak_cache_counter + 1) % RVT_SUBPAK_CACHE_COUNT;

        *pdata_len = 0;
        *pdata = spc->data;

        return;

    };


    DEBUGRVTSTATf("pak_world_ix %d, pak_world_iy %d, subpak_z_diff %d", pak_world_ix, pak_world_iy, subpak_z_diff);

    // Subpak position in pak
    int subpak_jx = subpak_world_ix - (pak_world_ix << subpak_z_diff);
    int subpak_jy = subpak_world_iy - (pak_world_iy << subpak_z_diff);
    int subpak_j = conv_xy_to_j(subpak_jx, subpak_jy);

    DEBUGRVTSTATf("subpak jx %d, jy %d, j %d\n", subpak_jx, subpak_jy, subpak_j);


    gpak_file_header_t hdr;
    subpak_map_record_t rec;

    fread(&hdr, sizeof(struct gpak_file_header_t), 1, fp);

    DEBUGRVTSTATf("Pak parts_mask: %08x\n", hdr.parts_mask );

    if (hdr.subpak_z != subpak_z) {

        if (downloaded) {
            fclose(fp);
            fp = NULL;
            unlink(s_filename);
        };

        rs_critical_alert_and_halt_sprintf("Wrong subpak_z (expected %d, pak has %d) (%d/%d/%d)\nYou may want to clear geodata cache", subpak_z, hdr.subpak_z,
            tile_z, tile_world_ix, tile_world_iy);
    };

    if (hdr.dest_z != tile_z) {
        if (downloaded) {
            fclose(fp);
            fp = NULL;
            unlink(s_filename);
        };

        rs_critical_alert_and_halt_sprintf("Wrong dest_z (expected %d, pak has %d) (%d/%d/%d)\nYou may want to clear geodata cache", tile_z, hdr.dest_z,
            tile_z, tile_world_ix, tile_world_iy);
    };

    #ifdef STREETS_GAME
    if (data_type == RVT_TYPE_GENERALIZED_WORLD) {
        hdr.parts_mask = 0x7FFF;
    };
    #endif

    fseek(fp, sizeof(struct subpak_map_record_t) * ( subpak_j ), SEEK_CUR );

    fread(&rec, sizeof(struct subpak_map_record_t), 1, fp );

    DEBUGRVTSTATf("Subpak map record: shift %d, len %d, unc len %d\n", rec.bytes_shift, rec.bytes_len, rec.uncompressed_bytes_len);


    fseek(fp, sizeof(struct gpak_file_header_t) + hdr.subpak_count*sizeof(struct subpak_map_record_t)
        + rec.bytes_shift, SEEK_SET );


    if ( (rec.bytes_len == 0) ) {

        DEBUGRVTSTATf("SUBPAK err: rec.bytes_len = 0 (rec.bytes_shift = %d)\n", rec.bytes_len, rec.bytes_shift);
        fclose(fp);

        return; // TODO: check if everything is fine

    };


    DEBUGRVTSTATf("allocating for compressed data, rec.bytes_len = %d\n", rec.bytes_len);
    unsigned char *subtile_compressed_data = rs_mem_alloc( rec.bytes_len, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( subtile_compressed_data, "rvt", __LINE__ );

    fread(subtile_compressed_data, rec.bytes_len, 1, fp );

    data_size = dest_data_size = dest_data_size_long = rec.uncompressed_bytes_len;

    DEBUGRVTSTATf("uncompressed bytes len = %d\n", data_size);


    rvt_subpak_cache_t *spc = &rvt_app_get_geodata()->subpak_cache[rvt_app_get_geodata()->subpak_cache_counter];
    if (spc->data != NULL) {
        rs_mem_free(spc->data);
    };

    spc->data = rs_mem_alloc( data_size, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( spc->data, "rvt", __LINE__ );
    spc->len = data_size;
    spc->z = subpak_z;
    spc->x = subpak_world_ix;
    spc->y = subpak_world_iy;
    spc->parts_count = hdr.parts_count;
    spc->parts_mask = hdr.parts_mask;
    spc->tiles_count_per_subpak = hdr.dest_count_per_subpak;
    spc->dest_z = tile_z;
    rvt_app_get_geodata()->subpak_cache_counter = (rvt_app_get_geodata()->subpak_cache_counter + 1) % RVT_SUBPAK_CACHE_COUNT;

    DEBUGRVTSTATf("spc: %d/%d/%d, %d tiles per subpak, parts count %d mask %d\n", spc->z, spc->x, spc->y, spc->tiles_count_per_subpak, hdr.parts_count, hdr.parts_mask);

    unsigned char *data = spc->data;


    if ( rec.compression_type == RS_COMPRESS_TYPE_NONE ) {
        memcpy(data, subtile_compressed_data, rec.uncompressed_bytes_len);
    }
    else if (rec.compression_type == RS_COMPRESS_TYPE_ZLIB) {

        int res = uncompress( (unsigned char*)(data), &dest_data_size_long, (unsigned char*)subtile_compressed_data, rec.bytes_len);

        if ((res != Z_OK) || (data_size != dest_data_size_long) ) {

            if (downloaded) {
                fclose(fp);
                fp = NULL;
                unlink(s_filename);
            };

            rs_critical_alert_and_halt_sprintf("Error while decompressing ZLIB geodata\nresult=%d, dest data size=%ld\nsubpak %d/%d/%d part_i %d, parts mask %d, parts count %d\nfilename %s\nYou may want to clear geodata cache",
                res, dest_data_size_long, spc->z, spc->x, spc->y, part_i, spc->parts_mask, spc->parts_count, s_filename);
        };

    }
    else if (rec.compression_type == RS_COMPRESS_TYPE_BZIP2) {

        int res = BZ2_bzBuffToBuffDecompress( (char*)data, &dest_data_size, (char*)subtile_compressed_data, rec.bytes_len, 0, 0 );

        if ((res != BZ_OK) || (data_size != dest_data_size) ) {
            if (downloaded) {
                fclose(fp);
                fp = NULL;
                unlink(s_filename);
            };

            rs_critical_alert_and_halt_sprintf("Error while decompressing BZ2 geodata\nresult=%d, dest data size=%ld\nsubpak %d/%d/%d part_i %d, parts mask %d, parts count %d\nfilename %s\nYou may want to clear geodata cache",
                res, dest_data_size_long, spc->z, spc->x, spc->y, part_i, spc->parts_mask, spc->parts_count, s_filename);
        };

    }
    else {

        if (downloaded) {
            fclose(fp);
            fp = NULL;
            unlink(s_filename);
        };

        rs_critical_alert_and_halt_sprintf("Unknown GPAK compression type (%d) (%d/%d/%d)\nYou may want to clear geodata cache", rec.compression_type,
            tile_z, tile_world_ix, tile_world_iy);
        return;
    };

    rs_mem_free(subtile_compressed_data);

    fclose(fp);


    if ( part_i >= spc->parts_count ) {
        // We don't have this part_i, probably out of bounds
        return;
    };


    tile_map_record_t *tmr = (tile_map_record_t*) spc->data;
    unsigned int bytes_shift = tmr[tile_j*spc->parts_count + part_i].bytes_shift;
    unsigned int bytes_len =   tmr[tile_j*spc->parts_count + part_i].bytes_len;

    DEBUGRVTSTATf("bytes_shift %d, bytes_len %d\n", bytes_shift, bytes_len);

    if (bytes_len == 0) {
        DEBUGRVTSTATf("No data for %d/%d/%d \n", tile_z, tile_world_ix, tile_world_iy );
        return;
    };


    DEBUGRVTSTATf("Data is ready: subpak len %d, bytes_shift: %d, bytes_len %d, parts_count %d, dest count per subpak %d\n", spc->len, bytes_shift, bytes_len, hdr.parts_count, hdr.dest_count_per_subpak);

    *pdata_len = bytes_len;
    *pdata = spc->data + bytes_shift;

    return;

};



#define DEBUGTAGSf  DEBUG30f


void rvt_load_tile(int tile_z, int tile_x, int tile_y, int subpak_z, int part_i, int tile_ix, int tile_iy, int tile_px, int tile_py, float tile_side_units) {


    int rvt_counter_vertices_total = rvt_reg.rvt_counter_vertices_total;

    memset(&rvt_reg, 0, sizeof(struct rvt_reg_t));

    rvt_reg.rvt_counter_vertices_total = rvt_counter_vertices_total;



    DEBUG20f("rvt_load_tile(): %d/%d/%d (sub%d) (part_i %d)...\n", tile_z, tile_x, tile_y, subpak_z, part_i);

    int tile_i = tile_iy * RVT_TILES_SIDE_COUNT + tile_ix;

    float scale = tile_side_units/256.0;

    rvt_reg.tile_z = tile_z;
    rvt_reg.tile_x = tile_x;
    rvt_reg.tile_y = tile_y;

    rvt_reg.shift_x = tile_px * tile_side_units;
    rvt_reg.shift_y = tile_py * tile_side_units;

    float geom_ox = 0.0;
    float geom_oy = 0.0;


    unsigned int data_len = 0;

    rvt_process_file(&rvt_data, &data_len, tile_z, tile_x, tile_y, subpak_z, RVT_TYPE_GEODATA, part_i/*, tile_ix, tile_iy*/);
    if (!rvt_data) {
        DEBUG20f("No rvt data. Empty tile\n");
        return;
    };


    rvt_data_pointer = rvt_data;

    char c_arr[4] = {'b', 'a', 'l', 'p'};
    char c = c_arr[part_i];


    uint32_t *points_buf = (uint32_t*) rs_mem_alloc( 4 * 8192, RS_MEM_POOL_AUTO );
    float *float_points_buf = (float*) rs_mem_alloc( 2 * 4 * 8192, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( points_buf, "rvt", __LINE__ );
    rs_app_assert_memory( float_points_buf, "rvt", __LINE__ );

    rs_shape_t *p;

    unsigned short objects_num;
    unsigned short polygons_num;
    unsigned short rings_num;
    unsigned short points_num;

    read_us(&objects_num);

    int vbo_layer_index = RVT_LAYER_BUILDING;



    DEBUGTAGSf("Processing geodata ('%c'): objects_num = %d\n", c, objects_num);

    char *s_name = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    char *s_name_user_lang = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    char *s_name_user_alt_lang = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    char *s_housenumber = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    char *s_ref = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);

    char *s_key = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    char *s_value = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);

    char *s_building_strings[RVT_BUILDING_MAX_STRINGS];
    for (int bi = 0; bi < RVT_BUILDING_MAX_STRINGS; bi++) {
       s_building_strings[bi] = (char*) rs_mem_alloc(4096, RS_MEM_POOL_AUTO);
    };

    char s_name_key_user_lang[10];
    sprintf(s_name_key_user_lang, "name:%s", rvt_app_get_lang() );
    char s_name_key_user_alt_lang[10];
    sprintf(s_name_key_user_lang, "name:%s", rvt_app_get_alt_lang() );


    for (int oi = 0; oi < objects_num; oi++) {


        s_name[0] = '\0';
        s_name_user_lang[0] = '\0';
        s_name_user_alt_lang[0] = '\0';
        s_housenumber[0] = '\0';
        s_ref[0] = '\0';
        for (int bi = 0; bi < RVT_BUILDING_MAX_STRINGS; bi++) {
            s_building_strings[bi][0] = '\0';
        };

        gd_building_data_t b_data;
        memset(&b_data, 0, sizeof(gd_building_data_t));

        gd_area_data_t a_data;
        memset(&a_data, 0, sizeof(gd_area_data_t));

        gd_lines_data_t l_data;
        memset(&l_data, 0, sizeof(gd_lines_data_t));

        gd_point_data_t p_data;
        memset(&p_data, 0, sizeof(gd_point_data_t));

        b_data.data_type = RVT_GD_LAYER_BUILDING;
        a_data.data_type = RVT_GD_LAYER_AREA;
        l_data.data_type = RVT_GD_LAYER_LINES;
        p_data.data_type = RVT_GD_LAYER_POINTS;

        b_data.building_colour = RVT_NO_COLOR;
        b_data.roof_colour = RVT_NO_COLOR;

        unsigned char tags_count = 0;

        read_byte(&tags_count);

        DEBUGTAGSf("\n\n%d tags\n", tags_count);

        for (int ti = 0; ti < tags_count; ti++) {

            char s_c = 0;

            unsigned char b_key_ref;
            read_byte(&b_key_ref);

            if (b_key_ref == 0) {

                int c_count = 0;

                do {
                    read_byte(&s_c);
                    s_key[c_count] = s_c;
                    c_count++;
                } while ( s_c != 0 );

            }
            else {
                rs_critical_alert_and_halt_sprintf("b_key_ref!=0, not implemented (%d) (%d/%d/%d)", b_key_ref, tile_z, tile_x, tile_y);
            };


            DEBUGTAGSf("%d) %s=", ti, s_key);

            unsigned char b_value_ref;
            read_byte(&b_value_ref);

            if (b_value_ref == 0) {

                int c_count = 0;

                do {
                    read_byte(&s_c);
                    s_value[c_count] = s_c;
                    c_count++;
                } while ( s_c != 0 );

            }
            else {
                rs_critical_alert_and_halt_sprintf("b_value_ref!=0, not implemented (%d/%d/%d)", tile_z, tile_x, tile_y );
            };

            DEBUGTAGSf("%s\n", s_value);

            // #tags

            if ( !strcmp(s_key, "name") ) {
                strcpy(s_name, s_value);
            };

            if ( !strcmp(s_key, s_name_key_user_lang) ) {
                strcpy(s_name_user_lang, s_value);
            };

            if ( !strcmp(s_key, s_name_key_user_alt_lang) ) {
                strcpy(s_name_user_alt_lang, s_value);
            };



            if ( !strcmp(s_key, "addr:housenumber") ) {

                strcpy(s_housenumber, s_value);

            };

            if ( !strcmp(s_key, "ref") ) {

                strcpy(s_ref, s_value);

            };


            if ( !strcmp(s_key, "building") ) {

                strcpy(s_building_strings[RVT_BUILDING_S_TYPE], s_value);

                if (!strcmp(s_value, "yes")) {
                    b_data.building = OSM_TAG_YES;
                }

                else {

                    b_data.building = rvt_get_building_type_from_string(s_value);

                };




            };

            if ( !strcmp(s_key, "building:part") ) {

                if (!strcmp(s_value, "yes")) {
                    b_data.building_part = OSM_TAG_YES;
                }
                else if (!strcmp(s_value, "no")) {
                    b_data.building_part = OSM_TAG_NO;
                };

            };

            if ( !strcmp(s_key, "RVT_RELATION_ROLE") ) {

                if (!strcmp(s_value, "outline")) {
                    b_data.rvt_relation_role = OSM_TAG_RVT_RELATION_ROLE_OUTLINE;
                };

            };



            if ( !strcmp(s_key, "building:levels") ) {
                sscanf(s_value, "%d", &b_data.b_levels);
            };

            if ( !strcmp(s_key, "height") ) {
                sscanf(s_value, "%f", &b_data.b_height);
            };
            if ( !strcmp(s_key, "building:height") ) {
                sscanf(s_value, "%f", &b_data.b_height);
            };

            if ( !strcmp(s_key, "min_height") ) {
                sscanf(s_value, "%f", &b_data.b_min_height);
            };
            if ( !strcmp(s_key, "building:min_height") ) {
                sscanf(s_value, "%f", &b_data.b_min_height);
            };

            if ( !strcmp(s_key, "roof:height") ) {
                sscanf(s_value, "%f", &b_data.roof_height);
            };




            if ( !strcmp(s_key, "building:colour") ) {
                b_data.building_colour = rvt_get_colour_from_string(s_value);

                strcpy(s_building_strings[RVT_BUILDING_S_COLOUR], s_value);

            };

            if ( !strcmp(s_key, "roof:colour") ) {
                b_data.roof_colour = rvt_get_colour_from_string(s_value);

                strcpy(s_building_strings[RVT_BUILDING_S_ROOF_COLOUR], s_value);
            };

            if ( !strcmp(s_key, "material") ) {
                strcpy(s_building_strings[RVT_BUILDING_S_MATERIAL], s_value);
            };

            if ( !strcmp(s_key, "place") ) {
                p_data.place = OSM_TAG_PLACE_YES;
            };

            if ( !strcmp(s_key, "width") ) {
                sscanf(s_value, "%f", &l_data.width);

                if (l_data.width > 75.0) {
                    l_data.width = 0.0;
                };

            };



            if ( !strcmp(s_key, "roof:shape") ) {
                if (!strcmp(s_value, "dome")) {
                    b_data.roof_shape = OSM_TAG_ROOF_SHAPE_DOME;
                }
                else if (!strcmp(s_value, "onion")) {
                    b_data.roof_shape = OSM_TAG_ROOF_SHAPE_ONION;
                }
                else if (!strcmp(s_value, "flat")) {
                    b_data.roof_shape = OSM_TAG_ROOF_SHAPE_FLAT;
                }
                else {
                    b_data.roof_shape = OSM_TAG_ROOF_SHAPE_UNKNOWN;
                }

            };



            if ( !strcmp(s_key, "aeroway") ) {
                if (!strcmp(s_value, "runway")) {
                    l_data.aeroway = OSM_TAG_AEROWAY_RUNWAY;
                }
                else if (!strcmp(s_value, "taxiway")) {
                    l_data.aeroway = OSM_TAG_AEROWAY_TAXIWAY;
                }
                else if (!strcmp(s_value, "apron")) {
                    b_data.aeroway = OSM_TAG_AEROWAY_APRON;
                    a_data.aeroway = OSM_TAG_AEROWAY_APRON;
                }
                else if (!strcmp(s_value, "terminal")) {
                    b_data.aeroway = OSM_TAG_AEROWAY_TERMINAL;
                    a_data.aeroway = OSM_TAG_AEROWAY_TERMINAL;
                }
            };



            if ( !strcmp(s_key, "waterway") ) {
                if (!strcmp(s_value, "riverbank")) {
                    a_data.waterway = OSM_TAG_WATERWAY_RIVERBANK;
                }
                else if (!strcmp(s_value, "river")) {
                    l_data.waterway = OSM_TAG_WATERWAY_RIVER;
                }
                else if (!strcmp(s_value, "stream")) {
                    l_data.waterway = OSM_TAG_WATERWAY_STREAM;
                }
            };

            if ( !strcmp(s_key, "water") ) {
                if (!strcmp(s_value, "reservoir")) {
                    a_data.water = OSM_TAG_WATER_RESERVOIR;
                }
                else if (!strcmp(s_value, "lake")) {
                    a_data.water = OSM_TAG_WATER_LAKE;
                };
            };

            if ( !strcmp(s_key, "natural") ) {

                if (!strcmp(s_value, "wood")) {
                    a_data.natural = OSM_TAG_NATURAL_WOOD;
                }
                else if (!strcmp(s_value, "water")) {
                    a_data.natural = OSM_TAG_NATURAL_WATER;
                }
                else if (!strcmp(s_value, "sand")) {
                    a_data.natural = OSM_TAG_NATURAL_SAND;
                }
                else if (!strcmp(s_value, "beach")) {
                    a_data.natural = OSM_TAG_NATURAL_BEACH;
                }
                else if (!strcmp(s_value, "grassland")) {
                    a_data.natural = OSM_TAG_NATURAL_GRASSLAND;
                }
                else if (!strcmp(s_value, "scrub")) {
                    a_data.natural = OSM_TAG_NATURAL_SCRUB;
                }
                else if (!strcmp(s_value, "heath")) {
                    a_data.natural = OSM_TAG_NATURAL_HEATH;
                }
                else if (!strcmp(s_value, "tree")) {
                    p_data.natural = OSM_TAG_NATURAL_TREE;
                }
                else if (!strcmp(s_value, "peak")) {
                    p_data.natural = OSM_TAG_NATURAL_PEAK;
                }
                else if (!strcmp(s_value, "volcano")) {
                    p_data.natural = OSM_TAG_NATURAL_VOLCANO;
                };

            };

            if ( !strcmp(s_key, "landuse") ) {

                if (!strcmp(s_value, "residential")) {
                    a_data.landuse = OSM_TAG_LANDUSE_RESIDENTIAL;
                }
                else if (!strcmp(s_value, "industrial")) {
                    a_data.landuse = OSM_TAG_LANDUSE_INDUSTRIAL;
                }
                else if (!strcmp(s_value, "commercial")) {
                    a_data.landuse = OSM_TAG_LANDUSE_COMMERCIAL;
                }
                else if (!strcmp(s_value, "quarry")) {
                    a_data.landuse = OSM_TAG_LANDUSE_QUARRY;
                }
                else if (!strcmp(s_value, "forest")) {
                    a_data.landuse = OSM_TAG_LANDUSE_FOREST;
                }
                else if (!strcmp(s_value, "grass")) {
                    a_data.landuse = OSM_TAG_LANDUSE_GRASS;
                }
                else if (!strcmp(s_value, "reservoir")) {
                    a_data.landuse = OSM_TAG_LANDUSE_RESERVOIR;
                }

            };

            if ( !strcmp(s_key, "residential") ) {
                if (!strcmp(s_value, "rural")) {
                    a_data.residential = OSM_TAG_RESIDENTIAL_RURAL;
                }
                else if (!strcmp(s_value, "urban")) {
                    a_data.residential = OSM_TAG_RESIDENTIAL_URBAN;
                }
                else {
                    a_data.residential = OSM_TAG_RESIDENTIAL_UNKNOWN;
                };
            };



            if ( !strcmp(s_key, "power") ) {

                if (!strcmp(s_value, "line")) {
                    l_data.power = OSM_TAG_POWER_LINE;
                }
                else if (!strcmp(s_value, "minor_line")) {
                    l_data.power = OSM_TAG_POWER_MINOR_LINE;
                }
                else if (!strcmp(s_value, "tower")) {
                    p_data.power = OSM_TAG_POWER_TOWER;
                }
                else if (!strcmp(s_value, "pole")) {
                    p_data.power = OSM_TAG_POWER_POLE;
                };

            };

            if ( !strcmp(s_key, "oneway") ) {
                if ( osm_value_is_non_zero(s_value) ) {
                    l_data.oneway = OSM_TAG_ONEWAY_YES;
                };
            };

            if ( !strcmp(s_key, "tunnel") ) {
                if ( osm_value_is_non_zero(s_value) ) {
                    l_data.tunnel = OSM_TAG_YES;
                };
            };

            if ( !strcmp(s_key, "lit") ) {
                if ( !strcmp(s_value, "no") ) {
                    l_data.lit = OSM_TAG_LIT_NO;
                }
                else if ( osm_value_is_non_zero(s_value) ) {
                    l_data.lit = OSM_TAG_LIT_YES;
                };
            };

            if ( !strcmp(s_key, "lanes") ) {

                l_data.lanes = 0;
                sscanf(s_value, "%d", &l_data.lanes );

            };


            if ( !strcmp(s_key, "railway") ) {

                if (!strcmp(s_value, "rail")) {
                    l_data.railway = OSM_TAG_RAILWAY_RAIL;
                }
                else if (!strcmp(s_value, "tram")) {
                    l_data.railway = OSM_TAG_RAILWAY_TRAM;
                }
                else if (!strcmp(s_value, "level_crossing")) {
                    p_data.railway = OSM_TAG_RAILWAY_LEVEL_CROSSING;
                };

            };

            if ( !strcmp(s_key, "surface") ) {

                if (!strcmp(s_value, "paved")) {
                    l_data.surface = OSM_TAG_SURFACE_PAVED;
                    a_data.surface = OSM_TAG_SURFACE_PAVED;
                }
                else if (!strcmp(s_value, "asphalt")) {
                    l_data.surface = OSM_TAG_SURFACE_ASPHALT;
                    a_data.surface = OSM_TAG_SURFACE_ASPHALT;
                }
                else if (!strcmp(s_value, "asphalt")) {
                    l_data.surface = OSM_TAG_SURFACE_CONCRETE;
                    a_data.surface = OSM_TAG_SURFACE_CONCRETE;
                }

                else if (!strcmp(s_value, "unpaved")) {
                    l_data.surface = OSM_TAG_SURFACE_UNPAVED;
                    a_data.surface = OSM_TAG_SURFACE_UNPAVED;
                }
                else if (!strcmp(s_value, "ground")) {
                    l_data.surface = OSM_TAG_SURFACE_GROUND;
                    a_data.surface = OSM_TAG_SURFACE_GROUND;
                }
                else if (!strcmp(s_value, "mud")) {
                    l_data.surface = OSM_TAG_SURFACE_MUD;
                    a_data.surface = OSM_TAG_SURFACE_MUD;
                }
                else if (!strcmp(s_value, "dirt")) {
                    l_data.surface = OSM_TAG_SURFACE_DIRT;
                    a_data.surface = OSM_TAG_SURFACE_DIRT;
                }
                else if (!strcmp(s_value, "sand")) {
                    l_data.surface = OSM_TAG_SURFACE_SAND;
                    a_data.surface = OSM_TAG_SURFACE_SAND;
                };

            };

            if ( !strcmp(s_key, "highway") ) {

                if (!strcmp(s_value, "pedestrian")) {
                    a_data.hw = OSM_TAG_HW_PEDESTRIAN;
                }

                if (!strcmp(s_value, "residential")) {
                    l_data.hw = OSM_TAG_HW_RESIDENTIAL;
                }
                else if (!strcmp(s_value, "service")) {
                    l_data.hw = OSM_TAG_HW_SERVICE;
                }
                else if (!strcmp(s_value, "tertiary_link")) {
                    l_data.hw = OSM_TAG_HW_TERTIARY_LINK;
                }
                else if (!strcmp(s_value, "tertiary")) {
                    l_data.hw = OSM_TAG_HW_TERTIARY;
                }
                else if (!strcmp(s_value, "secondary_link")) {
                    l_data.hw = OSM_TAG_HW_SECONDARY_LINK;
                }
                else if (!strcmp(s_value, "secondary")) {
                    l_data.hw = OSM_TAG_HW_SECONDARY;
                }
                else if (!strcmp(s_value, "primary_link")) {
                    l_data.hw = OSM_TAG_HW_PRIMARY_LINK;
                }
                else if (!strcmp(s_value, "primary")) {
                    l_data.hw = OSM_TAG_HW_PRIMARY;
                }
                else if (!strcmp(s_value, "motorway_link")) {
                    l_data.hw = OSM_TAG_HW_MOTORWAY_LINK;
                }
                else if (!strcmp(s_value, "motorway")) {
                    l_data.hw = OSM_TAG_HW_MOTORWAY;
                }
                else if (!strcmp(s_value, "trunk_link")) {
                    l_data.hw = OSM_TAG_HW_TRUNK_LINK;
                }
                else if (!strcmp(s_value, "trunk")) {
                    l_data.hw = OSM_TAG_HW_TRUNK;
                }


                else if (!strcmp(s_value, "track")) {
                    l_data.hw = OSM_TAG_HW_TRACK;
                }
                else if (!strcmp(s_value, "footway")) {
                    l_data.hw = OSM_TAG_HW_FOOTWAY;
                }
                else if (!strcmp(s_value, "path")) {
                    l_data.hw = OSM_TAG_HW_PATH;
                }

                else if (!strcmp(s_value, "cycleway")) {
                    l_data.hw = OSM_TAG_HW_CYCLEWAY;
                }

                else if (!strcmp(s_value, "living_street")) {
                    l_data.hw = OSM_TAG_HW_LIVING_STREET;
                }

                else if (!strcmp(s_value, "unclassified")) {
                    l_data.hw = OSM_TAG_HW_UNCLASSIFIED;
                }
                else if (!strcmp(s_value, "bridleway")) {
                    l_data.hw = OSM_TAG_HW_BRIDLEWAY;
                }

                else if (!strcmp(s_value, "bus_stop")) {
                    p_data.hw = OSM_TAG_HW_BUS_STOP;
                }
                else if (!strcmp(s_value, "crossing")) {
                    p_data.hw = OSM_TAG_HW_CROSSING;
                }
                else if (!strcmp(s_value, "traffic_signals")) {
                    p_data.hw = OSM_TAG_HW_TRAFFIC_SIGNALS;
                }

                else if (!strcmp(s_value, "give_way")) {
                    p_data.hw = OSM_TAG_HW_GIVE_WAY;
                }
                else if (!strcmp(s_value, "stop")) {
                    p_data.hw = OSM_TAG_HW_STOP;
                }
            };

            if ( !strcmp(s_key, "direction") ) {

                if (!strcmp(s_value, "forward")) {
                    p_data.direction = OSM_TAG_DIRECTION_FORWARD;
                }
                else if (!strcmp(s_value, "backward")) {
                    p_data.direction = OSM_TAG_DIRECTION_BACKWARD;
                };

            };


            if ( !strcmp(s_key, "bridge") ) {

                if (!strcmp(s_value, "yes")) {
                    l_data.bridge = OSM_TAG_YES;
                };

            };

            if ( !strcmp(s_key, "amenity") ) {

                if (!strcmp(s_value, "parking")) {
                    a_data.amenity = OSM_TAG_AMENITY_PARKING;
                };

            };

            if ( !strcmp(s_key, "leisure") ) {

                if (!strcmp(s_value, "pitch")) {
                    a_data.leisure = OSM_TAG_LEISURE_PITCH;
                }
                else if (!strcmp(s_value, "playground")) {
                    a_data.leisure = OSM_TAG_LEISURE_PLAYGROUND;
                    p_data.leisure = OSM_TAG_LEISURE_PLAYGROUND;
                }
                else if (!strcmp(s_value, "garden")) {
                    a_data.leisure = OSM_TAG_LEISURE_GARDEN;
                }
                else if (!strcmp(s_value, "park")) {
                    a_data.leisure = OSM_TAG_LEISURE_PARK;
                };

            };


            if ( !strcmp(s_key, "barrier") ) {

                if (!strcmp(s_value, "fence")) {
                    l_data.barrier = OSM_TAG_BARRIER_FENCE;
                    a_data.barrier = OSM_TAG_BARRIER_FENCE;
                }
                else if (!strcmp(s_value, "wall")) {
                    l_data.barrier = OSM_TAG_BARRIER_WALL;
                    a_data.barrier = OSM_TAG_BARRIER_WALL;
                }
                else if (!strcmp(s_value, "block")) {
                    p_data.barrier = OSM_TAG_BARRIER_BLOCK;
                };

            };


            if ( !strcmp(s_key, "entrance") ) {

                if (!strcmp(s_value, "yes")) {
                    p_data.entrance = OSM_TAG_ENTRANCE_YES;
                }
                else if (!strcmp(s_value, "main")) {
                    p_data.entrance = OSM_TAG_ENTRANCE_MAIN;
                }
                else if (!strcmp(s_value, "staircase")) {
                    p_data.entrance = OSM_TAG_ENTRANCE_STAIRCASE;
                };

            };

            if ( !strcmp(s_key, "crossing") ) {

                if (!strcmp(s_value, "uncontrolled")) {
                    p_data.crossing = OSM_TAG_CROSSING_UNCONTROLLED;
                }
                else if (!strcmp(s_value, "traffic_signals")) {
                    p_data.crossing = OSM_TAG_CROSSING_TRAFFIC_SIGNALS;
                };

            };


        };


        if (c != 'p') {

            read_us(&polygons_num);

            if (c == 'b') {
                geom_ox = -0.5 + 0.25 + 0.5 * ((polygons_num >> 15) & 0x01);
                geom_oy = -0.5 + 0.25 + 0.5 * ((polygons_num >> 14) & 0x01);
            }
            else {
                geom_ox = 0.0;
                geom_oy = 0.0;
            };

            int float_coordinates = 0;
            if (polygons_num & (1<<13) ) {
                float_coordinates = 1;
                geom_ox = geom_oy = 0.0;
            };

            int three_byte_coordinates = 0;
            uint16_t three_byte_coordinates_byte_ref = 0;
            int three_byte_coordinates_ref_x = 0;
            int three_byte_coordinates_ref_y = 0;
            if (polygons_num & (1<<12) ) {
                three_byte_coordinates = 1;
            };

            int two_byte_coordinates = 0;
            if (float_coordinates && three_byte_coordinates) {
                int two_byte_coordinates = 1;
                rs_critical_alert_and_halt_sprintf("(v2) Wrong polygons_num value (%d) (mark 1) (%d/%d/%d)", polygons_num, tile_z, tile_x, tile_y);
            };

            if (polygons_num & 0x0C00) {    // unused bytes in v2
                rs_critical_alert_and_halt_sprintf("(v2) Wrong polygons_num value (%d) (%d/%d/%d)", polygons_num, tile_z, tile_x, tile_y);
            };

            polygons_num &= 0x03FF;

            DEBUGRVTf("polygons_num = %d\n", polygons_num);


            if (three_byte_coordinates) {
                read_us(&three_byte_coordinates_byte_ref);
                three_byte_coordinates_ref_x = three_byte_coordinates_byte_ref & 0xFF;
                three_byte_coordinates_ref_y = (three_byte_coordinates_byte_ref>>8) & 0xFF;
            };


            for (int pi = 0; pi < polygons_num; pi++) {



                if (c != 'l') {
                    read_us(&rings_num);
                }
                else {
                    rings_num = 1;
                };

                DEBUGRVTf("rings_num = %d (c=%c)\n", rings_num, c);

                p = rs_shape_create(rings_num);

                float xcenter = 0.0, ycenter = 0.0;
                int center_points_num = 0;

                for (int ri = 0; ri < rings_num; ri++) {

                    read_us(&points_num);
                    DEBUGRVTf("points_num = [%d]\n", points_num);

                    if (points_num > 32767) {
                        rs_critical_alert_and_halt_sprintf("Wrong points_num, rvtloader:%d, points_num = %d (%d/%d/%d)\n", __LINE__, points_num,
                            tile_z, tile_x, tile_y);
                    };

                    if (float_coordinates) {
                        read_buf( (unsigned char*) float_points_buf, 2 * 4 * points_num);

                        rs_linestring_t *ls = rs_linestring_create(points_num);

                        rs_shape_append_ring(p, ls);

                        for (int vi = 0; vi < points_num; vi++) {
                            rs_point_t v = rs_vec2(1.0*tile_side_units*float_points_buf[vi*2+0], 1.0*tile_side_units*float_points_buf[vi*2+1] );
                            xcenter += v.x;
                            ycenter += v.y;
                            rs_linestring_append_point(ls, v);
                            DEBUG20f("(%.2f,%.2f) ", v.x, v.y);
                        };
                        DEBUG20f("\n");

                        center_points_num += points_num;

                    }

                    else if (three_byte_coordinates) {

                        read_buf( (unsigned char*) points_buf, 3 * points_num );

                        rs_linestring_t *ls = rs_linestring_create(points_num);

                        rs_shape_append_ring(p, ls);

                        unsigned char *ubyte_buf = (unsigned char*) points_buf;

                        for (int vi = 0; vi < points_num; vi++) {

                            uint32_t packed_coord = ( ((uint32_t)(ubyte_buf[vi*3+0])) << 0) | ( ((uint32_t)(ubyte_buf[vi*3+1]))  << 8) | ( ((uint32_t)(ubyte_buf[vi*3+2]))  << 16);

                            float vx = 256.0*three_byte_coordinates_ref_x + (packed_coord & 0x0FFF);
                            float vy = 256.0*three_byte_coordinates_ref_y + ((packed_coord>>12) & 0x0FFF);

                            rs_point_t v = rs_vec2( 1.0 * tile_side_units / 65536.0 * vx, 1.0 * tile_side_units / 65536.0 * vy );
                            xcenter += v.x;
                            ycenter += v.y;
                            rs_linestring_append_point(ls, v);

                        };

                        center_points_num += points_num;

                    }

                    else {


                        read_buf( (unsigned char*) points_buf, 4 * points_num);

                        rs_linestring_t *ls = rs_linestring_create(points_num);

                        rs_shape_append_ring(p, ls);

                        for (int vi = 0; vi < points_num; vi++) {
                            rs_point_t v = rs_vec2(tile_side_units*geom_ox + scale*(RVT_X(points_buf[vi])), tile_side_units*geom_oy + scale*(RVT_Y(points_buf[vi])) );

                            xcenter += v.x;
                            ycenter += v.y;
                            rs_linestring_append_point(ls, v);
                            DEBUGRVT20f("(%.2f,%.2f) ", v.x, v.y);
                        };
                        DEBUGRVT20f("\n");

                        center_points_num += points_num;

                    };


                };

                xcenter /= center_points_num;
                ycenter /= center_points_num;


                float y_start = 0.0;
                if (p->rings_count > 0) {
                    if (p->rings[0]->points_count > 0) {

                        y_start = rvt_hm_get_height_adv( xcenter/tile_side_units, ycenter/tile_side_units, tile_ix, tile_iy );

                    };
                };

                float y_start_max = -9999.0;
                for (int yri = 0; yri < p->rings_count; yri++) {
                    for (int ypi = 0; ypi < p->rings[yri]->points_count; ypi++) {
                        y_start_max = rs_max(y_start_max, rvt_hm_get_height_adv( (p->rings[yri]->p[ypi].x)/tile_side_units,
                            (p->rings[yri]->p[ypi].y)/tile_side_units, tile_ix, tile_iy ) );
                    };
                };

                if (c == 'b') {

                    b_data.cx = xcenter + rvt_reg.shift_x;
                    b_data.cy = ycenter + rvt_reg.shift_y;

                    b_data.base_height = y_start;

                    if (s_name[0]) {
                        b_data.name = (char*) rs_mem_alloc( strlen(s_name) + 1, RS_MEM_POOL_AUTO );
                        strcpy(b_data.name, s_name);
                    };

                    if (s_housenumber[0]) {
                        b_data.housenumber = (char*) rs_mem_alloc( strlen(s_housenumber) + 1, RS_MEM_POOL_AUTO );
                        strcpy(b_data.housenumber, s_housenumber);
                    };

                    for (int bi = 0; bi < RVT_BUILDING_MAX_STRINGS; bi++) {
                        if (s_building_strings[bi][0]) {
                            b_data.building_strings[bi] = (char*) rs_mem_alloc( strlen(s_building_strings[bi]) + 1, RS_MEM_POOL_AUTO );
                            strcpy(b_data.building_strings[bi], s_building_strings[bi]);
                        };
                    };

                    unsigned char *data_geom_content = rs_static_shape_data_create(p, &(b_data.content_len) );

                    gd_append( tile_i, RVT_GD_LAYER_BUILDING, sizeof(gd_building_data_t), (unsigned char*) &b_data, b_data.content_len, data_geom_content );
                    rs_mem_free(data_geom_content);


                }
                else if (c == 'a') {

                    a_data.cx = xcenter + rvt_reg.shift_x;
                    a_data.cy = ycenter + rvt_reg.shift_y;

                    if (s_name[0]) {
                        a_data.name = (char*) rs_mem_alloc( strlen(s_name) + 1, RS_MEM_POOL_AUTO );
                        strcpy(a_data.name, s_name);
                    };

                    unsigned char *data_geom_content = rs_static_shape_data_create(p, &(a_data.content_len) );
                    gd_append( tile_i, RVT_GD_LAYER_AREA, sizeof(gd_area_data_t), (unsigned char*) &a_data, a_data.content_len, data_geom_content );
                    rs_mem_free(data_geom_content);

                }
                else if (c == 'l') {

                    l_data.base_height = y_start_max;

                    if (s_name[0]) {
                        if (s_ref[0]) {
                            l_data.name = (char*) rs_mem_alloc( strlen(s_name) + 1 + strlen(s_ref) + 1, RS_MEM_POOL_AUTO );
                            sprintf(l_data.name, "%s %s", s_ref, s_name);
                        }
                        else {
                            l_data.name = (char*) rs_mem_alloc( strlen(s_name) + 1, RS_MEM_POOL_AUTO );
                            strcpy(l_data.name, s_name);
                        };
                    };

                    if (l_data.lanes) {
                        if ( (l_data.lanes > 7) && (l_data.hw == OSM_TAG_HW_SERVICE) ) {
                            l_data.lanes = 0;
                        }
                        else if ( (l_data.lanes > 8) && (l_data.hw == OSM_TAG_HW_RESIDENTIAL) ) {
                            l_data.lanes = 0;
                        }
                        else if ( (l_data.lanes > 12) ) {
                            l_data.lanes = 0;
                        };

                    };

                    unsigned char *data_geom_content = rs_static_shape_data_create(p, &(l_data.content_len) );
                    gd_append( tile_i, RVT_GD_LAYER_LINES, sizeof(gd_lines_data_t), (unsigned char*) &l_data, l_data.content_len, data_geom_content );
                    rs_mem_free(data_geom_content);

                };

                rs_shape_destroy(p);

            }

        }
        else {

            // Points

            uint32_t coord;

            read_int(&coord);


            rs_vec2_t v = rs_vec2(scale*(RVT_X(coord)), scale*(RVT_Y(coord)) );


            if (s_name[0]) {
                p_data.name = (char*) rs_mem_alloc( strlen(s_name) + 1, RS_MEM_POOL_AUTO );
                strcpy(p_data.name, s_name);
            };

            if (s_name_user_lang[0]) {
                p_data.name_user_lang = (char*) rs_mem_alloc( strlen(s_name_user_lang) + 1, RS_MEM_POOL_AUTO );
                strcpy(p_data.name_user_lang, s_name_user_lang);
            };

            if (s_name_user_alt_lang[0]) {
                p_data.name_user_alt_lang = (char*) rs_mem_alloc( strlen(s_name_user_alt_lang) + 1, RS_MEM_POOL_AUTO );
                strcpy(p_data.name_user_alt_lang, s_name_user_alt_lang);
            };

            unsigned char *data_geom_content = (unsigned char*) &v;
            p_data.content_len = sizeof(rs_vec2_t);
            gd_append( tile_i, RVT_GD_LAYER_POINTS, sizeof(gd_point_data_t), (unsigned char*) &p_data, p_data.content_len, data_geom_content );

        };

    };

    DEBUG20f("... %d/%d/%d: total %d objects \n", tile_z, tile_x, tile_y, objects_num);


    rs_mem_free(s_name);
    rs_mem_free(s_name_user_lang);
    rs_mem_free(s_name_user_alt_lang);

    rs_mem_free(s_housenumber);
    rs_mem_free(s_key);
    rs_mem_free(s_value);
    rs_mem_free(s_ref);

    for (int bi = 0; bi < RVT_BUILDING_MAX_STRINGS; bi++) {
       rs_mem_free(s_building_strings[bi]);
    };
    rs_mem_free(points_buf);



};


// ---------- #heightmap -------------


typedef struct rs_hm_vertex_t {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;

            // Normal
            float nx;
            float ny;
            float nz;
            float nw;

            // Tangent
            float tx;
            float ty;
            float tz;
            float tw;

            float u;
            float v;
            float p;
            float q;
        };
        float data[16];
    };
} rs_hm_vertex_t;




void heightmap_load_tile_to_data(unsigned char **output_super_tile_data, unsigned int *output_super_tile_data_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int vbo_index, int tile_ix, int tile_iy, int tile_x, int tile_y) {

    int super_tile_z = 11;

    int z_diff = tile_z - super_tile_z;

    int super_tile_world_ix = tile_world_ix >> z_diff;
    int super_tile_world_iy = tile_world_iy >> z_diff;


	int quads_count = RVT_HEIGHTMAP_TILE_SIDE_SIZE - 1;

    float scale_z = RVT_SC_HEIGHTMAP_Z;

    unsigned char *super_tile_data = NULL;


    unsigned int super_tile_data_len = 0;

    if ( ! (rvt_settings.flat_terrain) ) {
        rvt_process_file(&super_tile_data, &super_tile_data_len, super_tile_z, super_tile_world_ix, super_tile_world_iy, subpak_z, RVT_TYPE_HEIGHTMAP, 0);
    };


    if (super_tile_data == NULL) {
        // No heightmap
        DEBUG10f("No heightmap for %d/%d/%d, using flat terrain.\n", tile_z, tile_world_ix, tile_world_iy);

        super_tile_data_len = 259*259*2; // (quads_count+1+2) * (quads_count+1+2) * 2;
        super_tile_data = rs_mem_alloc( super_tile_data_len, RS_MEM_POOL_AUTO );
        memset(super_tile_data, 0, super_tile_data_len);

    };

    *output_super_tile_data = super_tile_data;
    *output_super_tile_data_len = super_tile_data_len;

};

void heightmap_load_tile_from_data(unsigned char *super_tile_data, unsigned int super_tile_data_len, int tile_z, int tile_world_ix, int tile_world_iy, int subpak_z, int vbo_index, int tile_ix, int tile_iy, int tile_x, int tile_y) {

    RS_UNUSED_PARAM(subpak_z);


    int super_tile_z = 11;

    int z_diff = tile_z - super_tile_z;

    int super_tile_world_ix = tile_world_ix >> z_diff;
    int super_tile_world_iy = tile_world_iy >> z_diff;


	int quads_count = RVT_HEIGHTMAP_TILE_SIDE_SIZE - 1;

    float scale_z = RVT_SC_HEIGHTMAP_Z;

    int image_width = 259;

    if (super_tile_data_len != (2*image_width*image_width)) {
        rs_critical_alert_and_halt_sprintf("super tile data len = %d, but 2*259*259 expected\n%d/%d/%d", super_tile_data_len,
            tile_z, tile_x, tile_y);
    };

    int tile_data_len = 2 * 35 * 35;
    unsigned char *tile_data = rs_mem_alloc( 2 * 35 * 35, RS_MEM_POOL_AUTO );

    int image_pos_x = 32 * ( tile_world_ix - (super_tile_world_ix<<z_diff) );
    int image_pos_y = 32 * ( tile_world_iy - (super_tile_world_iy<<z_diff) );

    for (int i = 0; i < 35; i++) {
        memcpy( &tile_data[ i*2*35 ], &super_tile_data[ 2*( (image_pos_y+i)*image_width + image_pos_x ) ], 2*35 );
    };


    if (tile_data_len != (quads_count+1+2) * (quads_count+1+2) * 2) {
        rs_critical_alert_and_halt_sprintf("tile_data_len is %d instead of %d\n%d/%d/%d", tile_data_len, (quads_count+1+2) * (quads_count+1+2) * 2,
            tile_z, tile_x, tile_y);
        return;
    };


    signed short *f = (signed short*) tile_data;


    const signed short nodata_value = 16384;

    int image_side = quads_count + 1 + 2;

    for (int i = 0; i < image_side * image_side; i++) {

        // Not perfect, but it works
        f[i] = f[i] << 1;
        f[i] = f[i] >> 1;

        if (f[i] == nodata_value) {
            f[i] = 0;
        };

    };


    float *hm = (float*) rs_mem_alloc( 4 * (quads_count+1) * (quads_count+1), RS_MEM_POOL_AUTO );

    rs_app_assert_memory( hm, "rvt", __LINE__ );
    memset(hm, 0, 4 * (quads_count+1) * (quads_count+1) );


    for (int i = 0; i < quads_count + 1; i++) {
        for (int j = 0; j < quads_count + 1; j++) {
            hm[ i*(quads_count+1) + j ] =
                scale_z * f[ i*(quads_count+1+2) + j + 1 ];
        };
    };

    int tile_i = (tile_iy) * RVT_TILES_SIDE_COUNT + (tile_ix);
    memcpy( rvt_app_get_geodata()->hm[tile_i].data, hm, 4 * (quads_count+1) * (quads_count+1) );



    // ----------------

    int geom_divider_k = 1; // Should be 1. Values other than 1 may result wrong normals.


    int geom_quads_count = quads_count / geom_divider_k;
    float *geom_hm = (float*) rs_mem_alloc( 4 * (geom_quads_count+1) * (geom_quads_count+1), RS_MEM_POOL_AUTO );
    rs_app_assert_memory( geom_hm, "rvt", __LINE__ );
    memset(geom_hm, 0, 4 * (geom_quads_count+1) * (geom_quads_count+1) );

    for (int i = 0; i < geom_quads_count + 1; i++) {
        for (int j = 0; j < geom_quads_count + 1; j++) {

            int hm_i = i*geom_divider_k;
            int hm_j = j*geom_divider_k;

            geom_hm[ i*(geom_quads_count+1) + j ] =
                hm[ hm_i*(quads_count+1) + hm_j ];
        };
    };

    signed short *geom_f = (signed short *) rs_mem_alloc( (geom_quads_count+1+2) * (geom_quads_count+1+2) * 2, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( geom_f, "rvt", __LINE__ );


    for (int i = 0; i < geom_quads_count + 1 + 2; i++) {
        int f_i = i > 0 ? (1 + (i-1)*geom_divider_k) : 0;
        f_i = rs_min_i(f_i, quads_count + 1 + 2 - 1);


        for (int j = 0; j < geom_quads_count + 1 + 2; j++) {

            int f_j = j > 0 ? (1 + (j-1)*geom_divider_k) : 0;

            f_j = rs_min_i(f_j, quads_count + 1 + 2 - 1);

            geom_f[ i*(geom_quads_count+1+2) + j ] = f[ f_i*(quads_count+1+2) + f_j ];

        };
    };




	float scale = rvt_app_get_sc();

    float vert_xstart = scale * tile_x;
    float vert_ystart = scale * tile_y;

    int vertex_size = sizeof(rs_hm_vertex_t);

    int data_len = vertex_size * (2 + 2*(geom_quads_count+1) ) * (geom_quads_count+1);

    scale /= geom_quads_count;


    int  ishift;

    rs_hm_vertex_t *data = (rs_hm_vertex_t*) rs_mem_alloc( data_len, RS_MEM_POOL_AUTO );
    rs_app_assert_memory( data, "rvt", __LINE__ );
    memset(data, 0, data_len);



    if (!data) {
        DEBUG10f("ERROR, Can't allocate %d bytes (%d kb) \n", data_len, data_len/1024);
    };

    memset(data, 0, data_len);


    for (int i = 0; i < geom_quads_count; i++) {

        ishift = ( 2*(geom_quads_count+1) +2) * i;


        for (int j = 0; j < geom_quads_count+1; j++) {

            data[ ishift + (j*2) + 0 ] = (rs_hm_vertex_t) {{{ vert_xstart + scale*(j) , geom_hm[ (i+0)*(geom_quads_count+1) + j ], vert_ystart + scale*i , 1.0,
                        0.0, 0.0, 0.0, 1.0,
                        0.0, 0.0, 1.0, 1.0,
                        1.0, 0.0, 0.0, 0.0  }}};


            data[ ishift + (j*2) + 1 ] = (rs_hm_vertex_t) {{{ vert_xstart + scale*(j) , geom_hm[ (i+1)*(geom_quads_count+1) + j ], vert_ystart + scale*(i+1)  , 1.0,
                        0.0, 0.0, 0.0, 1.0,
                        0.0, 0.0, 1.0, 1.0,
                        1.0, 0.0, 0.0, 0.0  }}};


        };

        int j = geom_quads_count+1;



        data[ ishift + (j*2) + 0 ] = (rs_hm_vertex_t) {{{ vert_xstart + scale*(j-1) , geom_hm[ (i+1)*(geom_quads_count+1) + j-1], vert_ystart + scale*(i+1) , 1.0,
                    0.0, 0.0, 0.0, 1.0,
                    0.0, 0.0, 1.0, 1.0,
                    1.0, 0.0, 0.0, 0.0  }}};

        data[ ishift + (j*2) + 1 ] = (rs_hm_vertex_t) {{{ vert_xstart + scale*(0) , geom_hm[ (i+1)*(geom_quads_count+1) + 0], vert_ystart + scale*(i+1) , 1.0,
                    0.0, 0.0, 0.0, 1.0,
                    0.0, 0.0, 1.0, 1.0,
                    1.0, 0.0, 0.0, 0.0  }}};


    };


    // Normals

    scale_z /= geom_divider_k;


    for (int geom_i = 0; geom_i < geom_quads_count; geom_i++) {

        int i = geom_i;


        for (int geom_j = 0; geom_j < geom_quads_count+1; geom_j++) {

            int j = geom_j;

            for (int k = 0; k < 2; k++) {


                int y = i + k;
                int x = j;

                int dxp = (x < quads_count) ? geom_divider_k : 1;
                int dyp = (y < quads_count-1) ? geom_divider_k : 1;
                int dxm = (x > 0) ? geom_divider_k : 1;
                int dym = (y > 1) ? geom_divider_k : 1;

                dxp = dyp = dxm = dym = 1;

                float h =     geom_f[ (1+y+0)*(geom_quads_count+1+2) + (x+0) + 1 ];

                float h_dxp = geom_f[ (1+y+  0)*(geom_quads_count+1+2) + (x+dxp) + 1 ] - h;
                float h_dyp = geom_f[ (1+y+dyp)*(geom_quads_count+1+2) + (x+  0) + 1 ] - h;
                float h_dxm = geom_f[ (1+y+  0)*(geom_quads_count+1+2) + (x-dxm) + 1 ] - h;
                float h_dym = geom_f[ (1+y-dym)*(geom_quads_count+1+2) + (x+  0) + 1 ] - h;

                float h_ddxp = geom_f[ (1+y+dyp)*(geom_quads_count+1+2) + (x+dxp) + 1 ] - h;
                float h_ddyp = geom_f[ (1+y-dym)*(geom_quads_count+1+2) + (x+dxp) + 1 ] - h;
                float h_ddxm = geom_f[ (1+y+dyp)*(geom_quads_count+1+2) + (x-dxm) + 1 ] - h;
                float h_ddym = geom_f[ (1+y-dym)*(geom_quads_count+1+2) + (x-dxm) + 1 ] - h;

                float vsc = rvt_app_get_sc() / 128;
                rs_vec3_t np = rs_vec3_cross(
                                            rs_vec3(0.0, scale_z*h_dyp, vsc),
                                            rs_vec3(vsc, scale_z*h_dxp, 0.0)
                                            );
                rs_vec3_t nm = rs_vec3_cross(
                                            rs_vec3(0.0, scale_z*h_dym, -vsc),
                                            rs_vec3(-vsc, scale_z*h_dxm, 0.0)
                                            );

                rs_vec3_t npd = rs_vec3_cross(
                                            rs_vec3(vsc, scale_z*h_ddxp, vsc),
                                            rs_vec3(vsc, scale_z*h_ddyp, -vsc)
                                            );
                rs_vec3_t nmd = rs_vec3_cross(
                                            rs_vec3(-vsc, scale_z*h_ddym, -vsc),
                                            rs_vec3(-vsc, scale_z*h_ddxm, vsc)
                                            );

                rs_vec3_t n1 = rs_vec3_normalize( rs_vec3_add(nm, np) );
                rs_vec3_t nd = rs_vec3_mult(rs_vec3_normalize( rs_vec3_add(nmd, npd) ), M_SQRT2);
                rs_vec3_t n = rs_vec3_normalize( rs_vec3_add(n1, nd));


                ishift = ( 2*(geom_quads_count+1) +2) * geom_i;
                int di = ishift + (geom_j*2) + k;

                data[ di ].nx = n.x;
                data[ di ].ny = n.y;
                data[ di ].nz = n.z;

            };

        };

    };

    rs_mem_free(hm);

    rs_mem_free(geom_hm);
    rs_mem_free(geom_f);


    int data_count =  (2 + 2*geom_quads_count +2) * geom_quads_count - 2; // -2 for last
    int stride = 16;


    if ( ( rvt_app_is_exporting() ) && (rvt_settings.layer_enabled[0]) ) {

        int layer_index = 0;

        if (rvt_settings.separate_to_tiles) {
            rvt_export_file_init( &rvt_settings.rvt_file_arr[layer_index], rvt_settings.file_format, rvt_settings.z_up, RVT_EXPORT_V_TYPE_16,
                tile_z, tile_world_ix, tile_world_iy, layer_index );
        }
        else {
            if (rvt_settings.rvt_file_arr[layer_index].fp == NULL) {
                rvt_export_file_init(&rvt_settings.rvt_file_arr[layer_index], rvt_settings.file_format, rvt_settings.z_up, RVT_EXPORT_V_TYPE_16, 0, 0, 0, layer_index);
            };
        };

        uint32_t *ind_data = (uint32_t*) rs_mem_alloc( (data_count*6/4 + 6) * 4, RS_MEM_POOL_AUTO );


        int ind_count = 0;

        for (int i = 0; i < data_count/2; i++) {

            // Triangle strip
            ind_data[ind_count + 0] = i*2 + 0;
            ind_data[ind_count + 1] = i*2 + 1;
            ind_data[ind_count + 2] = i*2 + 2;
            ind_data[ind_count + 3] = i*2 + 2;
            ind_data[ind_count + 4] = i*2 + 1;
            ind_data[ind_count + 5] = i*2 + 3;

            ind_count += 6;

            // Skip degenerate triangles between stripes
            if ( ! (( (ind_count) /6) % quads_count) ) {
                i += 2;
            };

        };

        rvt_export_file_write(&rvt_settings.rvt_file_arr[layer_index], (float*) data, data_count, ind_data, ind_count );

        rs_mem_free(ind_data);

        rvt_app_get_exporting_struct()->exporting_total_bytes += rvt_settings.rvt_file_arr[layer_index].total_bytes;

        rvt_settings.rvt_file_arr[layer_index].total_bytes = 0;

        if (rvt_settings.separate_to_tiles) {
            rvt_export_file_term( &rvt_settings.rvt_file_arr[layer_index] );
        };

    };


    rvt_app_create_terrain_vbo( vbo_index, (unsigned char*) data, data_count, stride );

    rs_mem_free(data);

    rs_mem_free(tile_data);


};


