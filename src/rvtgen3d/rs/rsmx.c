#include "rsmx.h"

#include <string.h>

#include <math.h>

#include <stdlib.h>

#include "rsdebug.h"



void rs_mx_identity(rs_mx_t mx) {

    mx[0] = 1;
    mx[1] = 0;
    mx[2] = 0;
    mx[3] = 0;

    mx[4] = 0;
    mx[5] = 1;
    mx[6] = 0;
    mx[7] = 0;

    mx[8] = 0;
    mx[9] = 0;
    mx[10] = 1;
    mx[11] = 0;

    mx[12] = 0;
    mx[13] = 0;
    mx[14] = 0;
    mx[15] = 1;

};



void rs_mx_ortho(rs_mx_t mx, float left, float right, float bottom, float top) {

    mx[0] = 2.0 / (right - left);
    mx[1] = 0;
    mx[2] = 0;
    mx[3] = 0;

    mx[4] = 0;
    mx[5] = 2.0 / (top - bottom);
    mx[6] = 0;
    mx[7] = 0;

    mx[8] = 0;
    mx[9] = 0;
    mx[10] = -2.0 / (128.0); // -2.0/(far-near)
    mx[11] = 0;

    mx[12] = - (right+left)/(right-left);
    mx[13] = - (top+bottom)/(top-bottom);
    mx[14] = 0.0;
    mx[15] = 1.0;

};



void rs_mx_frustum(rs_mx_t mx, float left, float right, float bottom, float top, float near1, float far1) {

    mx[0] = 2.0 / (right - left);
    mx[1] = 0;
    mx[2] = 0;
    mx[3] = 0;

    mx[4] = 0;
    mx[5] = 2.0 / (top - bottom);
    mx[6] = 0;
    mx[7] = 0;

    mx[8] = - (right+left)/(right-left);
    mx[9] = - (top+bottom)/(top-bottom);
    mx[10] = - (far1+near1)/(far1-near1);
    mx[11] = -1.0;

    mx[12] = 0.0;
    mx[13] = 0.0;
    mx[14] = - 2.0*far1*near1/(far1-near1);
    mx[15] = 1.0;

};




void rs_mx_mult_adv(rs_mx_t dest, rs_mx_t src1, rs_mx_t src2) {
    rs_mx_t res;
    memset(res, 0, 16*sizeof(float));
    int col, row, i;

    for (col = 0; col < 4; col++) {
        for (row = 0; row < 4; row++) {
             for (i = 0; i < 4; i++) {
                 res[col*4 + row] += src1[i*4 + row] * src2[col*4 + i];
             };
        };
    };

    memcpy(dest, res, 16*4);
};

void rs_mx_mult(rs_mx_t dest, rs_mx_t src) {
    rs_mx_mult_adv(dest, dest, src);
};


void rs_mx3_mult_adv(rs_mx3_t dest, rs_mx3_t src1, rs_mx3_t src2) {
    rs_mx3_t res;

    memset(res, 0, 9*4);

    int col;
    int row;
    int i;

    for (col = 0; col < 3; col++) { // col
        for (row = 0; row < 3; row++) { // row
             for (i = 0; i < 3; i++) { // pos in col/row
                 res[col*3 + row] += src1[i*3 + row] * src2[col*3 + i];
             };
        };
    };

    memcpy(dest, res, 9*4);
};

void rs_mx3_mult(rs_mx3_t dest, rs_mx3_t src) {
    rs_mx3_mult_adv(dest, dest, src);
};

void rs_mx_rotate(rs_mx_t mx, float angle, float x, float y, float z) {

    float c = cos(angle);
    float s = sin(angle);

    rs_mx_t mrot = {
        x*x*(1.0f-c)+c,      x*y*(1.0f-c)+z*s,        x*z*(1.0f-c)-y*s,    0,
        x*y*(1.0f-c)-z*s,    y*y*(1.0f-c)+c,          y*z*(1.0f-c)+x*s,    0,
        x*z*(1.0f-c)+y*s,    y*z*(1.0f-c)-x*s,        z*z*(1.0f-c)+c,      0,
        0,                  0,                      0,                  1
    };

    rs_mx_mult( mx, mrot );

};

