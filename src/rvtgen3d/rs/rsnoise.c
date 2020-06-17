#include "rsnoise.h"

#include <math.h>

rs_perlin_conf_t rs_perlin_conf;

void rs_perlin_configure(float freq, int octaves, float persistence, float seed, int tex_size, int shift_y) {
    rs_perlin_conf.freq = freq;
    rs_perlin_conf.octaves = octaves;
    rs_perlin_conf.persistence = persistence;
    rs_perlin_conf.seed = seed;
    rs_perlin_conf.tex_size = tex_size;
    rs_perlin_conf.period = (int) (0.1 + roundf(freq) );
    rs_perlin_conf.shift_y = shift_y;
};

float rs_noise(int x, int y) {
    // from here, http://www.cplusplus.com/forum/general/85758/
    // koef. changed
    int n = x + y * 57 * 5;
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    float ret = 1.0 - ((float)t) * 0.931322574615478515625e-9;
    return ret;
};

float rs_noise_for_perlin(int x, int y) {

    x %= rs_perlin_conf.period;
    y %= rs_perlin_conf.period;

    // from here, http://www.cplusplus.com/forum/general/85758/
    // koef. changed
    int n = x + y * 57 * 5;
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - ((float)t) * 0.931322574615478515625e-9;
};


double rs_interpolate(double x, double y, double a) {

    float c = 0.5 + 0.5*cos( a * M_PI );

    return y*(1.0 - c) + x*c;

}

double rs_perlin_noise(double x, double y) {

    int Xint = (int)x;
    int Yint = (int)y;
    double Xfrac = x - Xint;
    double Yfrac = y - Yint;

    double n01= rs_noise_for_perlin(Xint-1, Yint-1);
    double n02= rs_noise_for_perlin(Xint+1, Yint-1);
    double n03= rs_noise_for_perlin(Xint-1, Yint+1);
    double n04= rs_noise_for_perlin(Xint+1, Yint+1);
    double n05= rs_noise_for_perlin(Xint-1, Yint);
    double n06= rs_noise_for_perlin(Xint+1, Yint);
    double n07= rs_noise_for_perlin(Xint, Yint-1);
    double n08= rs_noise_for_perlin(Xint, Yint+1);
    double n09= rs_noise_for_perlin(Xint, Yint);

    double n12= rs_noise_for_perlin(Xint+2, Yint-1);
    double n14= rs_noise_for_perlin(Xint+2, Yint+1);
    double n16= rs_noise_for_perlin(Xint+2, Yint);

    double n23= rs_noise_for_perlin(Xint-1, Yint+2);
    double n24= rs_noise_for_perlin(Xint+1, Yint+2);
    double n28= rs_noise_for_perlin(Xint, Yint+2);

    double n34= rs_noise_for_perlin(Xint+2, Yint+2);

    //find the noise values of the four corners
    double x0y0 = 0.0625*(n01+n02+n03+n04) + 0.125*(n05+n06+n07+n08) + 0.25*(n09);
    double x1y0 = 0.0625*(n07+n12+n08+n14) + 0.125*(n09+n16+n02+n04) + 0.25*(n06);
    double x0y1 = 0.0625*(n05+n06+n23+n24) + 0.125*(n03+n04+n09+n28) + 0.25*(n08);
    double x1y1 = 0.0625*(n09+n16+n28+n34) + 0.125*(n08+n14+n06+n24) + 0.25*(n04);

    //interpolate between those values according to the x and y fractions
    double v1 = rs_interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
    double v2 = rs_interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
    double fin = rs_interpolate(v1, v2, Yfrac);  //interpolate in y direction

    return fin;
}

float rs_perlin(float i, float j) {

    double t = 0.0f;
    double _amplitude = 1.0;


    int k;

    float amplitude_divider = 0.0;
    for (k = 0; k < rs_perlin_conf.octaves; k++) {
        amplitude_divider += _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
    };

    _amplitude = 1.0;

    float freq = rs_perlin_conf.freq;

    for(k = 0; k < rs_perlin_conf.octaves; k++)
    {
        t += rs_perlin_noise(j * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed, i * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed) * _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
        freq *= 2;
    }

    return t / amplitude_divider;
};

float rs_quad_noise(float i, float j) {

    double t = 0.0f;
    double _amplitude = 1.0;


    int k;

    float amplitude_divider = 0.0;
    for (k = 0; k < rs_perlin_conf.octaves; k++) {
        amplitude_divider += _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
    };

    _amplitude = 1.0;

    float freq = rs_perlin_conf.freq;

    for(k = 0; k < rs_perlin_conf.octaves; k++)
    {
        t += rs_noise(j * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed, i * freq / rs_perlin_conf.tex_size + rs_perlin_conf.seed) * _amplitude;
        _amplitude *= rs_perlin_conf.persistence;
        freq *= 2;
    }

    return t / amplitude_divider;
};



// ============================== NEW PERLIN =============================
// from here: http://freespace.virgin.net/hugo.elias/models/m_perlin.htm



float rs_new_noise(int x, int y) {
    int n = x + (y) * 57 + rs_perlin_conf.seed;
    n = (n<<13) ^ n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float SmoothNoise_1(float x, float y) {
    float corners = ( rs_new_noise(x-1, y-1)+rs_new_noise(x+1, y-1)+rs_new_noise(x-1, y+1)+rs_new_noise(x+1, y+1) ) / 16;
    float sides   = ( rs_new_noise(x-1, y)  +rs_new_noise(x+1, y)  +rs_new_noise(x, y-1)  +rs_new_noise(x, y+1) ) /  8;
    float center  =  rs_new_noise(x, y) / 4;
    return corners + sides + center;
}

float InterpolatedNoise_1(float x, float y) {

  int integer_X    = (int) (x);
  float fractional_X = x - integer_X;

  int integer_Y    = (int) (y);
  float fractional_Y = y - integer_Y;

  float v1 = SmoothNoise_1(integer_X,     integer_Y);
  float v2 = SmoothNoise_1(integer_X + 1, integer_Y);
  float v3 = SmoothNoise_1(integer_X,     integer_Y + 1);
  float v4 = SmoothNoise_1(integer_X + 1, integer_Y + 1);

  float i1 = rs_interpolate(v1 , v2 , fractional_X);
  float i2 = rs_interpolate(v3 , v4 , fractional_X);

  return rs_interpolate(i1 , i2 , fractional_Y);

};


float rs_new_perlin_noise(float x, float y) {

  float total = 0;
  float p = rs_perlin_conf.persistence;
  int n = rs_perlin_conf.octaves - 1;
  float frequency;
  float amplitude;

  int i;
  for (i = 0; i < n; i++)  {

      frequency = pow(2.0, i) * rs_perlin_conf.period / rs_perlin_conf.tex_size;
      amplitude = pow(p, i);

      total = total + InterpolatedNoise_1 ( (x  + 1.0*rs_perlin_conf.shift_y) * frequency, (y ) * frequency) * amplitude;

  }

  return total;

};


