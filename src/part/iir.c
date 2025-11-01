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

static void iir_tune_LPF(IIR *p_unit,
                         const uint32_t fs,
                         const double fc,
                         const double Q
) {
    const double w0 = fast_f2w(fc / fs);
    const double sw0 = sin(w0);
    const double cw0 = cos(w0);
    const double alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (Low-Pass)
    const double m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_unit->a1 = -m_1_a0 * cw0 * 2.0;
    p_unit->a2 = +m_1_a0 * (1.0 - alpha);
    p_unit->b0 = +m_1_a0 * (1.0 - cw0) * 0.5;
    p_unit->b1 = +m_1_a0 * (1.0 - cw0);
    p_unit->b2 = +m_1_a0 * (1.0 - cw0) * 0.5;
}

static void iir_tune_HPF(IIR *p_unit,
                         const uint32_t fs,
                         const double fc,
                         const double Q
) {
    const double w0 = fast_f2w(fc / fs);
    const double sw0 = sin(w0);
    const double cw0 = cos(w0);
    const double alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (High-Pass)
    const double m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_unit->a1 = -m_1_a0 * cw0 * 2.0;
    p_unit->a2 = +m_1_a0 * (1.0 - alpha);
    p_unit->b0 = +m_1_a0 * (1.0 + cw0) * 0.5;
    p_unit->b1 = -m_1_a0 * (1.0 + cw0);
    p_unit->b2 = +m_1_a0 * (1.0 + cw0) * 0.5;
}

static void iir_tune_LSF(IIR *p_unit,
                         const uint32_t fs,
                         const double fc_low, const double low_dB,
                         const double fc_high, const double high_dB,
                         const double slope
) {
    const double A = fast_db2ratio(0.5 * (high_dB + low_dB));
    const double w0 = fast_f2w(sqrt(fc_high * fc_low) / fs);

    const double sqrt_A = sqrt(A);
    const double sw0 = sin(w0), cw0 = cos(w0);
    const double _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const double alpha = sw0 / 2.0 * _1_Q;

    // Biquad coefficients (Low-Shelf)
    const double m_1_a0 = 1.0 / ((A + 1.0) + (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
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

static void iir_tune_HSF(IIR *p_unit,
                         const uint32_t fs,
                         const double fc_low, const double low_dB,
                         const double fc_high, const double high_dB,
                         const double slope
) {
    const double A = fast_db2ratio(0.5 * (high_dB + low_dB));
    const double w0 = fast_f2w(sqrt(fc_high * fc_low) / fs);

    const double sqrt_A = sqrt(A);
    const double sw0 = sin(w0), cw0 = cos(w0);
    const double _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const double alpha = sw0 / 2.0 * _1_Q;

    // Biquad coefficients (High-Shelf)
    const double m_1_a0 = 1.0 / ((A + 1.0) - (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
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

static void iir_tune_EQ(IIR *p_unit,
                        const uint32_t fs,
                        const double fc,
                        const double Q,
                        const double G_dB
) {
    const double A = fast_db2ratio(G_dB);
    const double w0 = fast_f2w(fc / fs);
    const double sinw0 = sin(w0), cosw0 = cos(w0);
    const double alpha_x_A = A * sinw0 / (2.0 * Q);
    const double alpha_A = sinw0 / (2.0 * Q * A);

    // Biquad coefficients (Band-Equalize)
    const double m_1_a0 = 1.0 / (1.0 + alpha_A);
    p_unit->a1 = -m_1_a0 * (2.0 * cosw0);
    p_unit->a2 = +m_1_a0 * (1.0 - alpha_A);
    p_unit->b0 = +m_1_a0 * (1.0 + alpha_x_A);
    p_unit->b1 = -m_1_a0 * (2.0 * cosw0);
    p_unit->b2 = +m_1_a0 * (1.0 - alpha_x_A);
}

double apply_IIR(IIR *p_unit, const double x) {
    const double y = p_unit->b0 * x + p_unit->b1 * p_unit->x1
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
            const struct IIR_Filter *ft = &p_tune->value.filter;
            return iir_tune_LPF(p_unit, p_tune->fs, ft->fc, ft->Q);
        }
        case HPF: {
            const struct IIR_Filter *ft = &p_tune->value.filter;
            return iir_tune_HPF(p_unit, p_tune->fs, ft->fc, ft->Q);
        }
        case LSF: {
            const struct IIR_SHELF *sh = &p_tune->value.shelf;
            return iir_tune_LSF(p_unit, p_tune->fs, sh->fc_low, sh->low_dB, sh->fc_high, sh->high_dB, sh->slope);
        }
        case HSF: {
            const struct IIR_SHELF *sh = &p_tune->value.shelf;
            return iir_tune_HSF(p_unit, p_tune->fs, sh->fc_low, sh->low_dB, sh->fc_high, sh->high_dB, sh->slope);
        }
        case EQ: {
            const struct IIR_EQ *eq = &p_tune->value.eq;
            return iir_tune_EQ(p_unit, p_tune->fs, eq->fc, eq->Q, eq->G_dB);
        }
    }
}

void deinit_IIR(IIR *p_unit) {
    free(p_unit);
}
