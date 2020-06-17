#ifndef rs_mx_t_H_INCLUDED
#define rs_mx_t_H_INCLUDED

#include <inttypes.h>


#define RS_SQR(x) ((x)*(x))
#define RS_CUBE(x) ((x)*(x)*(x))


// aligned(16) for Bullet physics
typedef float __attribute__ ((aligned (16))) rs_mx_t[16];

typedef rs_mx_t __attribute__ ((aligned (16))) rs_mx4_t;
//typedef float *rs_mx_t_p;

typedef float rs_mx3_t[9];


typedef struct {
    float x;
    float y;
} rs_vec2_t;

typedef struct {
    int x;
    int y;
} rs_vec2i_t;

typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        struct {
            float r;
            float g;
            float b;
        };
        float v[3];
    };

} rs_vec3_t;


typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        struct {
            float r;
            float g;
            float b;
            float a;
        };
        float v[4];
    };

} rs_vec4_t;


typedef struct {
    union {
        struct {
            uint32_t x;
            uint32_t y;
            uint32_t z;
            uint32_t w;
        };
        uint32_t v[4];
    };

} rs_vec4i_t;




void rs_mx3_mult_vec3(rs_vec3_t dest, rs_mx3_t m, rs_vec3_t v);



void rs_mx_identity(rs_mx_t mx);

void rs_mx_ortho(rs_mx_t mx, float left, float right, float bottom, float top);
void rs_mx_frustum(rs_mx_t mx, float left, float right, float bottom, float top, float near, float far);

void rs_mx_mult_adv(rs_mx_t dest, rs_mx_t src1, rs_mx_t src2);
void rs_mx_mult(rs_mx_t dest, rs_mx_t src);

void rs_mx3_mult_adv(rs_mx3_t dest, rs_mx3_t src1, rs_mx3_t src2);
void rs_mx3_mult(rs_mx3_t dest, rs_mx3_t src);

void rs_mx_translate(rs_mx_t mx, float x, float y, float z);
void rs_mx_rotate(rs_mx_t mx, float angle, float x, float y, float z);
void rs_mx_scale(rs_mx_t mx, float s);
void rs_mx_scale_adv(rs_mx_t mx, float x, float y, float z);

void rs_mx_look_at(rs_mx_t mx, rs_vec3_t eye, rs_vec3_t center, rs_vec3_t up);


void rs_mx_copy(rs_mx_t dest, rs_mx_t src);
#define rs_mx4_copy(dest,src) rs_mx_copy(dest,src)
void rs_mx3_copy(rs_mx3_t dest, rs_mx3_t src);

void rs_mx_minor(rs_mx3_t dest, rs_mx4_t src);

float rs_mx3_det(rs_mx3_t mx);
float rs_mx4_det(rs_mx4_t mx);

void rs_mx3_inv(rs_mx3_t dest, rs_mx3_t src);
void rs_mx_inv(rs_mx_t inv, rs_mx_t m);

void rs_mx3_cofactor(rs_mx3_t cof, rs_mx3_t mx);
void rs_mx4_cofactor(rs_mx4_t cof, rs_mx4_t mx);

void rs_mx3_transp(rs_mx3_t dest, rs_mx3_t src);
void rs_mx4_transp(rs_mx4_t dest, rs_mx4_t src);

void rs_mx3_normal_matrix_from_modelview(rs_mx3_t dest, rs_mx4_t modelview);



rs_vec2_t rs_vec2(float x, float y);
rs_vec2_t rs_vec2_add(rs_vec2_t v1, rs_vec2_t v2);
rs_vec2_t rs_vec2_sub(rs_vec2_t v1, rs_vec2_t v2);
rs_vec2_t rs_vec2_mult(rs_vec2_t v, float s);
float rs_vec2_dot(rs_vec2_t v1, rs_vec2_t v2);
float rs_vec2_length_sqr(rs_vec2_t src);
float rs_vec2_length(rs_vec2_t v);
float rs_vec2_azimuth( rs_vec2_t v_from, rs_vec2_t v_to );

