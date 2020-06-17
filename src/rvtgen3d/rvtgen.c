#include "rvtgen.h"

#include "main.h"

#include <math.h>

#include <string.h>

#include "rs/rsnoise.h"

#include "rs/rsgeom.h"

#include "rs/rsboost.h"

#include "rvtapp.h"
#include "rvtutil.h"


void rvt_full_tile_naturals(void *geometry, int stage_i, void *data) {

    rs_triangle_t tree_tr[2];

    tree_tr[0].p[0] = rs_vec2(0.0, 0.0);
    tree_tr[0].p[1] = rs_vec2(0.0, 1.0*rvt_app_get_sc());
    tree_tr[0].p[2] = rs_vec2(1.0*rvt_app_get_sc(), 1.0*rvt_app_get_sc());

    tree_tr[1].p[0] = rs_vec2(1.0*rvt_app_get_sc(), 1.0*rvt_app_get_sc());
    tree_tr[1].p[1] = rs_vec2(1.0*rvt_app_get_sc(), 0.0);
    tree_tr[1].p[2] = rs_vec2(0.0, 0.0);


    for (int ti = 0; ti < 2; ti++) {

        rs_point_set_t *tree_ps;
        if (ti == 0) {
            tree_ps = rs_point_set_create_from_quad_random_islands(45, 17, rvt_app_get_sc()/20.0, rvt_app_get_sc(), 0);
        }
        else {
            tree_ps = rs_point_set_create_from_quad_random_islands(110, 2, rvt_app_get_sc()/40.0, rvt_app_get_sc(), 0);
        };

        rvt_set_writing_layer( RVT_LAYER_NATURAL );

        for (int pi = 0; pi < tree_ps->points_count; pi++) {
            rs_vec2_t trp = tree_ps->p[pi].p;
            rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

            if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_WATER, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                continue;
            };

            if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_BUILDINGS, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                continue;
            };

            if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_GREEN_NATURALS, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_GRASSLAND,  NULL ) ) {
                continue;
            };

            if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_ASPHALT, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE,  NULL ) ) {
                continue;
            };

            rvt_gline_t *closest_gline = NULL;
            float closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE,  &closest_gline );
            if (closest_gline) {
                if (closest_distance < 0.4) {
                    continue;
                };
            };

            // #tree
            rvt_color( 0.051+ 0.19/5 + 0.02*(pi%6), 0.051+ 0.26/5 + 0.015*(pi%3), 0.051+ 0.18/5 + 0.017*(pi%5));

            if (rvt_app_get_visstyle_index() == VISSTYLE_INDEX_WINTER) {
                rvt_color(0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6));
            };

            float scale = 0.33*(5.0+(pi%3));
            float scale_h = 0.8*scale;

            if (pi < tree_ps->points_count/3) {
                // #tree
                rvt_set_writing_igroup( RVT_IGROUP_ANGLE0 );
                scale *= 0.7;
                scale_h *= 0.8;

                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.5*(pi%2), 0.1 + (pi%2),  VBODATA_TREE1 );

            }
            else {
                // #bush
                rvt_set_writing_igroup( RVT_IGROUP_DETAILS );
                scale *= 0.5;
                scale_h *= 0.21;
                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.5*(pi%2), 0.1 + (pi%2),  VBODATA_BUSH1 );
            };


        };

        rs_point_set_destroy(tree_ps);

    };

};

void rvt_road_lamps(rs_shape_t *p, int stage_i,  gd_lines_data_t *sdata) {

    if (sdata->tunnel) {
        return;
    };

    if (!sdata->hw) {
        return;
    };

    if (sdata->bridge) {
        return;
    };

    if ( (sdata->hw != OSM_TAG_HW_FOOTWAY) && (sdata->lit != OSM_TAG_LIT_YES) ) {
        return;
    };

    float road_width = 0.750;

    int lanes = sdata->lanes;

    road_width = 0.375 * lanes;


    int vbodata_index = VBODATA_STREET_LAMP2;
    float wire_height = 2.2;
    float wire_thickness = 0.8;
    float lamp_distance = 11.0;
    float lamp_side_shift = road_width + 0.7;
    int sides = 2;

    float scale = 0.075;
    float scale_h = 0.075;

    if (sdata->hw == OSM_TAG_HW_FOOTWAY) {
        vbodata_index = VBODATA_STREET_LAMP1_FOOTWAY;
        wire_height = 1.0;
        wire_thickness = 0.56;
        lamp_distance = 3.9;
        sides = 1;

        scale = 0.18;
        scale_h = 0.092;
    };

    rvt_color(0.4, 0.4, 0.4);

    rvt_set_writing_layer( RVT_LAYER_PROP );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    for (int side_i = 0; side_i < sides; side_i++) {

        float sign = 1.0 - 2.0*side_i;

        for (int ri = 0; ri < p->rings_count; ri++) {

            rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[ri], lamp_distance, sign * lamp_side_shift, 0.0 + M_PI*side_i, 0 );

            int skip_wire = 0;

            for (int pi = 0; pi < ps->points_count; pi++ ) {

                rs_point_t trp = ps->p[pi].p;

                rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                rvt_gline_t *closest_gline = NULL;
                float closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE,  &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 0.1) {
                        skip_wire = 1;
                        continue;
                    };
                };

                rvt_set_writing_layer( RVT_LAYER_PROP );
                rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );
                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM , rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale, scale_h, scale), -M_PI_2 + ps->p[pi].azimuth, 0.25*(pi%4), 0.1 + (pi%2),
                   vbodata_index );

                rvt_set_writing_layer( RVT_LAYER_WIRE );
                rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

                if ( (pi > 1) && (!skip_wire) ) {
                    rs_angled_point_t ap = ps->p[pi];
                    rs_angled_point_t ap_prev = ps->p[pi-1];
                    float ap_distance = rs_vec2_distance(ps->p[pi].p, ap_prev.p);
                    float azimuth = atan2( ap_prev.p.y - ap.p.y, ap_prev.p.x - ap.p.x );

                    rvt_write_wire( rs_vec3(ap.p.x, wire_height, ap.p.y), rs_vec3(ap_prev.p.x, wire_height, ap_prev.p.y), wire_thickness, 0.4*wire_height, RVT_GEOM_FLAG_ADD_HM );

                };

                skip_wire = 0;

            };

            rs_point_set_destroy(ps);

        };

    };


};

void rvt_road_footway_trees(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata) {

    if (sdata->hw != OSM_TAG_HW_FOOTWAY) {
        return;
    };

    if (sdata->bridge) {
        return;
    };

    float road_width = 0.750;

    int lanes = sdata->lanes;

    road_width = 0.375 * lanes;


    int vbodata_index = VBODATA_TREE1;
    float lamp_distance = 7.0;
    float lamp_side_shift = road_width + 0.9;
    int sides = 2;

    float scale = 0.075;
    float scale_h = 0.075;


    rvt_color(0.4, 0.4, 0.4);

    rvt_set_writing_layer( RVT_LAYER_NATURAL );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    for (int side_i = 0; side_i < sides; side_i++) {

        float sign = 1.0 - 2.0*side_i;

        for (int ri = 0; ri < p->rings_count; ri++) {


            rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[ri], lamp_distance, sign * lamp_side_shift, 0.0 + M_PI*side_i, POINT_SET_FLAG_RANDOMIZER_TYPE_SIMPLE );


            for (int pi = 0; pi < ps->points_count; pi++ ) {

                rs_point_t trp = ps->p[pi].p;

                rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                rvt_gline_t *closest_gline = NULL;
                float closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE,  &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 0.1) {
                        continue;
                    };
                };

                if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_WATER, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                    continue;
                };

                if (rvt_point_is_in_area( trp_global_pos, RVT_GTRIANGLE_LAYER_BUILDINGS, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                    continue;
                };


                // #tree
                rvt_color( 0.051+ 0.19/5 + 0.02*(pi%6), 0.051+ 0.26/5 + 0.015*(pi%3), 0.051+ 0.18/5 + 0.017*(pi%5));

                if (rvt_app_get_visstyle_index() == VISSTYLE_INDEX_WINTER) {
                    rvt_color(0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6));
                };

                float scale = 0.33*(5.0+(pi%3));
                float scale_h = 0.8*scale;

                if ((pi+side_i)%3) {
                    // #tree
                    rvt_set_writing_igroup( RVT_IGROUP_ANGLE0 );
                    scale *= 0.7;
                    scale_h *= 0.8;

                    rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi+side_i, 0.5*(pi%2), 0.1 + (pi+side_i)%2,  VBODATA_TREE1 );

                }
                else {
                    // #bush
                    rvt_set_writing_igroup( RVT_IGROUP_DETAILS );
                    scale *= 0.5;
                    scale_h *= 0.21;
                    rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.5*( (side_i+pi)%2), 0.1 + (pi%2),  VBODATA_BUSH1 );
                };


            };

            rs_point_set_destroy(ps);

        };

    };

};



