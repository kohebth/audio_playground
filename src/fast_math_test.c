#include <fast_lut.h>
#include <fast_math.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define N 100

float *x;

int good = 0;

int bad = 0;

uint64_t fast_time_total = 0;

uint64_t norm_time_total = 0;

float tmp = 0.0;

float random_float() {
    const static double scale = 1e8;
    return (rand() % 2 == 0 ? -1.0 : 1.0) * (rand() % 100) + (double) (rand() % (int) scale) / scale;
}

void random_float_many(bool positive) {
    for (int i = 0; i < N; ++i) {
        x[i] = random_float();
        if (positive) x[i] = fast_abs(x[i]);
    }
    x[0] = 1.086654424667f;
}

inline float fast_invsqrt(float x) {
    union {
        float f;
        unsigned int i;
    } bits = {x};
    bits.i = 0x5f3759df - (bits.i >> 1);
    float y = bits.f;
    y = y * (1.5f - 0.5f * x * y * y);
    return y * (1.5f - 0.5f * x * y * y);
}

#define test_speed_m(func) \
void test_##func() { \
    volatile float tmp = 0.0; \
    struct timespec tps, tpe; \
    clock_gettime(CLOCK_REALTIME, &tps); \
    for (int i = 0; i < N; ++i) { \
        tmp += func(x[i]); \
    } \
    clock_gettime(CLOCK_REALTIME, &tpe); \
    printf("%-15s: %lu.%09lus\t%0.12f\n", #func, tpe.tv_sec-tps.tv_sec, tpe.tv_nsec-tps.tv_nsec, tmp); \
}

void test_accuracy(float (*fast_call)(float), float (*norm_call)(float), float error_rate, float min_x, float max_x) {
    int count = 0;
    float fast, norm, case_error, cast_error_rate;
    for (int i = 0; i < N; ++i) {
        if (min_x > x[i] || max_x < x[i]) continue;
        fast = fast_call(x[i]);
        norm = norm_call(x[i]);
        case_error = norm - fast;
        cast_error_rate = fabs(case_error / norm);
        if (cast_error_rate > error_rate) {
            printf(
                "[count=%d] x=%0.12f\tNorm=%0.24f\tFast=%0.24f\tDiff=%0.24f\tDiff(%%)=%0.12f %%\n",
                count, x[i], norm, fast, (norm - fast), cast_error_rate * 100
            );
            exit(0);
        }
        count++;
    };
    printf("passed error < %f %\n", error_rate*100);
}

test_speed_m(log2f)

test_speed_m(exp2f)

test_speed_m(fast_exp2)

test_speed_m(fast_log2)

// Fast float to int conversion using magic number trick


// Fast 2^x for float x


int main() {
    // printf("%0.12f| %0.12f\n", fast_log2(0.999999f), log2f(0.999999f));
    // printf("%0.12f| %0.12f\n", fast_exp2(0.999999940395f), exp2f(0.999999940395f));
    // printf("%0.12f| %0.12f\n", fast_exp2f(0.499999940395f), exp2f(0.499999940395f));
    // return 0;
    x = (float *) malloc(N * sizeof(float));
    srand(time(0));
    int test_count = 10;

    while (test_count--) {
        // random_float_many(false);
        // test_accuracy(fast_exp2, exp2f, 6e-6, -200, 200);
        // test_fast_exp2();
        // test_exp2f();

        random_float_many(true);
        test_accuracy(fast_log2, log2f, 6e-5f, 0, INFINITY);
        test_log2f();
        test_fast_log2();
        puts("");
    }
    printf("good=%d, bad=%d\nFast Total=%d\nNorm Total=%d\nRate (Fast/Norm)=%lf\n", good, bad, fast_time_total,
           norm_time_total, 1.0 * fast_time_total / norm_time_total);
    free(x);
}
