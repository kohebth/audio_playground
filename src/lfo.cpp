#include "lfo.h"

#include <cstdlib>
#include <cmath>
#include <cstdint>

//LowFrequencyOscillator
struct LFO {
    double *start_lut;
    double *end_lut;
    double *val;
};

LFO *lfo_init_sine(const double fs, const double fm, double ampl) {
    const auto lfo = static_cast<LFO *>(malloc(sizeof(LFO)));

    const auto n = static_cast<size_t>(fs / fm);
    const auto m_1_n = 1.0 / static_cast<double>(n);
    double nw0 = 2.0 * M_PI * m_1_n;

    lfo->start_lut = static_cast<double *>(malloc(n * sizeof(double)));
    lfo->end_lut = lfo->start_lut + n;
    lfo->val = lfo->start_lut;

    int i = 0;
    for (double *lut = lfo->start_lut; lut < lfo->end_lut; ++lut) {
        *lut = ampl * fs * sin(nw0 * i++);
    }

    return lfo;
}

LFO *lfo_init_triangle(const double fs, const double fm, double ampl) {
    const auto lfo = static_cast<LFO *>(malloc(sizeof(LFO)));

    const auto n = static_cast<size_t>(fs / fm);
    const auto m_1_n = 1.0 / static_cast<double>(n);
    // double nw0 = 2.0 * M_PI * m_1_n;

    lfo->start_lut = static_cast<double *>(malloc(n * sizeof(double)));
    lfo->end_lut = lfo->start_lut + n;
    lfo->val = lfo->start_lut;

    int i = 0;
    for (double *lut = lfo->start_lut; lut < lfo->end_lut; ++lut) {
        // *lut = ampl * M_2_PI * asin(sin(nw0 * i++));
        double phase = 4.0 * m_1_n * i++;
        double f_phase = phase - static_cast<uint32_t>(phase);

        switch (static_cast<uint32_t>(phase) % 4) {
            case 0:
                *lut = +f_phase;
                break;
            case 1:
                *lut = 1.0 - f_phase;
                break;
            case 2:
                *lut = -f_phase;
                break;
            case 3:
                *lut = f_phase - 1.0;
                break;
            default:
                exit(-1);
        }
        *lut *= ampl * fs;
    }

    return lfo;
}

double lfo_step(LFO *lfo) {
    ++lfo->val;
    if (lfo->val >= lfo->end_lut) {
        lfo->val = lfo->start_lut;
    }
    return *lfo->val;
}

void lfo_deinit(LFO *lfo) {
    free(lfo->start_lut);
    free(lfo);
}
