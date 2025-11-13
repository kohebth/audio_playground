#include <iir.h>

#include <fast_math.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

IIR *init_IIR(void) {
    IIR *p_unit = malloc(sizeof(IIR));
    p_unit->a1 = 0.0;
    p_unit->a2 = 0.0;
    p_unit->b0 = 1.0;
    p_unit->b1 = 0.0;
    p_unit->b2 = 0.0;
    p_unit->x1 = 0.0;
    p_unit->x2 = 0.0;
    p_unit->y1 = 0.0;
    p_unit->y2 = 0.0;
    return p_unit;
}

static void __iir_tune_LPF(IIR *p_unit,
                         const uint32_t fs,
                         const float fc,
                         const float Q
) {
    const float w0 = fast_freq2angle(fc / fs);
    const float sw0 = sin(w0);
    const float cw0 = cos(w0);
    const float alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (Low-Pass)
    const float m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_unit->a1 = -m_1_a0 * cw0 * 2.0;
    p_unit->a2 = +m_1_a0 * (1.0 - alpha);
    p_unit->b0 = +m_1_a0 * (1.0 - cw0) * 0.5;
    p_unit->b1 = +m_1_a0 * (1.0 - cw0);
    p_unit->b2 = +m_1_a0 * (1.0 - cw0) * 0.5;
}

static void __iir_tune_HPF(IIR *p_unit,
                         const uint32_t fs,
                         const float fc,
                         const float Q
) {
    const float w0 = fast_freq2angle(fc / fs);
    const float sw0 = sin(w0);
    const float cw0 = cos(w0);
    const float alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (High-Pass)
    const float m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_unit->a1 = -m_1_a0 * cw0 * 2.0;
    p_unit->a2 = +m_1_a0 * (1.0 - alpha);
    p_unit->b0 = +m_1_a0 * (1.0 + cw0) * 0.5;
    p_unit->b1 = -m_1_a0 * (1.0 + cw0);
    p_unit->b2 = +m_1_a0 * (1.0 + cw0) * 0.5;
}

static void __iir_tune_LSF(IIR *p_unit,
                         const uint32_t fs,
                         const float fc_low, const float low_dB,
                         const float fc_high, const float high_dB,
                         const float slope
) {
    const float A = fast_db2ratio(0.5 * (high_dB + low_dB));
    const float w0 = fast_freq2angle(sqrt(fc_high * fc_low) / fs);

    const float sqrt_A = sqrt(A);
    const float sw0 = sin(w0), cw0 = cos(w0);
    const float _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const float alpha = sw0 / 2.0 * _1_Q;

    // Biquad coefficients (Low-Shelf)
    const float m_1_a0 = 1.0 / ((A + 1.0) + (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
    p_unit->a1 = ((A - 1.0) + (A + 1.0) * cw0) * -2.0;
    p_unit->a2 = (A + 1.0) + (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;
    p_unit->b0 = (A + 1.0) - (A - 1.0) * cw0 + 2 * sqrt_A * alpha;
    p_unit->b1 = (A - 1.0) - (A + 1.0) * cw0;
    p_unit->b2 = (A + 1.0) - (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;

    p_unit->a1 *= -m_1_a0 * 2.0;
    p_unit->a2 *= +m_1_a0;
    p_unit->b0 *= +m_1_a0 * A;
    p_unit->b1 *= +m_1_a0 * A * 2.0;
    p_unit->b2 *= +m_1_a0 * A;
}

static void __iir_tune_HSF(IIR *p_unit,
                         const uint32_t fs,
                         const float fc_low, const float low_dB,
                         const float fc_high, const float high_dB,
                         const float slope
) {
    const float A = fast_db2ratio(0.5 * (high_dB + low_dB));
    const float w0 = fast_freq2angle(sqrt(fc_high * fc_low) / fs);

    const float sqrt_A = sqrt(A);
    const float sw0 = sin(w0), cw0 = cos(w0);
    const float _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const float alpha = sw0 / 2.0 * _1_Q;

    // Biquad coefficients (High-Shelf)
    const float m_1_a0 = 1.0 / ((A + 1.0) - (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
    p_unit->a1 = ((A - 1.0) - (A + 1.0) * cw0);
    p_unit->a2 = (A + 1.0) - (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;
    p_unit->b0 = (A + 1.0) + (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha;
    p_unit->b1 = (A - 1.0) + (A + 1.0) * cw0;
    p_unit->b2 = (A + 1.0) + (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;

    p_unit->a1 *= +m_1_a0 * 2.0;
    p_unit->a2 *= +m_1_a0;
    p_unit->b0 *= +m_1_a0 * A;
    p_unit->b1 *= -m_1_a0 * A * 2.0;
    p_unit->b2 *= +m_1_a0 * A;
}

static void __iir_tune_EQ(IIR *p_unit,
                        const uint32_t fs,
                        const float fc,
                        const float Q,
                        const float G_dB
) {
    const float A = fast_db2ratio(G_dB);
    const float w0 = fast_freq2angle(fc / fs);
    const float sinw0 = sin(w0), cosw0 = cos(w0);
    const float alpha_x_A = A * sinw0 / (2.0 * Q);
    const float alpha_A = sinw0 / (2.0 * Q * A);

    // Biquad coefficients (Band-Equalize)
    const float m_1_a0 = 1.0 / (1.0 + alpha_A);
    p_unit->a1 = -m_1_a0 * (2.0 * cosw0);
    p_unit->a2 = +m_1_a0 * (1.0 - alpha_A);
    p_unit->b0 = +m_1_a0 * (1.0 + alpha_x_A);
    p_unit->b1 = -m_1_a0 * (2.0 * cosw0);
    p_unit->b2 = +m_1_a0 * (1.0 - alpha_x_A);
}

float apply_IIR(IIR *p_unit, const float x) {
    const float y = p_unit->b0 * x + p_unit->b1 * p_unit->x1
                     + p_unit->b2 * p_unit->x2
                     - p_unit->a1 * p_unit->y1
                     - p_unit->a2 * p_unit->y2;
    p_unit->x2 = p_unit->x1;
    p_unit->x1 = x;
    p_unit->y2 = p_unit->y1;
    p_unit->y1 = y;
    return y;
}

void tune_IIR(IIR *p_unit, const IIRTune *p_tune) {
    switch (p_tune->type) {
        case LPF: {
            const IIRTune_Filter *ft = &p_tune->value.filter;
            return __iir_tune_LPF(p_unit, p_tune->fs, ft->fc, ft->Q);
        }
        case HPF: {
            const IIRTune_Filter *ft = &p_tune->value.filter;
            return __iir_tune_HPF(p_unit, p_tune->fs, ft->fc, ft->Q);
        }
        case LSF: {
            const IIRTune_Shelf *sh = &p_tune->value.shelf;
            return __iir_tune_LSF(p_unit, p_tune->fs, sh->fc_low, sh->low_dB, sh->fc_high, sh->high_dB, sh->slope);
        }
        case HSF: {
            const IIRTune_Shelf *sh = &p_tune->value.shelf;
            return __iir_tune_HSF(p_unit, p_tune->fs, sh->fc_low, sh->low_dB, sh->fc_high, sh->high_dB, sh->slope);
        }
        case EQ: {
            const IIRTune_Eq *eq = &p_tune->value.eq;
            return __iir_tune_EQ(p_unit, p_tune->fs, eq->fc, eq->Q, eq->G_dB);
        }
    }
}

void deinit_IIR(IIR *p_unit) {
    free(p_unit);
}
