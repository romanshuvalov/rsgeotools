#ifndef RS_NOISE_H
#define RS_NOISE_H

float rs_perlin(float i, float j);
float rs_quad_noise(float i, float j);

float rs_noise(int x, int y);
float rs_noise_for_perlin(int x, int y);

float rs_new_perlin_noise(float x, float y);

void rs_perlin_configure(float freq, int octaves, float persistence, float seed, int tex_size, int shift_y);

typedef struct rs_perlin_conf_t {
    float freq;
    int octaves;
    float persistence;
    float seed;
    int period;
    int tex_size;
    int shift_y;
} rs_perlin_conf_t;

extern rs_perlin_conf_t rs_perlin_conf;

#endif
