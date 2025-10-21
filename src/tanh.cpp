#include <cstdio>
#include <cmath>
#include <cstring>

#include <iostream>

#define LUT_SIZE 256

static inline double fast_exp(double x) {
    // Approximate 2^f with a small polynomial
    const double_t
        c0 = 1.0000000000000000,
        c1 = 6.931471805599453e-1,
        c2 = 2.402265069591007e-1,
        c3 = 5.550410866482158e-2,
        c4 = 9.618129107628477e-3,
        c5 = 1.3333558146428441e-3,
        c6 = 1.5403530393381606e-4;

    x = M_LOG2E * x; // convert base e to base 2
    double_t f = x - floor(x);
    // Build 2^i IEEE-754
    uint32_t qwords[2] = { 0x00, static_cast<uint32_t>(x + 1023) << 20 } ;
    double_t two_pow_i;
    memcpy(&two_pow_i, qwords, sizeof(double_t));
    double_t two_pow_f = ((((((c6 * f + c5) * f + c4) * f + c3) * f + c2) * f + c1) * f + c0);
    return two_pow_f * two_pow_i;
}

double fast_tanh(double x) {
    if (x > 32) {
        return 1.0f;
    }
    if (x < -32) {
        return -1.0f;
    }
    double e = exp2(2.0*fabs(x));
    // double e = fast_exp(2.0f * fabs(x));
    double y = 1.0f - 2.0f / (e + 1.0f);
    return x >= 0 ? y : -y;
}

void elapse(void (*func)(), std::string func_name) {
    long start, stop;
    start = clock();
    func();
    stop = clock();
    printf("f=%s time=%ldns\n", &func_name[0], stop - start);
}

void fast_tanh() {
    fast_tanh(10);
}

void std_tanh() {
    tanh(10);
}

void test_fst() {
    double xx_diff = 0.0;
    for (double x = -32.0; x < 32.0; x += 0.000001) {
        xx_diff += fast_tanh(x);
    }
    printf("%0.12f ", xx_diff);
}

void test_std() {
    double xx_diff = 0.0;
    for (double x = -32.0; x < 32.0; x += 0.000001) {
        xx_diff += tanh(x);
    }
    printf("%0.12f ", xx_diff);
}

int main() {
    elapse(fast_tanh, "fast_tanh");
    elapse(std_tanh, "std_tanh");

    elapse(test_std, "test_std");
    elapse(test_fst, "test_fst");
}
