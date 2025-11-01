#include <math.h>
#include <stddef.h>

typedef struct FIRFilter FIRFilter;

struct FIRFilter {
    double data[2048];
    size_t size;
};

void designLowpass(FIRFilter *h, float fc, float fs) {
    const int M = h->size;
    const double M_1 = M - 1;
    const float norm_fc = fc / fs;
    for (int n = 0; n < M; n++) {
        int m = n - M / 2;
        if (m == 0)
            h->data[n] = 2.0f * norm_fc;
        else
            h->data[n] = sinf(M_PI * 2.0f * norm_fc * m) * M_1_PI / m;
        h->data[n] *= 0.5f - 0.5f * cosf(M_PI * 2.0f * n / M_1); // Hann window
    }
}

void applyFIR(FIRFilter *x, FIRFilter *y, FIRFilter *h) {
    const int M = h->size;

    for (size_t n = 0; n < x->size; n++) {
        double acc = 0;
        for (int k = 0; k < M; k++) {
            if ((int)n - k >= 0)
                acc += x->data[n - k] * h->data[k];
        }
        y->data[n] = (float)acc;
    }
}