void rvt_road_rural_houses(rs_shape_t *p, int stage_i,  gd_lines_data_t *sdata) {

    if ( (sdata->hw != OSM_TAG_HW_TERTIARY) && (sdata->hw != OSM_TAG_HW_RESIDENTIAL) && (sdata->hw != OSM_TAG_HW_LIVING_STREET) ) {
        return;
    };

    int residential_rural_required = 0;
    if (sdata->hw == OSM_TAG_HW_TERTIARY) {
        residential_rural_required = 1;
    };

    if (sdata->bridge) {
        return;
    };


    float road_width = 0.750;
    int lanes = sdata->lanes;
    road_width = 0.375 * lanes;

    int random_i = (( ((intptr_t)sdata) & 0xFFFF )) / 8;


    float lamp_distance = 8.0;
    float lamp_side_shift = road_width + 2.0;
    int sides = 2;

    float scale = 0.8;
    float scale_h = scale;


    const int total_colors = 12;

    float colors[] = {

        0.90, 0.90, 0.99,
        0.23, 0.23, 0.27,

        // brown
        0.66, 0.27, 0.07,
        0.36, 0.14, 0.02,
        0.46, 0.21, 0.02,

        // red
        0.87, 0.15, 0.07,
        0.65, 0.21, 0.11,
        0.77, 0.04, 0.04,

        // gray
        0.60, 0.50, 0.40,
        0.80, 0.80, 0.80,
        0.57, 0.51, 0.34,
        0.40, 0.35, 0.30,

    };

    // !!! there is a copy in rvt_building()

    float roof_colors[] = {

        // green
        0.03, 0.61, 0.37,

        // pink (gray)
        0.70, 0.57, 0.70,

        // gray
        0.46, 0.46, 0.49,
        0.86, 0.86, 0.90,
        0.70, 0.75, 0.90,
        0.70, 0.75, 0.90,

        // red
        0.82, 0.33, 0.11,
        0.77, 0.44, 0.44,
        0.66, 0.05, 0.03,
        0.98, 0.20, 0.17,

        // blue
        0.35, 0.41, 0.98,
        0.55, 0.65, 0.99,

    };


    rvt_set_writing_layer( RVT_LAYER_PROP );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    for (int side_i = 0; side_i < sides; side_i++) {

        float sign = 1.0 - 2.0*side_i;



        for (int ri = 0; ri < p->rings_count; ri++) {

            rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[ri], lamp_distance, sign * lamp_side_shift, 0.0 + M_PI*side_i, 0 );

            float last_shift_forward = 0.0;


            int skip_wire = 0;

            int repeat_with_pos_closer_to_road = 0;

            for (int pi = 0; pi < ps->points_count; pi++ ) {

                rs_point_t trp = ps->p[pi].p;

                float f1 = rs_noise(ri*50 + pi, 100);
                float f2 = rs_noise(ri*73 + pi, 200);

                float rand_shift_side = 2.0 + 4.5 - f1*4.5;
                float rand_shift_forward = 2.0*f2;
                float max_side_wall = rand_shift_side + 5.0;

                int narrow_house = 0;

                if (repeat_with_pos_closer_to_road) {
                    rand_shift_side = 2.0 + 1.5 - f1*1.5;
                    rand_shift_forward = 1.5*f2;
                    max_side_wall = 2.0 + rand_shift_side;
                    narrow_house = 1;
                };

                trp.x += rand_shift_side * sin( -ps->p[pi].azimuth );
                trp.y += rand_shift_side * cos( -ps->p[pi].azimuth );

                trp.x += rand_shift_forward * cos( -ps->p[pi].azimuth );
                trp.y += rand_shift_forward * sin( -ps->p[pi].azimuth );

                if (pi > 0) {
                    last_shift_forward = rand_shift_forward;
                };

                rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                if (!repeat_with_pos_closer_to_road) {
                    float narrow_area_factor = rvt_near_glines_length(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, 0, rand_shift_side );
                    if (narrow_area_factor > 10.0) {
                        repeat_with_pos_closer_to_road = 1;
                        pi--;
                        continue;
                    };
                };


                if (rvt_gp_distance_to_nearest(trp_global_pos, RVT_GP_LAYER_BUILDING, RVT_GP_FLAG_GENERATED_BUILDING, 0) < 0.6*lamp_distance ) {
                    continue;
                };


                repeat_with_pos_closer_to_road = 0;


                rvt_gtriangle_t *gtriangle;
                int residential_rural = 0;
                int is_in_residential_area = rvt_point_is_in_area(trp_global_pos, RVT_GTRIANGLE_LAYER_LANDUSE, RVT_GTRIANGLE_FLAG_RESIDENTIAL_ANY, 0, &gtriangle);
                if (is_in_residential_area) {
                    if (gtriangle->flags & RVT_GTRIANGLE_FLAG_RESIDENTIAL_URBAN) {
                        continue;
                    }
                    else if (gtriangle->flags & RVT_GTRIANGLE_FLAG_RESIDENTIAL_RURAL) {
                        residential_rural = 1;
                    };
                };

                if ( residential_rural_required && (!residential_rural) ) {
                    continue;
                };

                float residential_roads_length_nearby = rvt_near_glines_length(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_HW_RESIDENTIAL, 0, 48.0 );

                float higher_roads_length_nearby =
                    1.7*rvt_near_glines_length(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_HW_SECONDARY_OR_HIGHER, 0, 31.0 );
                    + 1.0*rvt_near_glines_length(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_HW_TERTIARY, 0, 24.0 );


                if (rvt_gp_distance_to_nearest(trp_global_pos, RVT_GP_LAYER_BUILDING, RVT_GP_FLAG_ANY, RVT_GP_FLAG_GENERATED_BUILDING) < 14.0 ) { // 8.0
                    continue;
                };

                if ( residential_rural ) {
                    // No reasons to break this iteration if we are in legit rural area
                }
                else {
                    if ( (residential_roads_length_nearby < 60.0) && (!is_in_residential_area) && (higher_roads_length_nearby > 20.0) ) {
                        continue;
                    };

                    // large building nearby, probably detailed uban residental area
                    float gg1 = rvt_gp_distance_to_nearest(trp_global_pos, RVT_GP_LAYER_BUILDING, RVT_GP_FLAG_LARGE_BUILDING, RVT_GP_FLAG_GENERATED_BUILDING);
                    if (gg1 < 90.0 ) {
                        continue;
                    };

                };

                rvt_gline_t *closest_gline = NULL;
                float closest_distance;
                closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 1.1) {
                        continue;
                    };
                };

                closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_HW_TERTIARY|RVT_GLINE_FLAG_HW_SECONDARY_OR_HIGHER, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 2.1) {
                        continue;
                    };
                };

                closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_PATH, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 0.5) {
                        continue;
                    };
                };

                closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_RAILWAY, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 1.1) {
                        continue;
                    };
                };

                closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_RIVER, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 2.1) {
                        continue;
                    };
                };


                int vbodata_index = VBODATA_HOUSE01 + (random_i + ri + pi + side_i*3) % 4;

                int color_index = 3 * ( (random_i/2 + ri/2 + pi + side_i*5) % total_colors );
                int roof_color_index = 3 * ( (random_i/3 + ri/2 + pi*7 + side_i*3 + 3) % total_colors );

                rvt_color( roof_colors[ roof_color_index+0 ], roof_colors[ roof_color_index+1 ], roof_colors[ roof_color_index+2 ] );
                rvt_color_alt( colors[ color_index+0 ], colors[ color_index+1 ], colors[ color_index+2 ] );

                if (rvt_app_get_visstyle_index() == VISSTYLE_INDEX_WINTER) {
                    rvt_color( 0.7+0.3*roof_colors[ roof_color_index+0 ], 0.7+0.3*roof_colors[ roof_color_index+1 ], 0.8+0.2*roof_colors[ roof_color_index+2 ] );
                };


                float random_rotate_180 = M_PI*( (random_i/2 + ri/4 + pi/2) % 2 );

                rvt_set_writing_layer( RVT_LAYER_BUILDING );
                rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );
                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_BUILDING_COLOR_CODE , rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale, scale_h, scale), ps->p[pi].azimuth + random_rotate_180, 0.25*(pi%4), 0.1 + (pi%2),
                   vbodata_index );

                rvt_gp_add(RVT_GP_LAYER_BUILDING, rvt_reg.tile_i, trp_global_pos, ps->p[pi].azimuth, NULL, RVT_GP_FLAG_GENERATED_BUILDING, 1.5 );

                rvt_rural_write_stuff( ps->p[pi].p, ps->p[pi].azimuth, lamp_distance, rand_shift_forward, last_shift_forward, max_side_wall, (pi == ps->points_count-1) ? 1 : 0, (random_i/3 + ri/2 + pi) );


            };

            rs_point_set_destroy(ps);

        };

    };


};


void rvt_railway_towers(rs_shape_t *p, int stage_i,  gd_lines_data_t *sdata) {

    if (!sdata->railway) {
        return;
    };

    if (sdata->bridge) {
        return;
    };

    float road_width = 0.375;


    int vbodata_index = VBODATA_RAILWAY_TOWER;
    float wire_height = 2.2;
    float wire_thickness = 0.4;
    float lamp_distance = 11.0;
    float lamp_side_shift = road_width + 0.7;
    int sides = 1;

    float scale = 0.075;
    float scale_h = 0.075;


    rvt_set_writing_layer( RVT_LAYER_PROP );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    for (int side_i = 0; side_i < sides; side_i++) {

        float sign = 1.0 - 2.0*side_i;

        for (int ri = 0; ri < p->rings_count; ri++) {


            rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[ri], lamp_distance, sign * lamp_side_shift, 0.0 + M_PI*side_i, 0 );

            int skip_wire = 0;

            for (int pi = 0; pi < ps->points_count; pi++ ) {

                rs_point_t trp = ps->p[pi].p;

                rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                rvt_gline_t *closest_gline = NULL;
                float closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                if (closest_gline) {
                    if (closest_distance < 0.1) {
                        skip_wire = 1;
                        continue;
                    };
                };

                rvt_set_writing_layer( RVT_LAYER_PROP );
                rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );
                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_USE_VBODATA_COLORS , rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale, scale_h, scale), -M_PI_2 + ps->p[pi].azimuth, 0.25*(pi%4), 0.1 + (pi%2),
                   vbodata_index );


                rvt_set_writing_layer( RVT_LAYER_WIRE );
                rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

                if ( (pi > 1) && (!skip_wire) ) {

                    rs_angled_point_t ap_next = ps->p[pi+0];
                    rs_angled_point_t ap = ps->p[pi-1];

                    float ap_height =       rvt_hm_get_height_adv(ap.p.x/rvt_app_get_sc(), ap.p.y/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
                    float ap_next_height = rvt_hm_get_height_adv(ap_next.p.x/rvt_app_get_sc(), ap_next.p.y/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );

                    float nodes[] = {  0.32, 2.5, 0.4, 2.8,      -1.2, 2.2, -1.2, 2.8 };
                    for (int ni = 0; ni < 4; ni++) {
                        float ss = nodes[ni*2+0];
                        float sh = nodes[ni*2+1];
                        rvt_write_wire( rs_vec3(ap.p.x+ss*cos(M_PI_2+ap.azimuth), sh + ap_height, ap.p.y+ss*sin(M_PI_2+ap.azimuth)),
                        rs_vec3(ap_next.p.x+ss*cos(M_PI_2+ap_next.azimuth), sh + ap_next_height, ap_next.p.y+ss*sin(M_PI_2+ap_next.azimuth)), 1.0, 0.8, 0 ); // RVT_GEOM_FLAG_ADD_HM
                        if (pi == 0) {
                            rvt_write_wire( rs_vec3(ap.p.x+ss*cos(M_PI_2+ap.azimuth), sh + ap_height, ap.p.y+ss*sin(M_PI_2+ap.azimuth)),
                            rs_vec3(ap.p.x+4.0-6.0*((ni+pi)%3), sh + ap_height - 12.0, ap.p.y+6.0-4.5*(ni%5) ), 1.0, 0.8, 0 ); // RVT_GEOM_FLAG_ADD_HM
                        };
                    };

                };

                skip_wire = 0;



            };

            rs_point_set_destroy(ps);

        };

    };

};



void rvt_railway(rs_shape_t *p, int stage_i, gd_lines_data_t *data) {

    if (!data->railway) {
        return;
    };

    if (data->tunnel) {
        rvt_tunnel(p, stage_i, data);
        return;
    };


    if (data->bridge) {
        rvt_bridge(p, stage_i, data);

        rvt_set_writing_layer(RVT_LAYER_STRIPES);
        rvt_set_writing_igroup(RVT_IGROUP_DETAILS);

        rvt_write_stripe(RVT_GEOM_FLAG_OPEN, data->base_height + RVT_BRIDGE_HEIGHT*rvt_app_get_sc_z(), 0.70, 0.0, 0.0, 0.125, 1, 1.0, 1.0, 0.0, p);

        return;
    };



    rvt_set_writing_layer(RVT_LAYER_STRIPES);
    rvt_set_writing_igroup(RVT_IGROUP_DEFAULT);

    float tc_v_shift = 0.0;
    if (data->railway == OSM_TAG_RAILWAY_TRAM) {
        tc_v_shift = 6.0/8.0;
    };

    rvt_write_stripe(RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS, 0.0, 0.70, 0.0, 0.0+tc_v_shift, 0.125+tc_v_shift, 1, 1.0, 1.0, 0.0, p);


    float max_segment_length = 24.0;
    rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, max_segment_length);
    rvt_gline_add_shape( RVT_GLINE_LAYER_RAILWAY, rvt_reg.tile_i, rvt_reg.shift_x, rvt_reg.shift_y, p_segmentized, data, RVT_GLINE_FLAG_RAILWAY, 0.55, 1 );
    rs_shape_destroy(p_segmentized);



};