rs_vec3_t rs_vec3(float x, float y, float z);
rs_vec3_t rs_vec3_add(rs_vec3_t v1, rs_vec3_t v2);
rs_vec3_t rs_vec3_sub(rs_vec3_t v1, rs_vec3_t v2);
rs_vec3_t rs_vec3_mult(rs_vec3_t v, float s);
float rs_vec3_dot(rs_vec3_t v1, rs_vec3_t v2);
float rs_vec3_length_sqr(rs_vec3_t src);
float rs_vec3_length(rs_vec3_t v);


rs_vec4_t rs_vec4(float x, float y, float z, float w);
rs_vec4_t rs_vec4_add(rs_vec4_t v1, rs_vec4_t v2);
rs_vec4_t rs_vec4_sub(rs_vec4_t v1, rs_vec4_t v2);
rs_vec4_t rs_vec4_mult(rs_vec4_t v, float s);
float rs_vec4_dot(rs_vec4_t v1, rs_vec4_t v2);
float rs_vec4_length_sqr(rs_vec4_t src);
float rs_vec4_length(rs_vec4_t v);

rs_vec4_t rs_vec4_quat(float angle, rs_vec3_t axis);

rs_vec4_t rs_mx_mult_vec(rs_mx_t mx, rs_vec4_t v);

rs_vec3_t rs_vec3_cross(rs_vec3_t u, rs_vec3_t v);

rs_vec3_t rs_vec3_normalize(rs_vec3_t v);

float rs_vec4_angle(rs_vec4_t u, rs_vec4_t v);

float rs_vec3_cos_angle(rs_vec3_t u, rs_vec3_t v);


rs_vec2_t rs_vec2_normalize(rs_vec2_t v);

float rs_vec2_distance(rs_vec2_t u, rs_vec2_t v);
float rs_vec2_distance_sqr(rs_vec2_t u, rs_vec2_t v);

float rs_vec3_distance(rs_vec3_t u, rs_vec3_t v);
float rs_vec3_axis_distance(rs_vec3_t u, rs_vec3_t v);
float rs_vec3_distance_sqr(rs_vec3_t u, rs_vec3_t v);

float rs_clamp(float x, float min1, float max1);
int rs_clamp_i(int x, int min1, int max1);

float rs_smoothstep(float min1, float max1, float val);


float rs_periodical_clamp(float x, float min1, float max1);
int rs_periodical_clamp_i(int x, int min1, int max1);

float rs_max(float x, float y);
float rs_min(float x, float y);

int rs_max_i(int x, int y);
int rs_min_i(int x, int y);

float rs_sign(float f);
float rs_pow(float f, float p);

float rs_exp_interpolate(float v_from, float v_to, float dt);
float rs_exp_interpolate_angle(float a_from, float a_to, float dt);
rs_vec3_t rs_vec3_exp_interpolate(rs_vec3_t v_from, rs_vec3_t v_to, float dt);
rs_vec3_t rs_vec3_exp_interpolate2(rs_vec3_t v_from, rs_vec3_t v_to, float dt);

float rs_clamp_angle(float f);


int rs_cyclic_mod_i(int value, int base);
float rs_cyclic_mod_f(float value, float base);

rs_vec3_t rs_vec3_bezier3( rs_vec3_t v0, rs_vec3_t v1, rs_vec3_t v2, rs_vec3_t v3, float pos );
rs_vec3_t rs_vec3_bezier2( rs_vec3_t v0, rs_vec3_t v1, rs_vec3_t v2, float pos );

rs_vec3_t rs_vec3_linear( rs_vec3_t v0, rs_vec3_t v1, float pos );
rs_vec2_t rs_vec2_linear( rs_vec2_t v0, rs_vec2_t v1, float pos );

void rs_mx_linear( rs_mx_t dest, rs_mx_t src1, rs_mx_t src2, float pos );

unsigned int rs_log2_i (unsigned int i);

float rs_spline(float v0, float v1, float v2, float v3, float t);

float rs_short_angle_dist(float a0, float a1);

float rs_rand();

#endif // rs_mx_t_H_INCLUDED

