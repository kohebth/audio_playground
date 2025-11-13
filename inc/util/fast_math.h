#ifndef TANH_H
#define TANH_H

#include <stdint.h>

typedef int32_t i32;
typedef float f32;

typedef union {
    f32 value;
    i32 dword;
} ieee754_float32;

#define fast_abs(a) ((a) > 0 ? (a) : -(a))

#define fast_max(a,b) ((a) > (b) ? (a) : (b))

#define fast_min(a,b) ((a) < (b) ? (a) : (b))

#define fast_clamp(x,a,b) ({        \
    int32_t _x = (x);               \
    int32_t _l = (a);               \
    int32_t _ml = (_x - _l) >> 31;  \
    _x = _x ^ ((_x ^ _l) & _ml);    \
    int32_t _h = (b);               \
    int32_t _mh = (_x - _h) >> 31;  \
    _h ^ ((_x ^ _h) & _mh);         \
})

#define fast_clamp_min(x,low) ({    \
    int32_t _x = (x);               \
    int32_t _l = (low);             \
    int32_t _m = (_x - _l) >> 31;   \
    _x ^ ((_x ^ _l) & _m);          \
})

#define fast_clamp_max(x,high) ({   \
    int32_t _x = (x);               \
    int32_t _h = (high);            \
    int32_t _m = (_x - _h) >> 31;   \
    _h ^ ((_x ^ _h) & _m);          \
})

void init_FastMath(void);

float fast_log2(float x);

float fast_exp2(float x);

float fast_sigmoid(float x);

float fast_tanh_safe(float x);

float fast_ipol_cubic4(float xm1, float x0, float x1, float x2, float mu);

float fast_ipol_linear(float x0, float x1, float mu);

float fast_tanh(float x);

float fast_atan(float x);

float fast_db2ratio(float val_dB);

float fast_freq2angle(float f);

float fast_time2angle(float t);

float fast_recip(float x);

#endif //TANH_H