void rvt_hw_stripes(rs_shape_t *p, gd_lines_data_t *sdata) {

    float road_halfwidth = 0.375 * sdata->lanes;
    if (sdata->hw == OSM_TAG_HW_FOOTWAY) {
        road_halfwidth = 0.275;
    };

    int flags = RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS;
    float y_start = 0.0;
    if (sdata->bridge) {
        flags = RVT_GEOM_FLAG_OPEN;
        y_start = sdata->base_height + RVT_BRIDGE_HEIGHT*rvt_app_get_sc_z();
    };

    rvt_set_writing_layer(RVT_LAYER_STRIPES);
    rvt_set_writing_igroup(sdata->bridge ? RVT_IGROUP_DETAILS : RVT_IGROUP_DEFAULT);

    if (sdata->lanes == 2) {
        rvt_write_stripe(flags, y_start, 0.12,  0.0, 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
    }
    else if (sdata->lanes & 1) { // odd
        for (int li = 0; li < sdata->lanes-1; li++) {
            rvt_write_stripe(flags, y_start, 0.12, -road_halfwidth+ (li+1)*(2.0*road_halfwidth/sdata->lanes), 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
        };
    }
    else {  // even
        if (sdata->oneway == OSM_TAG_ONEWAY_YES) {
            rvt_write_stripe(flags, y_start, 0.12,  0.0, 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
        }
        else {
            rvt_write_stripe(flags, y_start, 0.12,  0.0, 7.0/16.0, 8.0/16.0, 1, 1.0, 1.0, 0.0, p);
        };

        for (int li = 0; li < sdata->lanes/2 - 1; li++) {
            rvt_write_stripe(flags, y_start, 0.12,  (li+1)*2.0*road_halfwidth/sdata->lanes, 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
            rvt_write_stripe(flags, y_start, 0.12, -(li+1)*2.0*road_halfwidth/sdata->lanes, 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
        };

    };

};

void rvt_road(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata) {

    if ( (!sdata->hw) && (!sdata->aeroway) ) {
        return;
    };


    rvt_color(0.2, 0.0, 0.0);

    int gline_layer = RVT_GLINE_LAYER_ROAD;


    if (!sdata->lanes) {
        if ( (sdata->hw == OSM_TAG_HW_FOOTWAY) || (sdata->hw == OSM_TAG_HW_PATH)  || (sdata->hw == OSM_TAG_HW_TRACK) ) {
            sdata->lanes = 1;
            gline_layer = RVT_GLINE_LAYER_PATH;
        }
        else if ( (sdata->hw == OSM_TAG_HW_CYCLEWAY) || (sdata->hw == OSM_TAG_HW_SERVICE) || (sdata->hw == OSM_TAG_HW_RESIDENTIAL)
                 || (sdata->hw == OSM_TAG_HW_TERTIARY) || (sdata->hw == OSM_TAG_HW_TERTIARY_LINK)|| (sdata->hw == OSM_TAG_HW_LIVING_STREET) || (sdata->hw == OSM_TAG_HW_UNCLASSIFIED) )  {
            sdata->lanes = 2;
        }
        else {
            sdata->lanes = 4;
        };

        if (sdata->oneway == OSM_TAG_ONEWAY_YES) {
            sdata->lanes = (sdata->lanes+1)/2;
        };
    };



    float road_halfwidth = 0.375 * sdata->lanes;
    if (sdata->hw == OSM_TAG_HW_FOOTWAY) {
        road_halfwidth = 0.275;
    };

    if (sdata->aeroway == OSM_TAG_AEROWAY_TAXIWAY) {
        road_halfwidth = 0.375 * 8.0;
    }
    else if (sdata->aeroway == OSM_TAG_AEROWAY_RUNWAY) {
        road_halfwidth = 0.375 * 18.0;
    };

    if (sdata->width) {
        road_halfwidth = 0.5 * sdata->width;
    };



    int gline_flags = RVT_GLINE_FLAG_HW_SECONDARY_OR_HIGHER;
    if ( (sdata->hw == OSM_TAG_HW_RESIDENTIAL) ||  (sdata->hw == OSM_TAG_HW_LIVING_STREET) ) {
        gline_flags = RVT_GLINE_FLAG_HW_RESIDENTIAL;
    }
    else if ( (sdata->hw == OSM_TAG_HW_TERTIARY) || (sdata->hw == OSM_TAG_HW_TERTIARY_LINK) ) {
        gline_flags = RVT_GLINE_FLAG_HW_TERTIARY;
    };

    if (!sdata->lit) {

        if ( (sdata->hw == OSM_TAG_HW_RESIDENTIAL) || (sdata->hw == OSM_TAG_HW_TERTIARY) ||
                (sdata->hw == OSM_TAG_HW_SECONDARY) || (sdata->hw == OSM_TAG_HW_PRIMARY)  ) {

                    sdata->lit = OSM_TAG_LIT_YES;

                }
        else {
            sdata->lit = OSM_TAG_LIT_NO;
        };

    };

    if (sdata->bridge) {
        rvt_bridge(p, stage_i, sdata);
        rvt_hw_stripes(p, sdata);
        return;
    };

    if (sdata->tunnel) {
        rvt_tunnel(p, stage_i, sdata);
        return;
    };


    if (sdata->hw == OSM_TAG_HW_PATH) {

        rvt_set_writing_layer(RVT_LAYER_STRIPES);
        rvt_set_writing_igroup(RVT_IGROUP_ALT);

        rvt_write_stripe(RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS, 0.0, 0.45, 0.0, 0.250, 0.375, 1, 1.0, 1.0, 0.0, p);

        return;

    };

    if ( (sdata->hw == OSM_TAG_HW_TRACK) || (sdata->surface & OSM_TAG_BIT_SURFACE_UNPAVED) ) {

        if (sdata->aeroway != OSM_TAG_AEROWAY_RUNWAY) {

            rvt_set_writing_layer(RVT_LAYER_STRIPES);
            rvt_set_writing_igroup(RVT_IGROUP_ALT);

            rvt_write_stripe(RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS, 0.0, 0.45, 0.0, 0.125, 0.250, 1, 1.0, 1.0, 0.0, p);

            float max_segment_length = 24.0;
            rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, max_segment_length);
            rvt_gline_add_shape( gline_layer, rvt_reg.tile_i, rvt_reg.shift_x, rvt_reg.shift_y, p_segmentized, sdata, gline_flags, 0.5 + road_halfwidth, 1 );
            rs_shape_destroy(p_segmentized);

            return;

        };

    };

    float stripe_halfwidth = 0.08;

    if ( (sdata->hw != OSM_TAG_HW_FOOTWAY) && (sdata->hw != OSM_TAG_HW_SERVICE) && (!sdata->aeroway)) {

        rvt_hw_stripes(p, sdata);

    };

    if ( (sdata->aeroway == OSM_TAG_AEROWAY_RUNWAY) && (!(sdata->surface & OSM_TAG_BIT_SURFACE_UNPAVED)) ) {
        int flags = RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS;
        float y_start = 0.0;
        rvt_set_writing_layer(RVT_LAYER_STRIPES);
        rvt_set_writing_igroup(sdata->bridge ? RVT_IGROUP_DETAILS : RVT_IGROUP_DEFAULT);
        rvt_write_stripe(flags, y_start, 0.952, 0.0, 6.2/16.0, 6.8/16.0, 1, 1.0, 1.0, 0.0, p);
    };

    if (sdata->surface & OSM_TAG_BIT_SURFACE_UNPAVED) {
        rvt_color(0.7, 0.0, 0.0);
        stripe_halfwidth = 1.5;
    };






    float max_segment_length = 24.0;
    rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, max_segment_length);


    rvt_gline_add_shape( gline_layer, rvt_reg.tile_i, rvt_reg.shift_x, rvt_reg.shift_y, p_segmentized, sdata, gline_flags, 0.5 + road_halfwidth, 1 );

    rs_shape_t *p_buf = rs_shape_create_buffered_from_polygon(p_segmentized, road_halfwidth, RS_LINESTRING_OPEN_ROUND);



    rs_triangle_set_t *tris = rs_triangle_set_create_triangulated(p_buf);

    rvt_set_writing_layer( RVT_LAYER_ROAD );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

        rvt_write_triangles(0, 0.0, 0.0, tris);

    if (rvt_settings.smooth_road_edges) {
        rvt_set_writing_igroup( RVT_IGROUP_DETAILS );

        rvt_write_stripe(RVT_GEOM_FLAG_OPEN, 0.0, stripe_halfwidth+0.02, -stripe_halfwidth-road_halfwidth, 0.0, 0.125, 1, 1.0, 0.0, 0.0, p_segmentized);
        rvt_write_stripe(RVT_GEOM_FLAG_OPEN, 0.0, stripe_halfwidth+0.02,  stripe_halfwidth+road_halfwidth, 0.0, 0.125, 1, 0.0, 1.0, 0.0, p_segmentized);
    };


    rs_shape_destroy(p_buf);
    rs_shape_destroy(p_segmentized);
    rs_triangle_set_destroy(tris);

};




void rvt_river(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata) {


    if (!sdata->waterway) {
        return;
    };

    if ( (sdata->tunnel) && (sdata->tunnel != OSM_TAG_NO) ) {
        return;
    };


    rvt_color(0.0, 1.0, 0.0);

    float river_width = 0.750;



    if ( (sdata->waterway == OSM_TAG_WATERWAY_STREAM) ) {
        river_width = 1.2;
    }
    else if (sdata->waterway == OSM_TAG_WATERWAY_RIVER) {
        river_width = 3.2;
    }
    else {
        return;
    };



    float max_segment_length = 24.0;
    rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, max_segment_length);

    rvt_gline_add_shape( RVT_GLINE_LAYER_RIVER, rvt_reg.tile_i, rvt_reg.shift_x, rvt_reg.shift_y, p_segmentized, sdata, RVT_GLINE_FLAG_WATERWAY, 0.5 + river_width, 1 );


    rs_shape_t *p_buf = rs_shape_create_buffered_from_polygon(p_segmentized, river_width, RS_LINESTRING_OPEN_ROUND);


    rs_triangle_set_t *tris = rs_triangle_set_create_triangulated(p_buf);

    rvt_set_writing_layer( RVT_LAYER_AREA );
    rvt_set_writing_igroup( RVT_IGROUP_DETAILS ); // for water


    rvt_write_triangles(0, 0.0, 0.0, tris);

    if (rvt_settings.smooth_road_edges) {
        float stripe_halfwidth = 0.6*river_width;
        rvt_write_stripe(RVT_GEOM_FLAG_OPEN, 0.0, stripe_halfwidth+0.02, -stripe_halfwidth-river_width, 0.0, 0.125, 1, 1.0, 0.0, 0.0, p_segmentized);
        rvt_write_stripe(RVT_GEOM_FLAG_OPEN, 0.0, stripe_halfwidth+0.02,  stripe_halfwidth+river_width, 0.0, 0.125, 1, 0.0, 1.0, 0.0, p_segmentized);
    };


    rs_shape_destroy(p_buf);
    rs_shape_destroy(p_segmentized);
    rs_triangle_set_destroy(tris);

};





void rvt_area(rs_shape_t *p, int stage_i, gd_area_data_t *sdata) {


    rvt_color(0.0, 0.0, 0.0);
    int flag = 0;

    rvt_set_writing_layer( RVT_LAYER_AREA );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

    float stripe_halfwidth = 1.5;


    // default (R), details (G), alt (G)

    if ( (sdata->landuse == OSM_TAG_LANDUSE_GRASS) || (sdata->leisure == OSM_TAG_LEISURE_GARDEN) || ( sdata->natural == OSM_TAG_NATURAL_WOOD ) || (sdata->landuse == OSM_TAG_LANDUSE_FOREST) ) {
        rvt_color(0.7, 0.0, 0.0);
        flag = 1;
    }
    else if ( (sdata->natural == OSM_TAG_NATURAL_WATER) || (sdata->waterway == OSM_TAG_WATERWAY_RIVERBANK) || (sdata->landuse == OSM_TAG_LANDUSE_RESERVOIR) ) {
        rvt_set_writing_igroup( RVT_IGROUP_DETAILS );
        rvt_color(0.0, 1.0, 0.0);
        flag = 1;
    }
    else if ( (sdata->amenity == OSM_TAG_AMENITY_PARKING) || (sdata->hw == OSM_TAG_HW_PEDESTRIAN)
            || (sdata->aeroway == OSM_TAG_AEROWAY_APRON) ) {
        if (sdata->surface & OSM_TAG_BIT_SURFACE_UNPAVED) {
            rvt_color(0.7, 0.0, 0.0);
        }
        else {
            rvt_color(0.2, 0.0, 0.0);
            stripe_halfwidth = 0.14;
        };
        flag = 1;
    }
    else if (sdata->landuse == OSM_TAG_LANDUSE_RESIDENTIAL) {
        flag = 2; // process triangles but don't create geometry
    };

    if ( (sdata->natural == OSM_TAG_NATURAL_SAND) || (sdata->natural == OSM_TAG_NATURAL_BEACH) || (sdata->natural == OSM_TAG_NATURAL_HEATH) ) {
        rvt_set_writing_igroup( RVT_IGROUP_ALT );

        rvt_color(0.0, 0.0, 0.2);
        flag = 1;
    };

    if ( sdata->landuse == OSM_TAG_LANDUSE_QUARRY ) {
        rvt_set_writing_igroup( RVT_IGROUP_ALT );

        rvt_color(0.0, 0.0, 0.7);
        flag = 1;
    };

    if ( (sdata->leisure == OSM_TAG_LEISURE_PARK) || (sdata->natural == OSM_TAG_NATURAL_GRASSLAND) || (sdata->natural == OSM_TAG_NATURAL_SCRUB) ) {
        flag = 2;
    };

    if (sdata->amenity == OSM_TAG_AMENITY_PARKING) {

        rs_vec2_t trp_global_pos = rs_vec2_add( rs_shape_get_middle_point(p), rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

        rvt_gp_add(RVT_GP_LAYER_VP, rvt_reg.tile_i, trp_global_pos, 0.0, NULL, RVT_GP_FLAG_NODATA, 1.0 );

    };


    if (flag) {

        int gtriangle_layer = RVT_GTRIANGLE_LAYER_LANDUSE;
        int gtriangle_flags = RVT_GTRIANGLE_FLAG_DEFAULT;


        if ( ( sdata->natural == OSM_TAG_NATURAL_WATER ) || (sdata->waterway == OSM_TAG_WATERWAY_RIVERBANK) || (sdata->landuse == OSM_TAG_LANDUSE_RESERVOIR) ) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_WATER;
        };

        if (sdata->landuse == OSM_TAG_LANDUSE_RESIDENTIAL) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_LANDUSE;
            gtriangle_flags = RVT_GTRIANGLE_FLAG_RESIDENTIAL_UNKNOWN;
            if (sdata->residential == OSM_TAG_RESIDENTIAL_RURAL) {
                gtriangle_flags = RVT_GTRIANGLE_FLAG_RESIDENTIAL_RURAL;
            }
            else if (sdata->residential == OSM_TAG_RESIDENTIAL_URBAN) {
                gtriangle_flags = RVT_GTRIANGLE_FLAG_RESIDENTIAL_URBAN;
            };
        };

        if ( (sdata->natural == OSM_TAG_NATURAL_WOOD) || (sdata->leisure == OSM_TAG_LEISURE_GARDEN) || (sdata->landuse == OSM_TAG_LANDUSE_FOREST) ) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_GREEN_NATURALS;
            gtriangle_flags = RVT_GTRIANGLE_FLAG_FOREST;
        };

        if ( (sdata->natural == OSM_TAG_NATURAL_GRASSLAND) || (sdata->leisure == OSM_TAG_LEISURE_PARK) ) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_GREEN_NATURALS;
            gtriangle_flags = RVT_GTRIANGLE_FLAG_GRASSLAND;
        };

        if ( sdata->natural == OSM_TAG_NATURAL_SCRUB ) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_GREEN_NATURALS;
            gtriangle_flags = RVT_GTRIANGLE_FLAG_SCRUB;
        };

        if ( sdata->natural == OSM_TAG_NATURAL_HEATH ) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_GREEN_NATURALS;
            gtriangle_flags = RVT_GTRIANGLE_FLAG_HEATH;
        };


        if (stripe_halfwidth < 0.5) {
            gtriangle_layer = RVT_GTRIANGLE_LAYER_ASPHALT;
        };



        if (flag != 2) {

            if ((sdata->landuse != OSM_TAG_LANDUSE_RESERVOIR)) {
                if (rvt_settings.smooth_area_edges) {
                    rvt_write_stripe(0, 0.0, stripe_halfwidth+0.02, -stripe_halfwidth, 0.0, 0.125, 1, 1.0, 0.0, 0.0, p);
                };
            };
        }



        int subtiles_count = RVT_SUBTILES_SIDE_COUNT_PER_TILE;

        rs_triangle_set_t **tris_array = rs_triangle_set_array_create_subdivided_and_triangulated(p, rvt_app_get_sc(), subtiles_count);
        for (int i = 0; i < subtiles_count*subtiles_count; i++) {
            if ( tris_array[i] != NULL ) {
                if (flag != 2) {
                    rvt_write_triangles(0, 0, 0.0, tris_array[i]);
                };
                rvt_gtriangle_add_triangle_set( gtriangle_layer, rvt_reg.tile_i, gtriangle_flags, rvt_reg.shift_x, rvt_reg.shift_y, tris_array[i] );

            };
        };



        if (sdata->landuse == OSM_TAG_LANDUSE_RESERVOIR) {

            rvt_set_writing_layer(RVT_LAYER_BUILDING);
            rvt_set_writing_igroup(RVT_IGROUP_DETAILS);



            float border_thickness = 0.16;
            float border_height = 0.23;

            for (int ri = 0; ri < p->rings_count; ri++ ) {

                rs_shape_t *p_topborder = rs_shape_create_buffered_from_linestring(p->rings[ri], border_thickness, RS_LINESTRING_CLOSED );

                // TODO: error? workaround:
                if (p_topborder != 0) {

                    rs_point_t middle_point = rs_shape_get_middle_point(p);

                    float y_start = rvt_hm_get_height( rvt_reg.shift_x + middle_point.x, rvt_reg.shift_y + middle_point.y);

                    rvt_color(0.8, 0.8, 0.8);

                    rvt_write_wall(RVT_GEOM_FLAG_FLAT_WALL, y_start - 1.5, border_height + 1.5, 0.0, 0.2, 1.0, 0.0, p_topborder);

                    rvt_color(0.58, 0.58, 0.58);

                    rs_triangle_set_t *tris_buf2 = rs_triangle_set_create_triangulated( p_topborder );
                    rvt_write_triangles( 0, y_start - 1.5, border_height + 1.5, tris_buf2);
                    rs_triangle_set_destroy(tris_buf2);

                    rs_shape_destroy(p_topborder);

                }
                else {
                    rs_critical_alert_and_halt("p_topborder is 0");
                }

            };

        };

        rs_triangle_set_array_destroy(tris_array, subtiles_count);


    };

};

