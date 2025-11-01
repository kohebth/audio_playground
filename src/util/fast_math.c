#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include <stdlib.h>
#include <fast_math.h>

double fast_exp(double x) {
    // Approximate 2^f with a small polynomial
    x = M_LOG2E * x; // convert base e to base 2
    int32_t i = (int32_t)x;
    double f = x - i;

    const double
            c0 = 1.0000000000000000,
            c1 = 6.931471805599453e-1,
            c2 = 2.402265069591007e-1,
            c3 = 5.550410866482158e-2,
            c4 = 9.618129107628477e-3,
            c5 = 1.3333558146428441e-3,
            c6 = 1.5403530393381606e-4;
    const double two_pow_f = ((((((c6 * f + c5) * f + c4) * f + c3) * f + c2) * f + c1) * f + c0);

    // Build 2^i IEEE-754

    double two_pow_i;
    if (i < -1022)
        two_pow_i = 0.0;
    else if (i > 1023)
        two_pow_i = INFINITY;
    else {
        uint64_t bits = (uint64_t)(i + 1023) << 52;
        memcpy(&two_pow_i, &bits, sizeof(double));
    }
    return two_pow_f * two_pow_i;
}

double fast_tanh_safe(double x) {
    double e = fast_exp(2.0f * fabs(x));
    double y = 1.0f - 2.0f / (e + 1.0f);
    return x >= 0 ? y : -y;
}

double fast_tanh(double x) {
    double y = 2.0*fast_sigmoid(2.0*fabs(x)) - 1.0;
    return x >= 0 ? y : -y;
}

inline float fast_recipf(float x)
{
    int32_t i = *(int32_t *)&x;
    i = 0x7F000000 - i;
    return *(float *)&i;
}

inline double fast_recip(double x)
{
    uint64_t i = 0x7FE0000000000000LL - *(int64_t *)(&x);
    double y = *(double *)(&i);
    return y * (2.0 - x * y);
}

double fast_interpolate_cubic4(float xm1, float x0, float x1, float x2, double mu) {
    double a0 = -0.5 * xm1 + 1.5 * x0 - 1.5 * x1 + 0.5 * x2;
    double a1 = xm1 - 2.5 * x0 + 2.0 * x1 - 0.5 * x2;
    double a2 = -0.5 * xm1 + 0.5 * x1;
    double a3 = x0;
    return ((a0 * mu + a1) * mu + a2) * mu + a3;
}

double fast_interpolate_linear(float x0, float x1, double mu) {
    return x0 + mu * (x1 - x0);
}

double fast_interpolate_sinc();

double fast_atan(double x) {
    double ax = fabs(x);
    double result;
    if (ax > 1.0) {
        ax = fast_recip(ax);
        result = M_PI_2 - ax * (M_PI_4 - (ax - 1) * (0.2447 + 0.0663 * ax));
        return x < 0 ? -result: result;
    }
    result = ax * (M_PI_4 - (ax - 1) * (0.2447 + 0.0663 * ax));
    return x < 0 ? -result: result;
}

double fast_sigmoid(double ratio) {
    return fast_recip(1.0 + fast_exp(-ratio));
}
