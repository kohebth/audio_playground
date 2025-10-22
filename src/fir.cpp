#include <cstdio>
#include <cstdint>
#include <vector>
#include "wave.h"

void designLowpass(std::vector<float>& h, float fc, float fs) {
    const int M = static_cast<int>(h.size());
    const double M_1 = M - 1;
    const float norm_fc = fc / fs;
    for (int n = 0; n < M; n++) {
        int m = n - M / 2;
        if (m == 0)
            h[n] = 2.0f * norm_fc;
        else
            h[n] = sin(M_PI * 2.0f * norm_fc * m) / (M_PI * m);
        h[n] *= 0.5f - 0.5f * cosf(M_PI * 2.0f * n / M_1); // Hann window
    }
}

void applyFIR(const std::vector<float>& x, std::vector<float>& y, const std::vector<float>& h) {
    const int M = static_cast<int>(h.size());
    y.resize(x.size());
    for (size_t n = 0; n < x.size(); n++) {
        double acc = 0;
        for (int k = 0; k < M; k++) {
            if ((int)n - k >= 0)
                acc += x[n - k] * h[k];
        }
        y[n] = (float)acc;
    }
}