void rvt_all_forest(rs_shape_t *unused_shape, int stage_i, gd_area_data_t *unused_data) {


    int trees_counter = 0;

    rvt_set_writing_layer( RVT_LAYER_NATURAL );
    rvt_set_writing_igroup( RVT_IGROUP_DETAILS );

    int subtile_ix_start = rvt_reg.tile_ix * (RVT_SUBTILES_SIDE_COUNT_PER_TILE);
    int subtile_iy_start = rvt_reg.tile_iy * (RVT_SUBTILES_SIDE_COUNT_PER_TILE);

    for (int st_ix = 0; st_ix < RVT_SUBTILES_SIDE_COUNT_PER_TILE; st_ix++) {
        for (int st_iy = 0; st_iy < RVT_SUBTILES_SIDE_COUNT_PER_TILE; st_iy++) {

            int subtile_ix = subtile_ix_start + st_ix;
            int subtile_iy = subtile_iy_start + st_iy;

            int subtile_i = subtile_ix + RVT_SUBTILES_SIDE_COUNT*subtile_iy;

            int current_gtriangle_layer = RVT_GTRIANGLE_LAYER_GREEN_NATURALS;

            for (int li = 0; li < rvt_app_get_geodata()->gtriangle_list_count[subtile_i][current_gtriangle_layer]; li++) {
                for (int i = 0; i < rvt_app_get_geodata()->gtriangle_items_count[subtile_i][current_gtriangle_layer][li]; i++ ) {

                    rvt_gtriangle_t *gtriangle = &rvt_app_get_geodata()->gtriangle[subtile_i][current_gtriangle_layer][li][i];

                    if ( gtriangle->flags & RVT_GTRIANGLE_FLAG_GRASSLAND ) {
                        continue;
                    };

                    rs_triangle_t tr;

                    tr.p[0].x = gtriangle->triangle.p[0].x - rvt_reg.shift_x;
                    tr.p[0].y = gtriangle->triangle.p[0].y - rvt_reg.shift_y;
                    tr.p[1].x = gtriangle->triangle.p[1].x - rvt_reg.shift_x;
                    tr.p[1].y = gtriangle->triangle.p[1].y - rvt_reg.shift_y;
                    tr.p[2].x = gtriangle->triangle.p[2].x - rvt_reg.shift_x;
                    tr.p[2].y = gtriangle->triangle.p[2].y - rvt_reg.shift_y;

                    float tr_area = rs_triangle_area(&gtriangle->triangle);
                    if (tr_area < 21.0) {
                        continue;
                    };

                    float points_distance = 7.1;
                    if (gtriangle->flags & RVT_GTRIANGLE_FLAG_HEATH) {
                        points_distance = 11.9;
                    };




                    rs_point_set_t *ps = rs_point_set_create_from_triangle(&tr, points_distance, 0.63*points_distance);

                    for (int pi = 0; pi < ps->points_count; pi++) {
                        rs_point_t trp = ps->p[pi].p;


                        rs_vec2_t trp_global_pos = rs_vec2_add(trp, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );






                        if (rvt_point_is_in_area( rs_vec2(trp.x + rvt_reg.shift_x, trp.y + rvt_reg.shift_y), RVT_GTRIANGLE_LAYER_WATER, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                            continue;
                        };

                        if (rvt_point_is_in_area( rs_vec2(trp.x + rvt_reg.shift_x, trp.y + rvt_reg.shift_y), RVT_GTRIANGLE_LAYER_BUILDINGS, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE, NULL ) ) {
                            continue;
                        };

                        if (rvt_point_is_in_area( rs_vec2(trp.x + rvt_reg.shift_x, trp.y + rvt_reg.shift_y), RVT_GTRIANGLE_LAYER_GREEN_NATURALS, RVT_GTRIANGLE_FLAG_GRASSLAND, RVT_GTRIANGLE_FLAG_NONE,  NULL ) ) {
                            continue;
                        };

                        if (rvt_point_is_in_area( rs_vec2(trp.x + rvt_reg.shift_x, trp.y + rvt_reg.shift_y), RVT_GTRIANGLE_LAYER_ASPHALT, RVT_GTRIANGLE_FLAG_ANY, RVT_GTRIANGLE_FLAG_NONE,  NULL ) ) {
                            continue;
                        };

                        float min_distance_to_road = 3.1; // tree pack
                        if (pi % 2) {
                            min_distance_to_road = 0.12; // tree
                        };


                        rvt_gline_t *closest_gline = NULL;
                        float closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                        if (closest_gline) {
                            if (closest_distance < min_distance_to_road) {
                                continue;
                            };
                        };

                        closest_distance = rvt_gline_find_nearest(trp_global_pos, RVT_GLINE_LAYER_RAILWAY, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
                        if (closest_gline) {
                            if (closest_distance < min_distance_to_road) {
                                continue;
                            };
                        };

                        float scale = 0.33*(5.0+(pi%3));
                        float scale_h = 0.8*scale;

                        trees_counter++;

                        // #tree and #treepack
                        rvt_color( 0.051+ 0.19/5 + 0.02*(pi%6), 0.051+ 0.26/5 + 0.015*(pi%3), 0.051+ 0.18/5 + 0.017*(pi%5));
                        if (rvt_app_get_visstyle_index() == VISSTYLE_INDEX_WINTER) {
                            rvt_color(0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6), 0.3 + 0.03*(pi%6));
                        };


                        if ( gtriangle->flags & RVT_GTRIANGLE_FLAG_SCRUB ) {

                            // #bush

                            rvt_set_writing_igroup( RVT_IGROUP_DETAILS );
                            scale *= 0.5;
                            scale_h *= 0.21;

                            rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.5*(pi%2), 0.1 + (pi%2),  VBODATA_BUSH1 );

                        }
                        else {

                            // #tree

                            if (pi%2) {

                                // tree
                                rvt_set_writing_igroup( RVT_IGROUP_ANGLE0 + (trees_counter%6) );

                                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.5*( (pi/2) %2), 0.1 + ( ( (1+pi)/2)%2),  VBODATA_TREE1 );

                            }
                            else {

                                // tree pack

                                rvt_set_writing_igroup( RVT_IGROUP_ALT );

                                rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM, rs_vec3(trp.x, 0.0, trp.y), rs_vec3(scale/2, scale_h, scale/2), 2.0*pi, 0.0, 0.1 + ( ( (1+pi)/2)%2),  VBODATA_TREEPACK1 );
                            };




                        };



                    };



                    rs_point_set_destroy(ps);


                }
            };


        };
    };









};