void rs_mx_scale(rs_mx_t mx, float s) {

    rs_mx_scale_adv(mx, s, s, s);

};

void rs_mx_scale_adv(rs_mx_t mx, float x, float y, float z) {

    rs_mx_t mscale = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1.0
    };

    rs_mx_mult(mx, mscale);

};


void rs_mx_translate(rs_mx_t mx, float x, float y, float z) {

    rs_mx_t mtr = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };

    rs_mx_mult(mx, mtr);

};

void rs_mx_look_at(rs_mx_t mx, rs_vec3_t eye, rs_vec3_t center, rs_vec3_t up) {

    rs_vec3_t f = rs_vec3_normalize( rs_vec3_sub(center, eye) );
    up = rs_vec3_normalize(up);

    rs_vec3_t s = rs_vec3_cross(f, up);

    rs_vec3_t u = rs_vec3_cross( rs_vec3_normalize(s), f );

    rs_mx4_t m = {
        s.x, u.x, -f.x, 0.0,
        s.y, u.y, -f.y, 0.0,
        s.z, u.z, -f.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    };

    rs_mx_copy(mx, m);
    rs_mx_translate(mx, -eye.x, -eye.y, -eye.z);

};




void rs_mx_copy(rs_mx_t dest, rs_mx_t src) {
    memcpy(dest, src, sizeof(rs_mx_t) );
};

void rs_mx3_copy(rs_mx3_t dest, rs_mx3_t src) {
    memcpy(dest, src, sizeof(rs_mx3_t) );
};



void rs_mx_minor(rs_mx3_t dest, rs_mx4_t src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];

    dest[3] = src[4];
    dest[4] = src[5];
    dest[5] = src[6];

    dest[6] = src[8];
    dest[7] = src[9];
    dest[8] = src[10];
};

float rs_mx3_det(rs_mx3_t mx) {

    /*

    // WRONG:
    a b c
    d e f
    g h i

    // RIGHT: OpenGL
    a d g
    b e h
    c f i

    // useful for both:
    det (A) = aei + bfg + cdh - afh - bdi - ceg.

    */


    float a = mx[0];
    float b = mx[1];
    float c = mx[2];


    float d = mx[3];
    float e = mx[4];
    float f = mx[5];

    float g = mx[6];
    float h = mx[7];
    float i = mx[8];

    return (a*e*i + b*f*g + c*d*h) - a*f*h - b*d*i - c*e*g;


};

void rs_mx3_inv(rs_mx3_t dest, rs_mx3_t src) {

    float det = rs_mx3_det(src);

    rs_mx3_cofactor(dest, src);
    rs_mx3_transp(dest, dest);

    int i;
    for (i = 0; i < 9; i++) {
        dest[i] = dest[i] / det;
    };

};



void rs_mx_inv(rs_mx_t inv, rs_mx_t m) {


    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det != 0) {
        det = 1.0 / det;
        int i;
        for (i = 0; i < 16; i++) {
            inv[i] = inv[i] * det;
        };
    };
};



void rs_mx3_normal_matrix_from_modelview(rs_mx3_t dest, rs_mx4_t modelview) {

    rs_mx3_t src;
    rs_mx_minor(src, modelview);

    float det = rs_mx3_det(src);

    rs_mx3_cofactor(dest, src);

    int i;
    for (i = 0; i < 9; i++) {
        dest[i] = dest[i] / det;
    };

};

