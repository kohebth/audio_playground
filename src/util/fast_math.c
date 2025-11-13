#include <fast_lut.h>
#include <stdio.h>
#include <math.h>

#include <fast_math.h>
#include <stdint.h>

#define LN2 0x1.62e42fp-1f
#define LOG2E 0x1.715476p+0f

#define poly3(x, c0, c1, c2, c3) ((c0 * (x) + c1) * (x) + c2)

#define poly4(x, c0, c1, c2, c3) (((c0 * (x) + c1) * (x) + c2) * (x) + c3)

#define poly5(x, c0, c1, c2, c3, c4) ((((c0 * (x) + c1) * (x) + c2) * (x) + c3) * (x) + c4)

inline f32 fast_int2float(i32 x) {
    return (ieee754_float32){.dword = 0x4b400000 + x}.value - 0x1.8p+23f;
}

inline i32 fast_float2int(f32 x) {
    return (ieee754_float32){.value = 0x1.8p+23f + x}.dword - 0x4b400000;
}


// applied to [-2^31, 2^31]
static int32_t fast_f2i(const float x) {
    const ieee754_float32 px = {.value = x};
    int32_t e = ((px.dword >> 23) & 0xff) - 127;
    const int32_t i = (px.dword & 0x007fffff | 0x00800000) >> (23 - fast_max(e, - 1));
    return px.dword < 0 ? -i : i;
}

static int32_t fast_log2i(int32_t x) {
    static const int tab32[32] = {
        0, 9, 1, 10, 13, 21, 2, 29,
        11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7,
        19, 27, 23, 6, 26, 5, 4, 31
    };

    if (x <= 0) {
        return -127;
    }
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return tab32[(uint32_t) (x * 0x07c4acdd) >> 27];
}

// applied to [-2^31, 2^31]
static float fast_i2f(const int32_t x) {
    const int32_t y = fast_abs(x);
    const int32_t e = fast_log2i(y);
    const int32_t m = e > 23 ? 0 : (y - (1 << e)) << (23 - e);
    const ieee754_float32 r = {.dword = (x & 0x80000000) | (((e + 0x7F) << 23) & 0x7f800000) | (m & 0x007fffff)};
    // printf("r=%0.12f x=%d abs(x)=%d exp=%d mant=%d |\n", r.f, x, y, e, m);
    return r.value;
}

static float fast_recip_float32(ieee754_float32 x) {
    register float xx = x.value;
    x.dword = 0x7ef311c2u - x.dword; // Approximate 1/x = sign * (0xF311C2-mantissa) * 2^(127 - exponent)
    x.value = x.value * (2.0f - xx * x.value); // 1sd Newton-Raphson iteration
    return x.value * (2.0f - xx * x.value); // 2nd Newton-Raphson iteration
}

float fast_recip(register float x) {
    return fast_recip_float32((ieee754_float32){.value = x});
}

inline float fast_floor(volatile float x) {
    volatile ieee754_float32 x32 = {.value = x};
    volatile ieee754_float32 s32 = {.value = 0.0};
    s32.dword = (((x32.dword >> 31) & 1) * 0x7f) << 23;
    return x32.value - s32.value;
}

float fast_tanh_safe(float x) {
    float e = fast_exp2(2.0f * fabs(x));
    float y = 1.0f - 2.0f * fast_recip(e + 1.0f);
    return x >= 0 ? y : -y;
}

float fast_tanh(float x) {
    float y = 2.0 * fast_sigmoid(2.0 * fabs(x)) - 1.0;
    return x >= 0 ? y : -y;
}

float fast_ipol_cubic4(float xm1, float x0, float x1, float x2, float mu) {
    float a0 = -0.5 * xm1 + 1.5 * x0 - 1.5 * x1 + 0.5 * x2;
    float a1 = xm1 - 2.5 * x0 + 2.0 * x1 - 0.5 * x2;
    float a2 = -0.5 * xm1 + 0.5 * x1;
    float a3 = x0;
    return ((a0 * mu + a1) * mu + a2) * mu + a3;
}

float fast_ipol_linear(float x0, float x1, float mu) {
    return x0 + mu * (x1 - x0);
}

float fast_interpolate_sinc();

float fast_atan(const float x) {
    float ax = fabs(x);
    float result;
    if (ax > 1.0) {
        ax = fast_recip(ax);
        result = M_PI_2 - ax * (M_PI_4 - (ax - 1) * (0.2447 + 0.0663 * ax));
        return x < 0 ? -result : result;
    }
    result = ax * (M_PI_4 - (ax - 1) * (0.2447 + 0.0663 * ax));
    return x < 0 ? -result : result;
}

float fast_sigmoid(const float x) {
    return fast_recip(1.0 + fast_exp2(-x));
}

float fast_db2ratio(const float val_dB) {
    return fast_exp2(M_LOG10E * 0.05 * val_dB);
}

float fast_freq2angle(const float f) {
    return 2.0 * M_PI * f;
}

float fast_time2angle(const float t) {
    return 2.0 * M_PI * fast_recip(t);
}

float fast_exp2(float x) {
    const ieee754_float32 magic = {.value = 0x1.8p+23f, .dword = 0x4b400000};
    const float absx = fabs(x);

    if (absx >= 127.0f) {
        if (x >= 128.0f) {
            return INFINITY;
        }
        if (x < 0) {
            return 0.0f;
        }
    }

    const ieee754_float32 e32 = {.value = x - 0.5f + magic.value};
    const ieee754_float32 p2i = {.dword = (e32.dword - magic.dword + 0x7F) << 23};

    x -= e32.value - magic.value;

    // error rate: 5.483e-06
    const float exp2xf = poly5(
        x,
        0x1.ce9e48p-7f, 0x1.a1b0d2p-5f, 0x1.ef921ep-3f, 0x1.62c62ep-1f, 0x1.00005cp+0f
    );
    const float exp2xi = p2i.value;

    return exp2xf * exp2xi;
}

float fast_log2(float x) {
    if (x <= 0.0f) return (ieee754_float32){.dword = 0xffc00000}.value;

    const float sqrt2 = 0x1.6a09e6p+0f;

    ieee754_float32 x32 = {.value = x};
    ieee754_float32 e32 = {.dword = (x32.dword & 0x7f800000) - 0x3f800000};

    x32.dword = x32.dword - e32.dword;
    e32.dword >>= 23;

    // Normalize to [1/sqrt2, sqrt2)
    const static float x_ratio[2] = {1.0f, 0.5f};
    const int32_t idx = x32.value > sqrt2;
    x32.value *= x_ratio[idx];
    e32.dword += idx + 0x4b400000;

    const float t = x32.value - 1.0f;

    // error rate: 5.055e-05
    const float log2xf = poly5(
        t,
        0x1.0531b6p-2f, -0x1.9067fep-2f, 0x1.f0f1c8p-2f, -0x1.70ebf2p-1f, 0x1.715140p+0f
    ) * t;
    const float log2xi = e32.value - 0x1.8p+23f;

    return log2xi + log2xf;
}

float fast_invsqrt(const float x) {
    ieee754_float32 x32 = {.value = x};
    x32.dword = 0x5f3759df - (x32.dword >> 1);
    float y = x32.value;
    y = y * (1.5f - 0.5f * x * y * y);
    return y * (1.5f - 0.5f * x * y * y);
}