void rvt_bridge(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata) {

    if (!sdata->bridge) {
        return;
    };

    if ( (!sdata->hw) && (!sdata->railway) ) {
        return;
    };

    if (p->rings_count == 0) {
        return;
    };

    if (p->rings[0]->points_count < 2) {
        return;
    };



    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

    int oi = (( ((intptr_t)sdata) & 0xFFFF ));


    float y_start = sdata->base_height;

    float y_height = RVT_BRIDGE_HEIGHT;
    float y_min_height = 6.0;

    float border_height = 0.4;

    float width = 0.375 * sdata->lanes;

    if (sdata->railway) {
        width = 0.7;
    };

    width += 0.25; // bridge should be wider than road


    float scale_z = width;
    float scale_x = 0.7 + 0.3*width;

    y_height *= rvt_app_get_sc_z();
    y_min_height *= rvt_app_get_sc_z();


    rs_shape_t *p_segmentized = rs_shape_create_segmentized(p, 24.0);

    rs_shape_t *sh = rs_shape_create_buffered_from_polygon(p_segmentized, width, RS_LINESTRING_OPEN_BUTT);
    rs_shape_t *border_lines = rs_shape_create_with_offset(p_segmentized, width);
    rs_shape_t *border_sh = rs_shape_create_buffered_from_polygon(border_lines, 0.2, RS_LINESTRING_OPEN_BUTT);



    rvt_color( 0.25, 0.25, 0.25 );

    rvt_set_writing_layer( RVT_LAYER_WALLS );
    int barrier_flags = RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT | RVT_GEOM_FLAG_OPEN;

    rvt_write_wall( barrier_flags, y_start + y_height, border_height, 0.873, 0.752, 2.0, 0.0, border_lines);


    rvt_set_writing_layer( RVT_LAYER_PROP );

    rvt_color( 0.45, 0.45, 0.46 );
    if (sdata->railway) {
        rvt_color( 0.3, 0.3, 0.305 );
    };

    rvt_write_wall(0, y_start + y_min_height, y_height - y_min_height, -0.25*2, 0.0, 1.0, 0.0, sh);

    if (sdata->hw) {
        rvt_color( 0.55, 0.55, 0.55 );
    };

    rs_triangle_set_t *tris_buf = rs_triangle_set_create_triangulated(sh);
    rvt_write_triangles(0, y_start + y_min_height, y_height - y_min_height, tris_buf);
    rs_triangle_set_destroy(tris_buf);




    rs_shape_destroy(border_lines);
    rs_shape_destroy(border_sh);

    rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[0], 8.5, 0.0, 0.0, 0 );


    rvt_color( 0.62, 0.62, 0.6 );

    rvt_set_writing_layer( RVT_LAYER_BUILDING );
    rvt_set_writing_igroup( RVT_IGROUP_DETAILS );

    for (int i = 0; i < ps->points_count; i++) {
        rs_angled_point_t ap = ps->p[i];
        rs_vec2_t gp_global_pos = rs_vec2_add(ap.p, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );
        rvt_write_vbodata( 0, rs_vec3(ap.p.x, y_start + y_min_height, ap.p.y), rs_vec3(scale_z, 1.0, scale_z), ap.azimuth, 0.0, 0.0, VBODATA_BRIDGE_BASE1 );
    };

    rs_point_set_destroy(ps);

    float azimuth0 = atan2( p->rings[0]->p[1].y - p->rings[0]->p[0].y, p->rings[0]->p[1].x - p->rings[0]->p[0].x );
    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS, rs_vec3(p->rings[0]->p[0].x, y_start + y_height, p->rings[0]->p[0].y), rs_vec3(scale_x, 1.0, scale_z), M_PI + azimuth0, 0.0, 0.0, VBODATA_BRIDGE_END1 );
    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS | RVT_GEOM_FLAG_ADD_HM, rs_vec3(p->rings[0]->p[0].x, 0.0, p->rings[0]->p[0].y), rs_vec3(scale_x, 1.0, scale_z), M_PI + azimuth0, 0.0, 0.0, VBODATA_BRIDGE_GARBAGE1 );

    int i_end = p->rings[0]->points_count-1;
    float azimuth1 = atan2( p->rings[0]->p[i_end].y - p->rings[0]->p[i_end-1].y, p->rings[0]->p[i_end].x - p->rings[0]->p[i_end-1].x );
    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS, rs_vec3(p->rings[0]->p[i_end].x, y_start + y_height, p->rings[0]->p[i_end].y), rs_vec3(scale_x, 1.0, scale_z), azimuth0, 0.0, 0.0, VBODATA_BRIDGE_END1 );
    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS | RVT_GEOM_FLAG_ADD_HM, rs_vec3(p->rings[0]->p[i_end].x, 0.0, p->rings[0]->p[i_end].y), rs_vec3(scale_x, 1.0, scale_z), azimuth0, 0.0, 0.0, VBODATA_BRIDGE_GARBAGE1 );



    rs_shape_destroy(sh);

    rs_shape_destroy(p_segmentized);

};



void rvt_tunnel(rs_shape_t *p, int stage_i, gd_lines_data_t *sdata) {

    if (!sdata->tunnel) {
        return;
    };

    if ( (!sdata->hw) && (!sdata->railway) ) {
        return;
    };

    if (p->rings_count == 0) {
        return;
    };

    if (p->rings[0]->points_count < 2) {
        return;
    };

    float width = 0.375 * sdata->lanes;

    if (sdata->railway) {
        width = 0.7;
    };

    float scale_z = width;
    float scale_y = width;
    float scale_x = 0.25 + 0.75*width;

    scale_x *= 2.5;
    scale_y *= 2.25;
    scale_z *= 2.0;

    rvt_color( 0.62, 0.62, 0.6 );

    rvt_set_writing_layer( RVT_LAYER_BUILDING );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    float azimuth0 = atan2( p->rings[0]->p[1].y - p->rings[0]->p[0].y, p->rings[0]->p[1].x - p->rings[0]->p[0].x );

    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS | RVT_GEOM_FLAG_ADD_HM, rs_vec3(p->rings[0]->p[0].x, 0.0, p->rings[0]->p[0].y), rs_vec3(scale_x, 1.0, scale_z), azimuth0, 0.0, 0.0, VBODATA_BRIDGE_GARBAGE1 );

    int i_end = p->rings[0]->points_count-1;
    float azimuth1 = atan2( p->rings[0]->p[i_end].y - p->rings[0]->p[i_end-1].y, p->rings[0]->p[i_end].x - p->rings[0]->p[i_end-1].x );

    rvt_write_vbodata( RVT_GEOM_FLAG_USE_VBODATA_COLORS | RVT_GEOM_FLAG_ADD_HM, rs_vec3(p->rings[0]->p[i_end].x, 0.0, p->rings[0]->p[i_end].y), rs_vec3(scale_x, 1.0, scale_z), M_PI + azimuth0, 0.0, 0.0, VBODATA_BRIDGE_GARBAGE1 );



};