void rs_mx3_cofactor(rs_mx3_t cof, rs_mx3_t mx) {

    /*

    0 3 6
    1 4 7
    2 5 8


    +|4 7|  -|1 7|  +|1 4|
     |5 8|   |2 8|   |2 5|

    -|3 6|  +|0 6|  -|0 3|
     |5 8|   |2 8|   |2 5|

    +|3 6|  -|0 6|  +|0 3|
     |4 7|   |1 7|   |1 4|

    */

    cof[0] = + (mx[4]*mx[8] - mx[5]*mx[7]);
    cof[1] = - (mx[3]*mx[8] - mx[5]*mx[6]);
    cof[2] = + (mx[3]*mx[7] - mx[4]*mx[6]);

    cof[3] = - (mx[1]*mx[8] - mx[2]*mx[7]);
    cof[4] = + (mx[0]*mx[8] - mx[2]*mx[6]);
    cof[5] = - (mx[0]*mx[7] - mx[1]*mx[6]);

    cof[6] = + (mx[1]*mx[5] - mx[2]*mx[4]);
    cof[7] = - (mx[0]*mx[5] - mx[2]*mx[3]);
    cof[8] = + (mx[0]*mx[4] - mx[1]*mx[3]);


};



void rs_mx3_transp(rs_mx3_t dest2, rs_mx3_t src) {

    rs_mx3_t dest;

    dest[0] = src[0];
    dest[1] = src[3];
    dest[2] = src[6];

    dest[3] = src[1];
    dest[4] = src[4];
    dest[5] = src[7];

    dest[6] = src[2];
    dest[7] = src[5];
    dest[8] = src[8];

    rs_mx_copy(dest2, dest);

};

void rs_mx4_transp(rs_mx4_t dest2, rs_mx4_t src) {

    rs_mx4_t dest;

    dest[0] = src[0];
    dest[1] = src[4];
    dest[2] = src[8];
    dest[3] = src[12];

    dest[4] = src[1];
    dest[5] = src[5];
    dest[6] = src[9];
    dest[7] = src[13];

    dest[8] = src[2];
    dest[9] = src[6];
    dest[10] = src[10];
    dest[11] = src[14];

    dest[12] = src[3];
    dest[13] = src[7];
    dest[14] = src[11];
    dest[15] = src[15];

    rs_mx_copy(dest2, dest);

};


rs_vec3_t rs_vec3_sub(rs_vec3_t v1, rs_vec3_t v2) {
    return rs_vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
};

rs_vec3_t rs_vec3_add(rs_vec3_t v1, rs_vec3_t v2) {
    return rs_vec3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
};

rs_vec3_t rs_vec3_mult(rs_vec3_t v, float s) {
    return rs_vec3( v.x * s, v.y * s, v.z * s );
};


rs_vec2_t rs_vec2_add(rs_vec2_t v1, rs_vec2_t v2) {
    return rs_vec2( v1.x + v2.x, v1.y + v2.y );
};

rs_vec2_t rs_vec2_sub(rs_vec2_t v1, rs_vec2_t v2) {
    return rs_vec2(v1.x - v2.x, v1.y - v2.y);
};


rs_vec2_t rs_vec2_mult(rs_vec2_t v, float s) {
    return rs_vec2( v.x * s, v.y * s );
};


rs_vec4_t rs_vec4_sub(rs_vec4_t v1, rs_vec4_t v2) {
    rs_vec4_t dest;
    dest.x = v1.x - v2.x;
    dest.y = v1.y - v2.y;
    dest.z = v1.z - v2.z;
    dest.w = v1.z - v2.w;
    return dest;
};

rs_vec4_t rs_vec4_mult(rs_vec4_t v, float s) {
    return rs_vec4( v.x*s, v.y*s, v.z*s, v.w*s );
};


rs_vec4_t rs_vec4_add(rs_vec4_t v1, rs_vec4_t v2) {
    rs_vec4_t dest;
    dest.x = v1.x + v2.x;
    dest.y = v1.y + v2.y;
    dest.z = v1.z + v2.z;
    dest.w = v1.z + v2.w;
    return dest;
};

