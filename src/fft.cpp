#include <cmath>
#include <complex>
#include <iostream>

struct Complex {
    double re;
    double im;

    Complex operator +(const Complex &arg) const {
        return Complex{re + arg.re, im + arg.im};
    }

    Complex operator -(const Complex &arg) const {
        return Complex{re - arg.re, im - arg.im};
    }

    Complex operator *(const Complex &arg) const {
        return Complex{re * arg.re - im * arg.im, re * arg.im + arg.re * im};
    }
};

typedef Complex FftComplex[16];

// double fast_atan(double x) {
//     return M_PI_4 * x + 0.273 * x * (1 - fabs(x));
// }

void show(Complex points[], size_t len) {
    // printf("Stage status:\n");
    for (int k = 0; k < len; k++) {
        printf("X_%.2d: %8.4f + j%8.4f\n", k, points[k].re, points[k].im);
    }
}

void bit_rev(FftComplex &X, size_t nBit) {
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
        const Complex swap = X[k];
        X[k] = X[rev_k];
        X[rev_k] = swap;
    }
    // show(X, 16);
}

void fft(FftComplex &X) {
    FftComplex W = {};
    const size_t n = sizeof(FftComplex) / sizeof(Complex);
    const size_t nBit = 32 - __builtin_clz(n - 1);
    bit_rev(X, nBit);

    for (int k = 0; k < n; ++k) {
        W[k] = Complex{
            cos(-2 * M_PI * k / static_cast<int>(n)),
            sin(-2 * M_PI * k / static_cast<int>(n))
        };
    }

    for (size_t stage = 0; stage < nBit; ++stage) {
        const size_t nStep = 1 << stage;
        const size_t nPatch = static_cast<int>(n) / (nStep << 1);
        for (size_t offset = 0; offset < n; offset += (nStep << 1)) {
            for (int step = 0; step < nStep; step++) {
                Complex *X1 = X + offset + step;
                Complex *X2 = X + offset + step + nStep;
                Complex u = *X1;
                Complex v = *X2 * W[step * nPatch];
                *X1 = u + v;
                *X2 = u - v;
            }
        }
    }
}

void ift(FftComplex &X) {
    FftComplex W = {};
    const size_t n = sizeof(FftComplex) / sizeof(Complex);
    const size_t nBit = 32 - __builtin_clz(n - 1);
    bit_rev(X, nBit);

    for (int k = 0; k < n; ++k) {
        W[k] = Complex{
            cos(2 * M_PI * k / static_cast<int>(n)),
            sin(2 * M_PI * k / static_cast<int>(n))
        };
    }

    for (size_t stage = 0; stage < nBit; ++stage) {
        const size_t nStep = 1 << stage;
        const size_t nPatch = static_cast<int>(n) / (nStep << 1);
        for (size_t offset = 0; offset < n; offset += (nStep << 1)) {
            for (int step = 0; step < nStep; step++) {
                Complex *X1 = X + offset + step;
                Complex *X2 = X + offset + step + nStep;
                Complex u = *X1;
                Complex v = *X2 * W[step * nPatch];
                *X1 = u + v;
                *X2 = u - v;
            }
        }
    }

    for (auto &k: X) {
        k.re /= n;
        k.im /= n;
    }
}

int main() {
    /**
     * Answer
     * 381.000000,0.000000
     * -10.780537,147.991758
     * -24.171573,64.769553
     * -20.766960,42.069746
     * -23.000000,28.000000
     * -26.889895,22.873786
     * -29.828427,8.769553
     * -25.562609,8.795798
     * -27.000000,0.000000
     * -25.562609,-8.795798
     * -29.828427,-8.769553
     * -26.889895,-22.873786
     * -23.000000,-28.000000
     * -20.766960,-42.069746
     * -24.171573,-64.769553
     * -10.780537,-147.991758
     */
    // double x[16] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53};
    // double x[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    double x[16] = {
        0, 0.707107, 1, 0.707107, 0, -0.707107, -1, -0.707107,
        0, 0.707107, 1, 0.707107, 0, -0.707107, -1, -0.707107
    };
    FftComplex X;
    for (int i = 0; i < 16; ++i) {
        X[i] = Complex{x[i], 0.0};
    }
    fft(X);
    show(X, 16);
    ift(X);
    show(X, 16);
}