void rvt_building_add_point(rs_shape_t *p, int stage_i, gd_building_data_t *data) {

    rs_shape_metadata_t md;
    rs_shape_analyze_metadata(&md, p);

    rs_vec2_t gp_global_pos = rs_vec2_add(md.center, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

    rvt_gp_add(RVT_GP_LAYER_BUILDING, rvt_reg.tile_i, gp_global_pos, 0.0, data, RVT_GP_FLAG_DATA_BUILDING, md.radius );

};

void rvt_all_buildings(void *unused_geometry, int stage_i, gd_building_data_t *unused_data) {

    int subtile_ix_start = rvt_reg.tile_ix * (RVT_SUBTILES_SIDE_COUNT_PER_TILE);
    int subtile_iy_start = rvt_reg.tile_iy * (RVT_SUBTILES_SIDE_COUNT_PER_TILE);

    for (int st_ix = 0; st_ix < RVT_SUBTILES_SIDE_COUNT_PER_TILE; st_ix++) {
        for (int st_iy = 0; st_iy < RVT_SUBTILES_SIDE_COUNT_PER_TILE; st_iy++) {

            int subtile_ix = subtile_ix_start + st_ix;
            int subtile_iy = subtile_iy_start + st_iy;

            int subtile_i = subtile_ix + RVT_SUBTILES_SIDE_COUNT*subtile_iy;

            int current_gp_layer = RVT_GP_LAYER_BUILDING;

            for (int li = 0; li < rvt_app_get_geodata()->gp_list_count[subtile_i][current_gp_layer]; li++) {
                for (int i = 0; i < rvt_app_get_geodata()->gp_items_count[subtile_i][current_gp_layer][li]; i++ ) {

                    rvt_gpoint_t *gpoint = &rvt_app_get_geodata()->gp[subtile_i][current_gp_layer][li][i];

                    if (gpoint->flags & RVT_GP_FLAG_GENERATED_BUILDING) {
                        continue;
                    };

                    gd_building_data_t *data = (gd_building_data_t*) gpoint->pdata;

                    if (data != NULL) {

                        rs_static_shape_t *st_shape = (rs_static_shape_t*) ( ((unsigned char*)data) + sizeof(gd_building_data_t) );
                        rs_shape_t *sh = rs_shape_create_from_static( st_shape );

                        data->area = rs_shape_area(sh);

                        if ((data->area > 398.0) || (data->b_levels > 5) || (data->b_height > 19.0) ) {
                            gpoint->flags |= RVT_GP_FLAG_LARGE_BUILDING;
                        };

                        rvt_building(sh, stage_i, data, rvt_get_gnote_by_point(gpoint) );

                        rs_shape_destroy(sh);

                    };

                    data->gen_flags |= RVT_GD_GEN_FLAG_DONE;

                }
            };


        };
    };


};



void rvt_building(rs_shape_t *p_orig, int stage_i, gd_building_data_t *sdata, rvt_gnote_t *gnote) {


    if (sdata->rvt_relation_role == OSM_TAG_RVT_RELATION_ROLE_OUTLINE) {
        return;
    };

    float perimeter = rs_shape_perimeter(p_orig);
    float tolerance = rs_min(RVT_BUILDING_SIMPLIFICATION_TOLERANCE, 0.016*perimeter );

    rs_shape_t *p = rs_shape_create_simplified(p_orig, tolerance);

    rs_shape_metadata_t md;
    rs_shape_analyze_metadata(&md, p);

    rs_vec2_t gp_global_pos = rs_vec2_add(md.center, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

    // GNotes are not implemented in this tool
    #ifdef STREETS_GAME
    if (gnote) {

        if (gnote->b_colour[0]) {
            sdata->building_colour = rvt_get_colour_from_string(gnote->b_colour);
        };

        if (gnote->roof_colour[0]) {
            sdata->roof_colour = rvt_get_colour_from_string(gnote->roof_colour);
        };

        if (gnote->b_levels) {
            sdata->b_levels = gnote->b_levels;
        };

        if (gnote->b_material) {
            // TODO: building materials
        };

        if (gnote->b_type) {
            sdata->building = rvt_get_building_type_from_string(gnote->b_type);
        };
    };
    #endif


    if (!sdata->building_part) {

        rvt_set_writing_layer( RVT_LAYER_AREA );
        rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

        rvt_color(0.2, 0.0, 0.0);

        const float building_base_width = 0.3;

        rs_shape_t *building_base = rs_shape_create_buffered_from_polygon(p, building_base_width, RS_POLYGON_CLOSED);
        rs_triangle_set_t *base_tris_buf = rs_triangle_set_create_triangulated(building_base);
        rvt_write_triangles(0, 0.0, 0, base_tris_buf);

        rvt_gtriangle_add_triangle_set( RVT_GTRIANGLE_LAYER_BUILDINGS, rvt_reg.tile_i, 0, rvt_reg.shift_x, rvt_reg.shift_y, base_tris_buf );

        rs_triangle_set_destroy(base_tris_buf);
        rs_shape_destroy(building_base);

    };



    rvt_set_writing_layer( RVT_LAYER_BUILDING );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


    int oi = (( ((intptr_t)sdata) & 0xFFFF )) / 37;


    if ( (sdata->b_height) && (sdata->roof_height) ) {
        sdata->b_height -= sdata->roof_height;
    };


    float y_start = sdata->base_height;
    int b_levels = sdata->b_levels;
    float y_height = sdata->b_height;
    float y_min_height = sdata->b_min_height;
    float roof_height = sdata->roof_height;


    float colors[] = {

        0.50, 0.35, 0.20,
        0.33, 0.33, 0.40, //
        0.66, 0.48, 0.21,
        0.80, 0.77, 0.77,

        0.62, 0.55, 0.50,
        0.53, 0.53, 0.56,
        0.55, 0.55, 0.50,
        0.50, 0.45, 0.49,
    };

    int color_index = 3 * (oi%8);

    rvt_color( colors[ color_index+0 ], colors[ color_index+1 ], colors[ color_index+2 ] );

    if (sdata->building == OSM_TAG_BUILDING_HOSPITAL) {
        rvt_color(0.85, 0.85, 0.91);
    };

    if (sdata->building_colour != RVT_NO_COLOR) {
        rvt_color_i( sdata->building_colour );
    };


    if (rvt_app_get_rvt_style() == RVT_BASE_STYLE_MAP_EDITOR) {
        rvt_color(0.0, 1.0, 0.0);
    };



    if (b_levels == 0) {
        if (sdata->b_height < 0.001) {
            b_levels = rvt_calc_building_levels_by_shape(p_orig);
            y_height = y_min_height + 1.0 * RVT_BUILDING_LEVEL_HEIGHT * b_levels;
        }
        else {
            b_levels = 1.0 / RVT_BUILDING_LEVEL_HEIGHT * (y_height - y_min_height);
        };
    }
    else {
        if (sdata->b_height < 0.001) {
            y_height = RVT_BUILDING_LEVEL_HEIGHT * b_levels;
        }
        else {
            // Do nothing
        };
    };

    if ( (sdata->building & OSM_TAG_BUILDING_COLLAPSED) || (sdata->building == OSM_TAG_BUILDING_GARAGES) ) {
        b_levels = 1;
        y_height = RVT_BUILDING_LEVEL_HEIGHT * b_levels;
    };



    sdata->b_height = y_height; // save it

    // vertical meter scale
    y_height *= rvt_app_get_sc_z();
    y_min_height *= rvt_app_get_sc_z();
    roof_height *= rvt_app_get_sc_z();


    int flat_wall_flag = 0;
    if ( sdata->building == OSM_TAG_BUILDING_WALL ) {
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };

    if ( (y_height - y_min_height < 5.05*rvt_app_get_sc_z()) && (!sdata->b_levels) ) {
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };

    if (sdata->area < 9.1) {
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };

    if ( (md.shape_type == RS_SHAPE_TYPE_ROUND) ) {
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };



    int building_face_index = 1; // default: commercial-style

    if ( sdata->building & OSM_TAG_BIT_BUILDING_RESIDENTIAL ) {
        flat_wall_flag = 0;
        building_face_index = 0;
    }
    else if ( sdata->building & OSM_TAG_BIT_BUILDING_PUBLIC ) {
        flat_wall_flag = 0;
        building_face_index = 1;
    }
    else if ( sdata->building & OSM_TAG_BIT_BUILDING_INDUSTRIAL ) {
        flat_wall_flag = 0;
        building_face_index = 2;
    }
    else if ( sdata->building & OSM_TAG_BIT_BUILDING_RUINS ) {
        building_face_index = 0;
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };

    if ( (sdata->building == OSM_TAG_BUILDING_CHURCH) || (sdata->building == OSM_TAG_BUILDING_CATHEDRAL) ) {
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };

    if (rvt_app_get_rvt_style() == RVT_BASE_STYLE_MAP_EDITOR) {
        building_face_index = 0;
        flat_wall_flag = RVT_GEOM_FLAG_FLAT_WALL;
    };


    float ca_component = (float) (1 << building_face_index);


    // Not ideal
    if (sdata->building_part) {
        if ( ((y_height-y_min_height) < 2.6) ) {
            rvt_set_writing_igroup(RVT_IGROUP_DETAILS);
        };
    };

    rvt_write_wall(flat_wall_flag, y_start + y_min_height, y_height - y_min_height, 0.25*b_levels, 0.0, 1.0, ca_component, p);

    if (y_min_height < 0.01) {
        // Base
        rvt_write_wall(RVT_GEOM_FLAG_FLAT_WALL, y_start - 0.25*2.0*RVT_BUILDING_LEVEL_HEIGHT, 0.25*2.0*RVT_BUILDING_LEVEL_HEIGHT, 0.25*2, 1.0, 0.0, ca_component, p);
    };

    // Roof
    if (sdata->roof_colour != RVT_NO_COLOR) {
        rvt_color_i( sdata->roof_colour );
    }
    else {
        float dest_color_component = 0.25;
        if (sdata->building & OSM_TAG_BUILDING_COLLAPSED) {
            dest_color_component = 0.85;
        };
        rs_vec3_t c = rs_vec3_linear( rvt_reg.geom_color, rs_vec3(dest_color_component, dest_color_component, dest_color_component), 0.75 );
        rvt_color( c.r, c.g, c.b );
    };

    if (rvt_app_get_rvt_style() == RVT_BASE_STYLE_MAP_EDITOR) {
        rvt_color(0.0, 1.0, 0.0);
    };

    if (rvt_app_get_visstyle_index() == VISSTYLE_INDEX_WINTER) {
        rvt_color(0.97, 0.97, 0.99);
    };

    int need_roof_decorations = (!sdata->building_part) ? 1 : 0;

    if (!rvt_settings.building_decorations) {
        need_roof_decorations = 0;
    };

    int need_special_roof = (roof_height > 0.001) ? 1 : 0;

    int need_rural_house_roof = 0;
    if (!sdata->building_part) {

        if (md.shape_type == RS_SHAPE_TYPE_RECTANGLE) {
            if ( (sdata->area < 24.0) && (sdata->area > 4.0) ) {
                if ( (sdata->building == OSM_TAG_YES) || (sdata->building == OSM_TAG_BUILDING_DETACHED) || (sdata->building == OSM_TAG_BUILDING_HOUSE) || (sdata->building == OSM_TAG_BUILDING_HUT) ) {
                    need_rural_house_roof = 1;
                };
            };
        };
    };

    if ( !need_special_roof ) {

        rs_triangle_set_t *tris_buf = rs_triangle_set_create_triangulated(p);
        rvt_write_triangles(0, y_start + y_min_height, y_height - y_min_height, tris_buf);
        rs_triangle_set_destroy(tris_buf);


        if (need_rural_house_roof) {

            need_roof_decorations = 0;


            int vbodata_index = ( (oi/16) % 3) ? VBODATA_ROOF_RURAL_01 : VBODATA_ROOF_RURAL_02;

            rs_point_t middle_point = md.center;


            float sc_w = 0.5 * 1.10 * md.halfwidth;
            float sc_l = 0.5 * 0.98 * md.halflength;
            float sc_h = 0.75;

            if (sdata->b_levels > 1) {
                sc_h = 1.1;
            };

            // Warning, this code has a copy in rural house renderer

            float roof_colors[] = {

                // green
                0.03, 0.61, 0.37,

                // pink (gray)
                0.70, 0.57, 0.70,

                // gray
                0.46, 0.46, 0.49,
                0.86, 0.86, 0.90,
                0.70, 0.75, 0.90,
                0.70, 0.75, 0.90,

                // red
                0.82, 0.33, 0.11,
                0.77, 0.44, 0.44,
                0.66, 0.05, 0.03,
                0.98, 0.20, 0.17,

                // blue
                0.35, 0.41, 0.98,
                0.55, 0.65, 0.99,

            };

            int roof_color_index = ((oi/16)+7) % 12;

            rvt_color_alt(rvt_reg.geom_color.r, rvt_reg.geom_color.g, rvt_reg.geom_color.b);
            rvt_color( roof_colors[roof_color_index*3 + 0], roof_colors[roof_color_index*3 + 1], roof_colors[roof_color_index*3 + 2] );

            rvt_write_vbodata( RVT_GEOM_FLAG_BUILDING_COLOR_CODE, rs_vec3(middle_point.x, y_start + y_height, middle_point.y), rs_vec3(sc_l, sc_h, sc_w), md.azimuth + ( (oi/4)%2 ? M_PI : 0.0 ), 0.0, 0.0, vbodata_index );
        };


    }
    else {
        need_roof_decorations = 0;


        int vbodata_index = -1;
        if (sdata->roof_shape == OSM_TAG_ROOF_SHAPE_DOME) {
            vbodata_index = VBODATA_ROOF_SHAPE_DOME;
        }
        else if (sdata->roof_shape == OSM_TAG_ROOF_SHAPE_ONION) {
            vbodata_index = VBODATA_ROOF_SHAPE_ONION;
        };

        if (vbodata_index != -1) {
            // premodelled vbodata roof

            rs_point_t middle_point = md.center;

            float sc_r = 0.98*md.radius;
            float sc_h = roof_height;

            if (sdata->roof_shape == OSM_TAG_ROOF_SHAPE_ONION) {
                sc_r = rs_min(sc_h, 1.3*md.radius);
                sc_h = rs_min(1.2*sc_r, roof_height );
            };

            rvt_write_vbodata( 0, rs_vec3(middle_point.x, y_start + y_height, middle_point.y), rs_vec3(sc_r, sc_h, sc_r), 0.0, 0.0, 0.0, vbodata_index );

        }
        else {
            // geometry-depended roof
            rvt_write_roof( y_start + y_height, roof_height, p );
        };

    };


    float internal_roof_shift = 1.3;
    rs_shape_t *p_internal_roof = rs_shape_create_buffered_from_polygon(p, -internal_roof_shift, RS_POLYGON_CLOSED);


    rvt_set_writing_igroup(RVT_IGROUP_DETAILS);


    if (need_roof_decorations) {

        int vbodata_index = (oi%2) ? VBODATA_BUILDING_ROOF_PART_2 : VBODATA_BUILDING_ROOF_PART_1;

        if (sdata->building & OSM_TAG_BUILDING_COLLAPSED) {
            vbodata_index = VBODATA_BUILDING_CONSTRUCTION_PART;
        };

        for (int ri = 0; ri < p_internal_roof->rings_count; ri++) {


            rs_point_set_t *ps;

            ps = rs_point_set_create_from_linestring(p_internal_roof->rings[ri], 6.5, 0.000, 0.0, POINT_SET_FLAG_CLOSED );
            rs_point_set_filter_by_distance(ps, (sdata->building & OSM_TAG_BUILDING_COLLAPSED) ? 2.2 : 3.5);

            for (int i = 0; i < ps->points_count; i++) {

                rs_angled_point_t ap = ps->p[i];
                rs_vec2_t gp_global_pos = rs_vec2_add(ap.p, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                rvt_gp_add(RVT_GP_LAYER_ROOF_PART1, rvt_reg.tile_i, gp_global_pos, ap.azimuth, NULL, 0, 1.0 );

                rs_vec3_t roof_part_scale = rs_vec3(0.6*0.75, 0.6*0.4, 0.6*0.55);

                rvt_write_vbodata( 0, rs_vec3(ap.p.x, y_start + y_height , ap.p.y), roof_part_scale, ap.azimuth, 0.0, 0.0, vbodata_index );
            };

            rs_point_set_destroy(ps);



            ps = rs_point_set_create_from_linestring(p_internal_roof->rings[ri], 8.5, 1.75, 0.0, POINT_SET_FLAG_CLOSED );
            rs_point_set_filter_by_distance(ps, 3.5);

            for (int i = 0; i < ps->points_count; i++) {

                rs_angled_point_t ap = ps->p[i];
                rs_vec2_t gp_global_pos = rs_vec2_add(ap.p, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

                rvt_write_vbodata( 0, rs_vec3(ap.p.x, y_start , ap.p.y), rs_vec3(0.67, 0.67, 0.67), ap.azimuth, 0.0, 0.0, VBODATA_BUILDING_ENTRANCE_1 );

            };

            rs_point_set_destroy(ps);


        };

    };



    // border on roof perimeter

    if (need_roof_decorations) {

        rvt_set_writing_igroup(RVT_IGROUP_DETAILS);

        float border_thickness = 0.08;
        float border_height = 0.1667;

        if (sdata->building & OSM_TAG_BUILDING_COLLAPSED) {
            border_height = 0.96;
        };


        rs_shape_t *p_topborder_b1 = rs_shape_create_buffered_from_polygon(p, -border_thickness, RS_POLYGON_CLOSED);

        for (int ri = 0; ri < p_topborder_b1->rings_count; ri++ ) {

            rs_shape_t *p_topborder = rs_shape_create_buffered_from_linestring(p_topborder_b1->rings[ri], border_thickness, RS_LINESTRING_CLOSED );

            if (p_topborder != 0) {

                rvt_write_wall(RVT_GEOM_FLAG_FLAT_WALL, y_start + y_height, border_height, 0.0, 0.1, 1.0, 0.0, p_topborder);

                rs_triangle_set_t *tris_buf2 = rs_triangle_set_create_triangulated( p_topborder );
                rvt_write_triangles(0, y_start + y_height, border_height, tris_buf2);
                rs_triangle_set_destroy(tris_buf2);

                rs_shape_destroy(p_topborder);

            }
            else {
                // Something if wrong, maybe topborder is collapsed to nothing
                // Ignore.
            }

        };


        rs_shape_destroy(p_topborder_b1);

    };

    rs_shape_destroy(p_internal_roof);

    rs_shape_destroy(p);

};


void rvt_road_signs(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    if ( (data->hw != OSM_TAG_HW_GIVE_WAY) && (data->hw != OSM_TAG_HW_STOP) ) {
        return;
    };

    rs_vec2_t shift_v = rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y);

    rs_vec2_t p_global_pos = rs_vec2_add(*v, shift_v );

    rvt_gline_t *closest_gline = NULL;
    float closest_distance = rvt_gline_find_nearest(p_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
    if (!closest_gline) {
        return;
    };

    float crossing_halflen = 0.35 * closest_gline->pdata->lanes;

    float azimuth = M_PI_2 + atan2( closest_gline->p2.y - closest_gline->p1.y, closest_gline->p2.x - closest_gline->p1.x );


    if (1) {

        float az = azimuth;

        crossing_halflen += 1.0;

        rs_vec2_t p1 = rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  );
        rs_vec2_t p2 = rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  );


        int vbodata_index = (data->hw == OSM_TAG_HW_GIVE_WAY) ? VBODATA_ROAD_SIGN_GIVE_WAY : VBODATA_ROAD_SIGN_STOP;
        float scale = 0.25;
        float scale_h = scale;

        int pi = 0;

        rvt_set_writing_layer( RVT_LAYER_PROP );
        rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

        if ( data->direction != OSM_TAG_DIRECTION_BACKWARD ) {
            rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_USE_VBODATA_COLORS , rs_vec3(p1.x, 0.0, p1.y), rs_vec3(scale, scale_h, scale), az, 0.25*(pi%4), 0.1 + (pi%2),
               vbodata_index );
        }
        else {
            rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_USE_VBODATA_COLORS , rs_vec3(p2.x, 0.0, p2.y), rs_vec3(scale, scale_h, scale), M_PI + az, 0.25*(pi%4), 0.1 + (pi%2),
               vbodata_index );
        };

    };

};