rs_vec4_t rs_vec4_quat(float angle, rs_vec3_t axis) {

    rs_vec4_t q;

    float s = sin(0.5*angle);
    q.x = s*axis.x;
    q.y = s*axis.y;
    q.z = s*axis.z;
    q.w = cos(0.5*angle);

    return q;

};


rs_vec4_t rs_vec4(float x, float y, float z, float w) {
    rs_vec4_t r;
    r.x = x;
    r.y = y;
    r.z = z;
    r.w = w;
    return r;
};

rs_vec3_t rs_vec3(float x, float y, float z) {
    rs_vec3_t r;
    r.x = x;
    r.y = y;
    r.z = z;
    return r;
};


rs_vec2_t rs_vec2(float x, float y) {
    rs_vec2_t r;
    r.x = x;
    r.y = y;
    return r;
};




float rs_vec4_length_sqr(rs_vec4_t src) {
    return src.x*src.x + src.y*src.y + src.z*src.z + src.w*src.w;
};

float rs_vec3_length_sqr(rs_vec3_t src) {
    return src.x*src.x + src.y*src.y + src.z*src.z;
};

float rs_vec2_length_sqr(rs_vec2_t src) {
    return src.x*src.x + src.y*src.y;
};



float rs_vec4_length(rs_vec4_t v) {
    return sqrtf( rs_vec4_length_sqr(v) );
};

float rs_vec3_length(rs_vec3_t v) {
    return sqrtf( rs_vec3_length_sqr(v) );
};

float rs_vec2_length(rs_vec2_t v) {
    return sqrtf( rs_vec2_length_sqr(v) );
};

float rs_vec2_azimuth( rs_vec2_t v_from, rs_vec2_t v_to ) {
    return atan2( v_to.y - v_from.y, v_to.x - v_from.x );
};


rs_vec3_t rs_vec3_normalize(rs_vec3_t v) {
    float s = rs_vec3_length(v);
    if (s > 0.00001) {
        return rs_vec3( v.x / s, v.y / s, v.z / s );
    }
    return rs_vec3(0.0, 0.0, 0.0);
};


rs_vec2_t rs_vec2_normalize(rs_vec2_t v) {
    float s = rs_vec2_length(v);
    if (s > 0.00001) {
        return rs_vec2( v.x / s, v.y / s );
    }
    return rs_vec2(0.0, 0.0);
};


float rs_vec4_dot(rs_vec4_t v1, rs_vec4_t v2) {
    return ( (v1.x) * (v2.x) ) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
};

