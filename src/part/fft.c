#include <math.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Complex Complex;

struct Complex {
    double re;
    double im;
};

inline Complex add(const Complex a, const Complex b) {
    return (Complex){
        .re = a.re + b.re,
        .im = a.im + b.im,
    };
}

inline Complex sub(const Complex a, const Complex b) {
    return (Complex){
        .re = a.re - b.re,
        .im = a.im - b.im,
    };
}

inline Complex mul(const Complex a, const Complex b) {
    return (Complex){
        .re = a.re * b.re - a.im * b.im,
        .im = a.re * b.im + b.re * a.im,
    };
}

typedef Complex FftComplex[16];

void show(Complex points[], const size_t len) {
    // printf("Stage status:\n");
    for (int k = 0; k < len; k++) {
        printf("X_%.2d: %8.4f + j%8.4f\n", k, points[k].re, points[k].im);
    }
}

void bit_rev(FftComplex *X, const size_t nBit) {
    size_t n = 1 << nBit;
    for (size_t k = 0; k < n; ++k) {
        size_t rev_k = 0;
        for (int bit = 0; bit < nBit; ++bit) {
            rev_k <<= 1;
            rev_k |= (k >> bit) & 1;
        }
        if (k > rev_k) {
            continue;
        }
        const Complex swap = *X[k];
        *X[k] = *X[rev_k];
        *X[rev_k] = swap;
    }
    // show(X, 16);
}

void fft(FftComplex *X) {
    FftComplex W = {};
    const size_t n = sizeof(FftComplex) / sizeof(Complex);
    const size_t nBit = 32 - __builtin_clz(n - 1);
    bit_rev(X, nBit);

    for (int k = 0; k < n; ++k) {
        W[k] = (Complex){
            .re = cos(-2 * M_PI * k / (int) n),
            .im = sin(-2 * M_PI * k / (int) n)
        };
    }

    for (size_t stage = 0; stage < nBit; ++stage) {
        const size_t nStep = 1 << stage;
        const size_t nPatch = (int) (n) / (nStep << 1);
        for (size_t offset = 0; offset < n; offset += (nStep << 1)) {
            for (int step = 0; step < nStep; step++) {
                Complex *X1 = X[offset + step];
                Complex *X2 = X[offset + step + nStep];
                Complex u = *X1;
                Complex v = mul(*X2, W[step * nPatch]);
                *X1 = add(u, v);
                *X2 = sub(u, v);
            }
        }
    }
}

void ift(FftComplex *X) {
    FftComplex W = {};
    const size_t n = sizeof(FftComplex) / sizeof(Complex);
    const size_t nBit = 32 - __builtin_clz(n - 1);
    bit_rev(X, nBit);

    for (int k = 0; k < n; ++k) {
        W[k] = (Complex){
            cos(2 * M_PI * k / (int) (n)),
            sin(2 * M_PI * k / (int) (n))
        };
    }

    for (size_t stage = 0; stage < nBit; ++stage) {
        const size_t nStep = 1 << stage;
        const size_t nPatch = (int) (n) / (nStep << 1);
        for (size_t offset = 0; offset < n; offset += (nStep << 1)) {
            for (int step = 0; step < nStep; step++) {
                Complex *X1 = X[offset + step];
                Complex *X2 = X[offset + step + nStep];
                Complex u = *X1;
                Complex v = mul(*X2, W[step * nPatch]);
                *X1 = add(u, v);
                *X2 = sub(u, v);
            }
        }
    }

    for (size_t i = 0; i < 16; ++i) {
        X[i]->re /= n;
        X[i]->im /= n;
    }
}
