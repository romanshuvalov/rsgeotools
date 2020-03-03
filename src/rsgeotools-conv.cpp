#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string.h>

using namespace std;


/*
void degrees2proj(double lon, double lat) {

    double x = lon * 20037508.342789 / 180;
    double y = log(tan((90.0 + lat) * M_PI / 360)) / (M_PI / 180);
    y = y * 20037508.342789 / 180;

}
*/

void tc2proj(int zoom_level, double x, double y, double *output_x, double *output_y) {

    double px = ((x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI) * 20037508.342789 / M_PI;
    double py = ( M_PI - y*2.0*M_PI/( pow(2.0, zoom_level) ) ) * 20037508.342789 / M_PI;

    *output_x = px;
    *output_y = py;

};


void degrees2tc(double lon, double lat, int zoom_level, double *output_x, double *output_y) {

    lon *= M_PI/180.0;
    lat *= M_PI/180.0;

    *output_x = 0.5 / M_PI * pow(2.0, zoom_level) * (M_PI + lon);
    *output_y = 0.5 / M_PI * pow(2.0, zoom_level) * ( M_PI - log( tan(M_PI/4.0 + lat/2.0) ) );

};

void tc2degrees(int zoom_level, double x, double y, double *output_lon, double *output_lat) {

    double lon;
    double lat;

    lon = (x * 2.0 * M_PI) / ( pow(2.0, zoom_level) ) - M_PI;
    lat = 2.0 * atan(exp(M_PI - y*2.0*M_PI/pow(2.0, zoom_level))) - M_PI/2.0;

    lon *= 180.0 / M_PI;
    lat *= 180.0 / M_PI;

    *output_lon = lon;
    *output_lat = lat;

};


int file_is_present(char *s) {

    FILE *fp = fopen(s, "r");
    if (!fp) {
        return 0;
    };
    fclose(fp);
    return 1;

};



void deg2ci( double lon1, double lat1, int *ilon1, int *ilat1, char *clon1, char *clat1 ) {

    if (lon1 >= 180.0) {
        lon1 -= 360.0;
    };

    *ilon1 = (int) lon1;
    *ilat1 = (int) lat1;

    *clon1 = 'E';
    *clat1 = 'N';

    if (lon1 < 0) {
        *ilon1 = -*ilon1 + 1;
        *clon1 = 'W';
    };
    if (lat1 < 0) {
        *ilat1 = -*ilat1 + 1;
        *clat1 = 'S';
    };

};


int main(int argc, char ** argv) {

    char op = 'X';

    if (argc > 1) {
        op = argv[1][0];
    };

    if ( (argc < 5) || ( strchr("cphfiIDot", op) == NULL ) ) {
        fprintf( stderr, "Usage: geoconv <OP> z_scale tile_x tile_y [dest_z/heightmap_image_size] \n");
        fprintf( stderr, "   or: geoconv <OP> lat lon dest_z side_tiles \n");
        fprintf( stderr, "Ops: \n");
        fprintf( stderr, "  c - print BB coordinates in 3857 projection to stdout\n");
        fprintf( stderr, "  D - print deg. coordinates to stdout using comma separator\n");
        fprintf( stderr, "  p - print poly file to stdout\n");
        fprintf( stderr, "  f - print list of HGT files to stdout\n");
        fprintf( stderr, "  i - print all tiles for scale <dest_z> located inside given tile \n");
        fprintf( stderr, "  I - same, but exclude lower 1/4 of world map (Antarctica) \n");
        fprintf( stderr, "  o - print tile for scale <dest_z> contains given tile \n");
        fprintf( stderr, "  h - create heightmap using GDAL utils (gdalwarp) and HGT data\n");
        fprintf( stderr, "  t - print list of NxN tiles around point defined by lat/lon " );
        fprintf( stderr, "\n");
        return -1;
    };

    if ( op == 't' ) {

        double lat, lon;
        int dest_z;
        int side_tiles;

        double tx, ty;

        sscanf(argv[2], "%lf", &lat);
        sscanf(argv[3], "%lf", &lon);
        sscanf(argv[4], "%d", &dest_z);
        sscanf(argv[5], "%d", &side_tiles);

        degrees2tc(lon, lat, dest_z, &tx, &ty);

        int xsign = -1 + 2 * (1 & (int) ( floor(2.0*tx) ));
        int ysign = -1 + 2 * (1 & (int) ( floor(2.0*ty) ));

        int ix = (int) tx;
        int iy = (int) ty;

        int total_tiles_side_count = (1 << dest_z);

        for (int i = 1; i <= side_tiles; i++) {

            int a = i/2;
            int s = -1 + 2 * ((i+1)%2);

            for (int j = 1; j <= side_tiles; j++) {

                int aj = j/2;
                int sj = -1 + 2 * ( (j+1)%2 );

                int tile_x = ((ix   + a*s*xsign) + total_tiles_side_count) % total_tiles_side_count;
                int tile_y = ((iy + aj*sj*ysign) + total_tiles_side_count) % total_tiles_side_count;

                printf("%d_%d\n", tile_x, tile_y );

            };

        };



        return 0;

    }

    int tile_x, tile_y, z_scale;

    sscanf(argv[2], "%d", &z_scale);
    sscanf(argv[3], "%d", &tile_x);
    sscanf(argv[4], "%d", &tile_y);


    double lon1, lat1, lon2, lat2, proj_x1, proj_y1, proj_x2, proj_y2;
    int ilon1, ilat1, ilon2, ilat2;
    int s_ilon1, s_ilat1, s_ilon2, s_ilat2;
    char clon1 = 'E';
    char clat1 = 'N';
    char clon2 = 'E';
    char clat2 = 'N';


    tc2proj( z_scale, tile_x, tile_y+1, &proj_x1, &proj_y1 );
    tc2proj( z_scale, tile_x+1, tile_y, &proj_x2, &proj_y2 );

    // lon2>lon1, lat2>lat1
    tc2degrees( z_scale, tile_x, tile_y+1, &lon1, &lat1);
    tc2degrees( z_scale, tile_x+1, tile_y, &lon2, &lat2);

    double min_proj_x = min(proj_x1, proj_x2);
    double max_proj_x = max(proj_x1, proj_x2);
    double min_proj_y = min(proj_y1, proj_y2);
    double max_proj_y = max(proj_y1, proj_y2);



    s_ilon1 = (int) lon1;
    s_ilat1 = (int) lat1;
    s_ilon2 = (int) lon2;
    s_ilat2 = (int) lat2;

    deg2ci( lon1, lat1, &ilon1, &ilat1, &clon1, &clat1 );
    deg2ci( lon2, lat2, &ilon2, &ilat2, &clon2, &clat2 );


    if (op == 'c') {
        printf("%.11f %.11f %.11f %.11f", min_proj_x, min_proj_y, max_proj_x, max_proj_y );
    }

    else if (op == 'D') {

        printf("%.11f,%.11f,%.11f,%.11f", lon1, lat1, lon2, lat2 );

    }

    else if (op == 'o') {
        if (argc != 6) {
            fprintf( stderr, "Usage: geoconv o z_scale tile_x tile_y dest_z\n");
            exit(-1);
        };

        int dest_z;
        sscanf(argv[5], "%d", &dest_z);

        int z_diff = z_scale - dest_z;

        if (z_diff <= 0) {
            fprintf( stderr, "dest_z must be lower than z_scale. \n");
            exit(-1);
        };

        printf("%d_%d", tile_x >> z_diff, tile_y >> z_diff);

    }

    else if ( (op == 'i') || (op == 'I') ) {

        if (argc != 6) {
            fprintf( stderr, "Usage: geoconv i z_scale tile_x tile_y dest_z\n");
            exit(-1);
        };

        int dest_z;
        sscanf(argv[5], "%d", &dest_z);

        int z_diff = dest_z - z_scale;


        if (z_diff < 0) {
            fprintf( stderr, "dest_z must be higher than z_scale or equal to z_scale. \n");
            exit(-1);
        };

        int t_side_count = 1 << z_diff;
        int t_total_count = t_side_count*t_side_count;

        if (t_total_count > 16384) {
            fprintf(stderr, "Too many tiles (%d). Aborted. \n", t_total_count);
            exit(-1);
        }

        int t_start_x = tile_x * (1 << z_diff);
        int t_start_y = tile_y * (1 << z_diff);

        int a = 0;
        int a_diff = dest_z - 2; // zoom level 2 = 4x4 tiles

        if (op == 'I') {
            a = 1;
        };

        for (int i = 0; i < t_side_count; i++) {
            for (int j = 0; j < t_side_count; j++) {
                if ( a*((t_start_y+j) >> a_diff) < 3) {
                    printf("%d_%d\n", t_start_x + i, t_start_y + j);
                };
            };
        };

    }

    else if (op == 'f') {

        double coord_threshold = 0.02;

        lat1 -= coord_threshold;
        lat2 += coord_threshold;
        lon1 -= coord_threshold;
        lon2 += coord_threshold;


        for (int i = (int)(floor(lat1+500.0) - 500.0 ); i <= (int)(floor(lat2+500.0) - 500.0); i++) {
            for (int j = (int)(floor(lon1+500.0) - 500.0 ); j <= (int)(floor(lon2+500.0) - 500.0); j++) {

                char a_clat, a_clon;
                int a_ilat, a_ilon;

                // 0.001 forces round to right side
                deg2ci( 0.001 + (double) j, 0.001 + (double) i, &a_ilon, &a_ilat, &a_clon, &a_clat);

                printf("%c%02d%c%03d.hgt\n", a_clat, a_ilat, a_clon, a_ilon );

            }
        }

    }

    else if (op == 'p') {

        // Poly

        printf("polygon\n1\n");

        printf("   %.11f   %.11f\n", lon1, lat1);
        printf("   %.11f   %.11f\n", lon1, lat2);
        printf("   %.11f   %.11f\n", lon2, lat2);
        printf("   %.11f   %.11f\n", lon2, lat1);
        printf("   %.11f   %.11f\n", lon1, lat1);

        printf("END\nEND\n");

    }

    else if (op == 'h') {

        // Heightmap
        // fprintf(stderr, "Processing tile: [%d, %d] of zoom %d \n", tile_x, tile_y, z_scale);

        if (argc != 6) {
            fprintf( stderr, "Usage: geoconv h z_scale tile_x tile_y image_size\n");
            exit(-1);
        };

        int image_size;
        sscanf(argv[5], "%d", &image_size);

        if ( (image_size & (image_size - 1)) != 0 ) {
            fprintf( stderr, "Error, image_size must be power of 2.\nAborted.\n");
            exit(-1);
        };


        char hgt_filenames[4][200];

        sprintf(hgt_filenames[0], "%c%02d%c%03d.hgt", clat1, ilat1, clon1, ilon1 );
        sprintf(hgt_filenames[1], "%c%02d%c%03d.hgt", clat1, ilat1, clon2, ilon2 );
        sprintf(hgt_filenames[2], "%c%02d%c%03d.hgt", clat2, ilat2, clon1, ilon1 );
        sprintf(hgt_filenames[3], "%c%02d%c%03d.hgt", clat2, ilat2, clon2, ilon2 );

        int f_present[4];

        for (int i = 0; i < 4; i++) {
            f_present[i] = file_is_present(hgt_filenames[i]);
            if (!f_present[i]) {
                printf("(%s doesn't exist, skipping.) \n", hgt_filenames[i] );
            }
        };


        int i_f[4] = {1, 0, 0, 0};

        if ( (s_ilat1 == s_ilat2) && (s_ilon1) == (s_ilon2) ) {
            // single file, i_f[0] only
        }
        else if (s_ilat1 != s_ilat2) {
            if (s_ilon1 != s_ilon2) {
                // 4 files
                i_f[1] = i_f[2] = i_f[3] = 1;
            }
            else {
                // same lon
                i_f[2] = 1;
            }
        }
        else {
            // same lat
            i_f[1] = 1;
        };


        char hgt_filenames_concat[200];
        hgt_filenames_concat[0] = '\0';

        for (int i = 0; i < 4; i++) {
            if ( i_f[i] && f_present[i] ) {
                strcat(hgt_filenames_concat, hgt_filenames[i]);
                strcat(hgt_filenames_concat, " ");
            }
        };

        if (hgt_filenames_concat[0] == '\0') {
            //fprintf(stderr, "None of needed HGT files are present. Skipping (%d_%d_%d). \n", z_scale, tile_x, tile_y);
        }
        else {

            char s_cmd[400];


            double pixel_scale_x = (max_proj_x - min_proj_x) / (image_size);
            double pixel_scale_y = (max_proj_y - min_proj_y) / (image_size);

            min_proj_x -= pixel_scale_x;// * 3.0 / 2.0;
            min_proj_y -= pixel_scale_y;// * 3.0 / 2.0;
            max_proj_x += pixel_scale_x*2.0; // * 3.0 / 2.0;
            max_proj_y += pixel_scale_y*2.0; // * 3.0 / 2.0;

            image_size += 2 + 1; // result image size must be (2^N + 1) + 2 near rows

            sprintf(s_cmd, "gdalwarp -q -overwrite -r cubicspline -t_srs EPSG:3857 -te_srs EPSG:3857 -te %.11f, %.11f, %.11f, %.11f -ts %d %d %s tile_%d_%d_%d_quad.tif",
                    min_proj_x, min_proj_y, max_proj_x, max_proj_y, image_size, image_size, hgt_filenames_concat, z_scale, tile_x, tile_y);

            system(s_cmd);

        };

    };

    return 0;
}