float rs_vec3_dot(rs_vec3_t v1, rs_vec3_t v2) {
    return (v1.x) * (v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
};

float rs_vec2_dot(rs_vec2_t v1, rs_vec2_t v2) {
    return (v1.x) * (v2.x) + (v1.y * v2.y);
};




rs_vec3_t rs_vec3_cross(rs_vec3_t u, rs_vec3_t v) {
    rs_vec3_t d;

    d.x = u.y * v.z - u.z * v.y;
    d.y = u.z * v.x - u.x * v.z;
    d.z = u.x * v.y - u.y * v.x;

    return d;

};


rs_vec4_t rs_mx_mult_vec(rs_mx_t mx, rs_vec4_t v) {

    rs_vec4_t d;

    d.x = mx[0]*v.x + mx[4]*v.y + mx[ 8]*v.z + mx[12]*v.w;
    d.y = mx[1]*v.x + mx[5]*v.y + mx[ 9]*v.z + mx[13]*v.w;
    d.z = mx[2]*v.x + mx[6]*v.y + mx[10]*v.z + mx[14]*v.w;
    d.w = mx[3]*v.x + mx[7]*v.y + mx[11]*v.z + mx[15]*v.w;

    return d;

};


float rs_vec4_angle(rs_vec4_t u, rs_vec4_t v) {
    return rs_vec4_dot(u, v) / (rs_vec4_length(u) * rs_vec4_length(v) );
};

float rs_vec3_cos_angle(rs_vec3_t u, rs_vec3_t v) {
    float ret = rs_vec3_dot(u, v) / (rs_vec3_length(u) * rs_vec3_length(v) );
    return ret;
};

float rs_vec3_distance_sqr(rs_vec3_t u, rs_vec3_t v) {
    return rs_vec3_length_sqr( rs_vec3(u.x - v.x, u.y - v.y, u.z - v.z) );
};

float rs_vec3_distance(rs_vec3_t u, rs_vec3_t v) {
    return rs_vec3_length( rs_vec3(u.x - v.x, u.y - v.y, u.z - v.z) );
};

float rs_vec3_axis_distance(rs_vec3_t u, rs_vec3_t v) {
    float dx = fabs(u.x - v.x);
    float dy = fabs(u.y - v.y);
    float dz = fabs(u.z - v.z);
    return rs_max( rs_max(dx, dy), dz );
};


float rs_vec2_distance_sqr(rs_vec2_t u, rs_vec2_t v) {
    return rs_vec2_length_sqr( rs_vec2(u.x - v.x, u.y - v.y) );
};

float rs_vec2_distance(rs_vec2_t u, rs_vec2_t v) {
    return rs_vec2_length( rs_vec2(u.x - v.x, u.y - v.y) );
};


float rs_clamp(float x, float min1, float max1) {
    if (x < min1) {
        return min1;
    };
    if (x > max1) {
        return max1;
    };
    return x;
};

int rs_clamp_i(int x, int min1, int max1) {
    if (x < min1) {
        return min1;
    };
    if (x > max1) {
        return max1;
    };
    return x;
};

float rs_smoothstep(float edge0, float edge1, float x) {

    float t;
    t = rs_clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);

};

int rs_periodical_clamp_i(int x, int min1, int max1) {

    while (x < min1) {
        x += (max1 - min1);
    };

    x = (x-min1)%(max1-min1) + min1;

    return x;
};

float rs_periodical_clamp(float x, float min1, float max1) {

    while (x < min1) {
        x += (max1 - min1);
    };

    while (x > max1) {
        x -= (max1 - min1);
    };

    return x;
};



float rs_exp_interpolate(float v_from, float v_to, float dt) {
    return v_from + ( v_to - v_from ) * ( 1.0 - exp(-dt/1.0) );
};

float rs_short_angle_dist(float a0, float a1) {

    float d = rs_cyclic_mod_f(a1 - a0, 2.0*M_PI);

    return rs_cyclic_mod_f( (2.0*d), 2.0*M_PI ) - d;

};


float rs_exp_interpolate_angle(float a_from, float a_to, float dt) {

    return a_from + rs_short_angle_dist(a_from, a_to)*( 1.0 - exp(-dt/1.0) );

};

rs_vec3_t rs_vec3_exp_interpolate(rs_vec3_t v_from, rs_vec3_t v_to, float dt) {
    return rs_vec3(v_from.x + ( v_to.x - v_from.x ) * ( 1.0 - exp(-dt/1.0) ),
                    v_from.y + ( v_to.y - v_from.y ) * ( 1.0 - exp(-dt/1.0) ),
                    v_from.z + ( v_to.z - v_from.z ) * ( 1.0 - exp(-dt/1.0) ) );
};

rs_vec3_t rs_vec3_exp_interpolate2(rs_vec3_t v_from, rs_vec3_t v_to, float dt) {
    return rs_vec3(v_from.x + ( v_to.x - v_from.x ) * ( 1.0 - exp(-dt/1.0) ),
                    v_from.y + ( v_to.y - v_from.y ) * ( 1.0 - exp(-dt/1.0) ),
                    v_from.z );
};

float rs_max(float x, float y) {
    return x > y ? x : y;
};

float rs_min(float x, float y) {
    return x < y ? x : y;
};

int rs_max_i(int x, int y) {
    return x > y ? x : y;
};

int rs_min_i(int x, int y) {
    return x < y ? x : y;
};