void rvt_traffic_light(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    if (data->hw != OSM_TAG_HW_TRAFFIC_SIGNALS) {
        return;
    };

    rs_vec2_t shift_v = rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y);

    rs_vec2_t p_global_pos = rs_vec2_add(*v, shift_v );

    rvt_gline_t *closest_gline = NULL;
    float closest_distance = rvt_gline_find_nearest(p_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
    if (!closest_gline) {
        return;
    };

    float crossing_halflen = 0.35 * closest_gline->pdata->lanes;

    float azimuth = M_PI_2 + atan2( closest_gline->p2.y - closest_gline->p1.y, closest_gline->p2.x - closest_gline->p1.x );

    rs_vec2_t p1 = rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  );
    rs_vec2_t p2 = rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  );

    if (1) {

        float az = azimuth;

        crossing_halflen *= 1.41;
        crossing_halflen += 1.0;
        azimuth += 0.5*M_PI_2;

        rs_vec2_t p1 = rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  );
        rs_vec2_t p2 = rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  );


        int vbodata_index = VBODATA_TRAFFIC_LIGHT;
        float scale = 0.108;
        float scale_h = scale;

        int pi = 0;

        rvt_set_writing_layer( RVT_LAYER_PROP );
        rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

        rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_USE_VBODATA_COLORS , rs_vec3(p1.x, 0.0, p1.y), rs_vec3(scale, scale_h, scale), -M_PI_2-M_PI_2 + az, 0.25*(pi%4), 0.1 + (pi%2),
           vbodata_index );
    };

};


void rvt_crossing(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    if (data->hw != OSM_TAG_HW_CROSSING) {
        return;
    };

    rs_vec2_t shift_v = rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y);
    rs_vec2_t p_global_pos = rs_vec2_add(*v, shift_v );


    rvt_gline_t *closest_gline = NULL;


    float closest_distance = rvt_gline_find_nearest(p_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
    if (!closest_gline) {
        return;
    };

    float crossing_halflen = 0.35 * closest_gline->pdata->lanes;

    float azimuth = M_PI_2 + atan2( closest_gline->p2.y - closest_gline->p1.y, closest_gline->p2.x - closest_gline->p1.x );

    rs_vec2_t p1 = rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  );
    rs_vec2_t p2 = rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  );

    rs_linestring_t *ls = rs_linestring_create(2);
    rs_linestring_append_point(ls, p1 );
    rs_linestring_append_point(ls, p2 );

    rs_shape_t *sh = rs_shape_create(1);
    sh->outer_rings_count = 1;
    rs_shape_append_ring(sh, ls);

    rvt_set_writing_layer(RVT_LAYER_STRIPES);
    rvt_set_writing_igroup(RVT_IGROUP_DEFAULT);
    rvt_write_stripe(RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS, 0.0, 0.52,  0.0, 8.0/16.0, 10.0/16.0, 1, 1.0, 1.0, 0.0, sh);

    rs_shape_destroy(sh);

    rs_vec2_t trp_global_pos = rs_vec2_add( *v, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );
    rvt_gp_add(RVT_GP_LAYER_VP, rvt_reg.tile_i, trp_global_pos, 0.0, NULL, RVT_GP_FLAG_NODATA, 1.0 );

    if ( (data->crossing == OSM_TAG_CROSSING_TRAFFIC_SIGNALS) || (closest_gline->pdata->lanes > 3) ) {

        float az = azimuth;

        crossing_halflen += 1.0;
        azimuth += 0.35;

        rs_vec2_t p1 = rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  );
        rs_vec2_t p2 = rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  );


        int vbodata_index = VBODATA_TRAFFIC_LIGHT;
        float scale = 0.092;
        float scale_h = scale;

        int pi = 0;

        rvt_set_writing_layer( RVT_LAYER_PROP );
        rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );
        rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM  | RVT_GEOM_FLAG_USE_VBODATA_COLORS, rs_vec3(p1.x, 0.0, p1.y), rs_vec3(scale, scale_h, scale), -M_PI_2-M_PI_2 + az, 0.25*(pi%4), 0.1 + (pi%2),
           vbodata_index );

        if (crossing_halflen > 1.8) { // crossing is wide enough for traffic lights on both sides
            rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM  | RVT_GEOM_FLAG_USE_VBODATA_COLORS, rs_vec3(p2.x, 0.0, p2.y), rs_vec3(scale, scale_h, scale), -M_PI_2+M_PI_2 + az, 0.25*(pi%4), 0.1 + (pi%2),
               vbodata_index );
        };


    };


};


void rvt_level_crossing(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    if (data->railway != OSM_TAG_RAILWAY_LEVEL_CROSSING) {
        return;
    };

    rs_vec2_t shift_v = rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y);

    rs_vec2_t p_global_pos = rs_vec2_add(*v, shift_v );

    rvt_gline_t *closest_gline = NULL;
    float closest_distance = rvt_gline_find_nearest(p_global_pos, RVT_GLINE_LAYER_RAILWAY, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_gline );
    if (!closest_gline) {
        return;
    };

    rvt_gline_t *closest_road = NULL;
    closest_distance = rvt_gline_find_nearest(p_global_pos, RVT_GLINE_LAYER_ROAD, RVT_GLINE_FLAG_ANY, RVT_GLINE_FLAG_NONE, &closest_road );
    if (!closest_road) {
        return;
    };

    float crossing_halflen = 0.5 + 2.0*0.35*closest_road->pdata->lanes;

    float azimuth = atan2( closest_gline->p2.y - closest_gline->p1.y, closest_gline->p2.x - closest_gline->p1.x );

    rs_linestring_t *ls = rs_linestring_create(2);
    rs_linestring_append_point(ls, rs_vec2_add(*v, rs_vec2( crossing_halflen*cos(azimuth),  crossing_halflen*sin(azimuth))  ) );
    rs_linestring_append_point(ls, rs_vec2_add(*v, rs_vec2(-crossing_halflen*cos(azimuth), -crossing_halflen*sin(azimuth))  ) );

    rs_shape_t *sh = rs_shape_create(1);
    sh->outer_rings_count = 1;
    rs_shape_append_ring(sh, ls);

    rvt_set_writing_layer(RVT_LAYER_STRIPES);
    rvt_set_writing_igroup(RVT_IGROUP_DEFAULT);
    rvt_write_stripe(RVT_GEOM_FLAG_OPEN | RVT_GEOM_FLAG_ADD_HM_TO_POINTS, 0.0, 0.7,  0.0, 10.0/16.0, 12.0/16.0, 0, 1.0, 1.0, 0.0, sh);

    rs_shape_destroy(sh);


    rs_vec2_t trp_global_pos = rs_vec2_add( *v, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );
    rvt_gp_add(RVT_GP_LAYER_VP, rvt_reg.tile_i, trp_global_pos, 0.0, NULL, RVT_GP_FLAG_NODATA, 1.0 );


};

void rvt_barrier_block(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    // Not implemented

};


void rvt_natural_tree(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    // Not implemented

};


void rvt_building_entrance(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    // All buildings have automatically created random entrances
    // Custom building entrances are not implemented

};


void rvt_bus_stop(rs_vec2_t *v, int stage_i, gd_point_data_t *data) {

    if ( data->hw != OSM_TAG_HW_BUS_STOP) {
        return;
    };

    rs_vec2_t gp_global_pos = rs_vec2_add( *v, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

    rvt_gp_add(RVT_GP_LAYER_BUS_STOP, rvt_reg.tile_i, gp_global_pos, 0.0, data, RVT_GP_FLAG_DATA_POINT, 1.0 );

};


void rvt_rural_write_stuff( rs_point_t p, float azimuth, float distance, float shift_forward, float last_shift_forward, float max_side_wall_len, int last, int rand_i ) {

    float side_shift_value = max_side_wall_len * ( 0.85 + 0.15 * rs_noise( (int) (1000.0*p.x), 1 ) );

    shift_forward = last_shift_forward = 0.0; // bugged, doesn't significantly affect quality anyway

    int flags = RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT | RVT_GEOM_FLAG_ADD_HM_TO_POINTS | ( RVT_GEOM_FLAG_OPEN );

    rs_shape_t *sh = rs_shape_create(1);
    rs_linestring_t *ls = rs_linestring_create(4);
    rs_shape_append_ring(sh, ls);


    rs_vec2_t v_forward_half = rs_vec2( (0.5 * distance + shift_forward) * cos(azimuth), (0.5 * distance + shift_forward) * sin(azimuth) );
    rs_vec2_t v_backward_half = rs_vec2( (0.5 * distance - last_shift_forward) * cos(azimuth), (0.5 * distance - last_shift_forward) * sin(azimuth) );
    rs_vec2_t v_side = rs_vec2( side_shift_value * sin(-azimuth), side_shift_value*cos(-azimuth) );

    rs_point_t p1 = rs_vec2_add(p, v_forward_half );
    rs_point_t p2 = rs_vec2_sub(p, v_backward_half );
    rs_point_t p3 = rs_vec2_add(p2, v_side );

    rs_linestring_append_point(ls, p3);
    rs_linestring_append_point(ls, p2);
    rs_linestring_append_point(ls, p1);


    float barrier_height = 1.1;

    int wall_type = (rand_i) % 2; // wooden or metal

    rvt_barrier_write_common( flags, 0, 0.0, barrier_height, wall_type, sh );

    rs_shape_destroy(sh);


};

void rvt_barrier_write_common( int flags, int barrier_osm_value, float y_start, int barrier_height, int wall_type_index, rs_shape_t *p ) {

    rvt_set_writing_layer( RVT_LAYER_WALLS );
    rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

    int k = wall_type_index;

    if (k == -1) {

        if (barrier_osm_value == OSM_TAG_BARRIER_WALL) {
            k = 2; // concrete
        };

        if (barrier_osm_value == OSM_TAG_BARRIER_FENCE) {
            k = 1; // metal
        };

    };


    float uv_scale = 1.0;
    if (k == 1) {
        uv_scale = 3.0;
    };

    rvt_write_wall( flags, y_start, barrier_height, 1.0 - 0.001 - 0.125*k, 1.0-0.125*k-0.123-0.001, uv_scale, 0.0, p);

    if (k == 2) {
        // concrete top
        rvt_write_stripe( flags, barrier_height-0.1, 0.08, 0.08, 762.0/1024.0, 720.0/1024.0, 1, 1.0, 1.0, 0.2, p );
    };


};

void rvt_barrier_wall_area(rs_shape_t *p, int stage_i, gd_area_data_t *data) {

    if (data) {
        if (!data->barrier) {
            return;
        };
    };

    int oi = (( ((intptr_t)data) & 0xFFFF ));

    rvt_color( 0.2 + 0.015*(oi%10), 0.2 + 0.02*(oi%7), 0.2 + 0.02*(oi%6));

    float barrier_height = 1.1;

    int flags = RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT | RVT_GEOM_FLAG_ADD_HM_TO_POINTS;

    rvt_barrier_write_common( flags, data->barrier, 0.0, barrier_height, -1, p );


};

void rvt_barrier_wall_line(rs_shape_t *p, int stage_i, gd_lines_data_t *data) {

    if (data) {
        if (!data->barrier) {
            return;
        };
    };

    int oi = (( ((intptr_t)data) & 0xFFFF ));

    rvt_color( 0.2 + 0.015*(oi%10), 0.2 + 0.02*(oi%7), 0.2 + 0.02*(oi%6));

    float barrier_height = 1.1;

    int flags = RVT_GEOM_FLAG_REMOVE_SIDE_EFFECT | RVT_GEOM_FLAG_ADD_HM_TO_POINTS | ( RVT_GEOM_FLAG_OPEN );

    rvt_barrier_write_common( flags, data->barrier, 0.0, barrier_height, -1, p );

};


void rvt_powerline(rs_shape_t *p, int stage_i, gd_lines_data_t *data) {


    if ( ! ((data->power == OSM_TAG_POWER_LINE) || (data->power == OSM_TAG_POWER_MINOR_LINE) ) ) {
        return;
    };

    int minor = ((data->power == OSM_TAG_POWER_LINE)) ? 0 : 1;

    int oi = (((intptr_t)data)) & 0xFFFF;

    rvt_color( 0.2 + 0.015*(oi%10), 0.2 + 0.02*(oi%7), 0.2 + 0.02*(oi%6));

    float filter_distance = minor ? 17.0 : 53.5;


    for (int ri = 0; ri < p->rings_count; ri++) {

        rs_point_set_t *ps = rs_point_set_create_from_linestring(p->rings[ri], 70.0, 0.000, 0.0, 0 );

        rs_point_set_make_azimuths_smooth(ps);


        for (int i = 0; i < ps->points_count; i++) {

            rs_angled_point_t ap = ps->p[i];
            rs_vec2_t gp_global_pos = rs_vec2_add(ap.p, rs_vec2(rvt_reg.shift_x, rvt_reg.shift_y) );

            rvt_gp_add(RVT_GP_LAYER_POWER_TOWER, rvt_reg.tile_i, gp_global_pos, ap.azimuth, data, RVT_GP_FLAG_DATA_LINE, 1.0 );

            float scale;

            rvt_set_writing_layer( RVT_LAYER_PROP );
            rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );

            scale = 0.3;

            int vbodata_index = VBODATA_MINOR_TOWER;
            if (!minor) {
                vbodata_index = VBODATA_POWER_TOWER;
                const float corner_threshold = 0.02;
                if ( (i > 0) && (i < ps->points_count-1) ) {
                    if ( fabs( ps->p[i-1].azimuth - ps->p[i].azimuth ) > corner_threshold ) {
                        if ( fabs( ps->p[i+1].azimuth - ps->p[i].azimuth ) > corner_threshold ) {
                            vbodata_index = VBODATA_POWER_TOWER_METAL;
                        };
                    };
                };
            };
             (minor) ? VBODATA_MINOR_TOWER : VBODATA_POWER_TOWER;
            rvt_write_vbodata( RVT_GEOM_FLAG_ADD_HM | RVT_GEOM_FLAG_USE_VBODATA_COLORS, rs_vec3(ap.p.x, 0.0, ap.p.y), rs_vec3(scale, scale, scale), ap.azimuth + M_PI_2, 0.25*(i%4), 0.1 + (i%2), vbodata_index );

            rvt_set_writing_layer( RVT_LAYER_WIRE );
            rvt_set_writing_igroup( RVT_IGROUP_DEFAULT );


            if (i < ps->points_count-1) {
                rs_angled_point_t ap_next = ps->p[i+1];

                float ap_height =       rvt_hm_get_height_adv(ap.p.x/rvt_app_get_sc(), ap.p.y/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );
                float ap_next_height = rvt_hm_get_height_adv(ap_next.p.x/rvt_app_get_sc(), ap_next.p.y/rvt_app_get_sc(), rvt_reg.tile_ix, rvt_reg.tile_iy );


                if (minor) {
                    rvt_write_wire( rs_vec3(ap.p.x, 5.0, ap.p.y), rs_vec3(ap_next.p.x, 5.0, ap_next.p.y), 1.0, 1.0, RVT_GEOM_FLAG_ADD_HM );
                    if (i == 0) {
                        rvt_write_wire( rs_vec3(ap.p.x, 5.0, ap.p.y), rs_vec3(ap.p.x + 5.0, 5.0 - 7.0, ap.p.y), 1.0, 1.0, RVT_GEOM_FLAG_ADD_HM );
                    };
                }
                else {

                    float nodes[] = { 0.0, 10.5, -1.4, 9.6, 1.4, 9.6, -2.0, 8.2, 2.0, 8.2, -1.4, 6.8, 1.4, 6.8 };
                    for (int ni = 0; ni < 7; ni++) {
                        float ss = nodes[ni*2+0];
                        float sh = nodes[ni*2+1];
                        rvt_write_wire( rs_vec3(ap.p.x+ss*cos(M_PI_2+ap.azimuth), sh + ap_height, ap.p.y+ss*sin(M_PI_2+ap.azimuth)),
                        rs_vec3(ap_next.p.x+ss*cos(M_PI_2+ap_next.azimuth), sh + ap_next_height, ap_next.p.y+ss*sin(M_PI_2+ap_next.azimuth)), 1.0, 5.0, 0 );
                        if (i == 0) {
                            rvt_write_wire( rs_vec3(ap.p.x+ss*cos(M_PI_2+ap.azimuth), sh + ap_height, ap.p.y+ss*sin(M_PI_2+ap.azimuth)),
                            rs_vec3(ap.p.x+4.0-6.0*((ni+i)%3), sh + ap_height - 12.0, ap.p.y+6.0-4.5*(ni%5) ), 1.0, 5.0, 0 );
                        };
                    };

                };
            };

        };

        rs_point_set_destroy(ps);

    };


};




void gd_gen_geom(int tile_i) {


    int tile_ix = tile_i % RVT_TILES_SIDE_COUNT;
    int tile_iy = tile_i / RVT_TILES_SIDE_COUNT;


    int tile_world_ix = rvt_app_get_geodata()->tile[tile_i].tile_x + rvt_app_get_geodata()->start_tile_world_ix;
    int tile_world_iy = rvt_app_get_geodata()->tile[tile_i].tile_y + rvt_app_get_geodata()->start_tile_world_iy;

    DEBUG10f("gd_gen_geom(tile_i=%d, world ix/iy %d/%d)... \n", tile_i, tile_world_ix, tile_world_iy);

    rvt_begin( tile_i );


    size_t gd_data_sizes[RVT_GD_MAX_LAYERS_COUNT] = {
            sizeof(gd_building_data_t),
            sizeof(gd_area_data_t),
            sizeof(gd_lines_data_t),
            sizeof(gd_point_data_t)
        };


    for (int stage_i = 0; stage_i < RVT_GEN_MAX_STAGES; stage_i++) {

        for (int current_gd_layer = 0; current_gd_layer < RVT_GD_MAX_LAYERS_COUNT; current_gd_layer++) {

            for (int a = 0; a < rvt_app_get_geodata()->gd_list_count[tile_i][current_gd_layer]; a++) {
                unsigned char *p = rvt_app_get_geodata()->gd_data[tile_i][current_gd_layer][a];
                for (int i = 0; i < rvt_app_get_geodata()->gd_items_count[tile_i][current_gd_layer][a]; i++ ) {

                    if (current_gd_layer != RVT_GD_LAYER_POINTS) {

                        gd_general_data_t *data = (gd_general_data_t*) p;
                        p += gd_data_sizes[current_gd_layer];

                        rs_static_shape_t *st_shape = (rs_static_shape_t*) p;
                        p += data->content_len;

                        rs_shape_t *sh = rs_shape_create_from_static( st_shape );


                        for (int func_i = 0; func_i < rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[current_gd_layer]; func_i++) {

                            rvt_gen_func_rec_t *func_rec = &rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs[current_gd_layer][func_i];
                            if (func_rec->func_type == RVT_GEN_FUNC_TYPE_C) {
                                func_rec->func(sh, stage_i, data);
                            }
                            else {
                                // Not implemented: Lua function or other function
                            };
                        };

                        rs_shape_destroy(sh);

                    }
                    else {
                        // Point

                        gd_point_data_t *data = (gd_point_data_t*) p;
                        p += sizeof(gd_point_data_t);

                        rs_vec2_t *v = (rs_vec2_t*) p;
                        p += data->content_len;

                        for (int func_i = 0; func_i < rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[current_gd_layer]; func_i++) {

                            rvt_gen_func_rec_t *func_rec = &rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs[current_gd_layer][func_i];
                            if (func_rec->func_type == RVT_GEN_FUNC_TYPE_C) {
                                func_rec->func(v, stage_i, data);
                            }
                            else {
                                // Not implemented: Lua function or other function
                            };
                        };

                    };

                };
            };

        };

        // Last: layer-independent generations

        for (int func_i = 0; func_i < rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[RVT_GD_MAX_LAYERS_COUNT]; func_i++) {
            rvt_gen_func_rec_t *func_rec = &rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs[RVT_GD_MAX_LAYERS_COUNT][func_i];
            if (func_rec->func_type == RVT_GEN_FUNC_TYPE_C) {
                func_rec->func(NULL, stage_i, NULL);
            }
            else {
                // Not implemented: Lua function or other function
            };
        };

    };

    // Finally, creating geometry

    rvt_end(0, tile_ix, tile_iy);


};

void rvt_add_gen_func(int stage_i, int layer_i, int func_type, PRVTGENFUNC func, int lua_func_index) {

    if (rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[layer_i] == RVT_GEN_MAX_FUNCS_PER_LAYER) {
        rs_critical_alert_and_halt_sprintf("RVT_GEN_MAX_FUNCS_PER_LAYER reached\nstage %d, layer %d, func_type %d\n", stage_i, layer_i, func_type);
    };

    rvt_gen_func_rec_t *rec = &rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs[layer_i][ rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[layer_i] ];
    rvt_app_get_geodata()->rvt_gen_stages[stage_i].func_recs_count[layer_i]++;
    rec->func_type = func_type;
    rec->func = func;
    rec->lua_func_index = lua_func_index;
};


void rvt_gen_init_default_stages() {

    memset(rvt_app_get_geodata()->rvt_gen_stages, 0, sizeof(rvt_gen_stage_t) * RVT_GEN_MAX_STAGES );

    rvt_add_gen_func(0, RVT_GD_LAYER_AREA, RVT_GEN_FUNC_TYPE_C, rvt_area, 0);
    rvt_add_gen_func(0, RVT_GD_LAYER_BUILDING, RVT_GEN_FUNC_TYPE_C, rvt_building_add_point, 0);
    rvt_add_gen_func(0, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_railway, 0);
    rvt_add_gen_func(0, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_road, 0);
    rvt_add_gen_func(0, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_river, 0);
    rvt_add_gen_func(0, RVT_GD_MAX_LAYERS_COUNT, RVT_GEN_FUNC_TYPE_C, rvt_all_buildings, 0);

    rvt_add_gen_func(1, RVT_GD_LAYER_AREA, RVT_GEN_FUNC_TYPE_C, rvt_barrier_wall_area, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_road_lamps, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_road_rural_houses, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_road_footway_trees, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_railway_towers, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_powerline, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_LINES, RVT_GEN_FUNC_TYPE_C, rvt_barrier_wall_line, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_bus_stop, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_traffic_light, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_road_signs, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_crossing, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_level_crossing, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_barrier_block, 0);
    rvt_add_gen_func(1, RVT_GD_LAYER_POINTS, RVT_GEN_FUNC_TYPE_C, rvt_natural_tree, 0);
    rvt_add_gen_func(1, RVT_GD_MAX_LAYERS_COUNT, RVT_GEN_FUNC_TYPE_C, rvt_all_forest, 0);
    rvt_add_gen_func(1, RVT_GD_MAX_LAYERS_COUNT, RVT_GEN_FUNC_TYPE_C, rvt_full_tile_naturals, 0);


};