float rs_sign(float f) {
    return (f >= 0.0) ? 1.0 : -1.0;
};

float rs_pow(float f, float p) {
    return rs_sign(f) * pow( fabs(f), p );
};

float rs_clamp_angle(float f) { // !!!! only one iteration
    if (f > 2.0*M_PI) {
        return f - 2.0*M_PI;
    };

    if (f < -2.0*M_PI) {
        return f + 2.0*M_PI;
    };

    return f;
};


rs_vec3_t rs_vec3_bezier3( rs_vec3_t v0, rs_vec3_t v1, rs_vec3_t v2, rs_vec3_t v3, float t ) {

    return rs_vec3(
                   RS_CUBE((1.0-t))*v0.x + 3.0*t*RS_SQR(1.0-t)*v1.x + 3.0*RS_SQR(t)*(1.0-t)*v2.x + RS_CUBE(t)*v3.x,
                   RS_CUBE((1.0-t))*v0.y + 3.0*t*RS_SQR(1.0-t)*v1.y + 3.0*RS_SQR(t)*(1.0-t)*v2.y + RS_CUBE(t)*v3.y,
                   RS_CUBE((1.0-t))*v0.z + 3.0*t*RS_SQR(1.0-t)*v1.z + 3.0*RS_SQR(t)*(1.0-t)*v2.z + RS_CUBE(t)*v3.z
                   );
};

rs_vec3_t rs_vec3_bezier2( rs_vec3_t v0, rs_vec3_t v1, rs_vec3_t v2, float t ) {

    return rs_vec3(
                   RS_SQR((1.0-t))*v0.x + 2.0*t*(1.0-t)*v1.x + RS_SQR(t)*v2.x,
                   RS_SQR((1.0-t))*v0.y + 2.0*t*(1.0-t)*v1.y + RS_SQR(t)*v2.y,
                   RS_SQR((1.0-t))*v0.z + 2.0*t*(1.0-t)*v1.z + RS_SQR(t)*v2.z
                   );
};


rs_vec3_t rs_vec3_linear( rs_vec3_t v0, rs_vec3_t v1, float t ) {

    return rs_vec3(
                   ((1.0-t))*v0.x + (t)*v1.x,
                   ((1.0-t))*v0.y + (t)*v1.y,
                   ((1.0-t))*v0.z + (t)*v1.z
                   );
};

rs_vec2_t rs_vec2_linear( rs_vec2_t v0, rs_vec2_t v1, float t ) {

    return rs_vec2(
                   ((1.0-t))*v0.x + (t)*v1.x,
                   ((1.0-t))*v0.y + (t)*v1.y
                   );
};

void rs_mx_linear( rs_mx_t dest, rs_mx_t src1, rs_mx_t src2, float pos ) {
    for (int i = 0; i < 16; i++) {
        dest[i] = (1.0-pos)*src1[i] + pos*src2[i];
    };
};

int rs_cyclic_mod_i(int value, int base) {

    if (value < 0) {
        int k = -value/base + 1;
        value += base * k;
    };

    return value % base;

};


float rs_cyclic_mod_f(float value, float base) {

    if (value < 0) {
        int k = (int)(-value/base) + 1;
        value += base * k;
    };

    return fmod(value, base);

};



unsigned int rs_log2_i (unsigned int i) {
    if (i == 0) {
        return 0;
    };
    if (i == 1) {
        return 0;
    };
    unsigned int r = 0;
    while (i >>= 1) {
        r++;
    }
    return r;
};


float rs_spline(float v0, float v1, float v2, float v3, float t) {
    return (rs_pow(1.0-t, 3)*(v0) + 3.0*RS_SQR(1.0-t)*t*(v1) + 3.0*(1.0-t)*RS_SQR(t)*(v2) + rs_pow(t, 3)*(v3));
};


float rs_rand() {

    return 1.0 / 1024.0 * ( rand() % 1024 );

};


