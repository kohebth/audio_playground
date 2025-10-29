#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iir.h>

#define M2PI (2.0 * M_PI)

struct IIRFilter {
    double a1, a2;
    double b0, b1, b2;
    double x1, x2;
    double y1, y2;
};

IIRFilter *iir_init_LPF(const uint32_t fs, const double fc, const double Q) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double w0 = M2PI * fc / fs;
    const double sw0 = sin(w0);
    const double cw0 = cos(w0);
    const double alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (Low-Pass)
    const double m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_filter->a1 = -m_1_a0 * cw0 * 2.0;
    p_filter->a2 = +m_1_a0 * (1.0 - alpha);
    p_filter->b0 = +m_1_a0 * (1.0 - cw0) * 0.5;
    p_filter->b1 = +m_1_a0 * (1.0 - cw0);
    p_filter->b2 = +m_1_a0 * (1.0 - cw0) * 0.5;

    return p_filter;
}

IIRFilter *iir_init_HPF(const uint32_t fs, const double fc, const double Q) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double w0 = M2PI * fc / fs;
    const double sw0 = sin(w0);
    const double cw0 = cos(w0);
    const double alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (High-Pass)
    const double m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_filter->a1 = -m_1_a0 * cw0 * 2.0;
    p_filter->a2 = +m_1_a0 * (1.0 - alpha);
    p_filter->b0 = +m_1_a0 * (1.0 + cw0) * 0.5;
    p_filter->b1 = -m_1_a0 * (1.0 + cw0);
    p_filter->b2 = +m_1_a0 * (1.0 + cw0) * 0.5;

    return p_filter;
}

IIRFilter *iir_init_BPF(const uint32_t fs, const double fc, const double Q) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double w0 = M2PI * fc / fs;
    const double sw0 = sin(w0);
    const double cw0 = cos(w0);
    const double alpha = sw0 / (2.0 * Q);

    // Biquad coefficients (Band-Pass)
    const double m_1_a0 = 1.0 / (1.0 + alpha); // 1/a0
    p_filter->a1 = -m_1_a0 * cw0 * 2.0;
    p_filter->a2 = +m_1_a0 * (1.0 - alpha);
    p_filter->b0 = +m_1_a0 * sw0 * 0.5;
    p_filter->b1 = 0.0;
    p_filter->b2 = -m_1_a0 * sw0 * 0.5;

    return p_filter;
}

IIRFilter *iir_init_HSF(
    const uint32_t fs,
    const double fc_low,
    const double low_dB,
    const double fc_high,
    const double high_dB,
    const double slope
) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double fc_pole = sqrt(fc_high * fc_low);
    const double g_dB_pole = 0.5 * (high_dB + low_dB);
    const double A = pow(10, 0.05 * g_dB_pole);
    const double w0 = M2PI * fc_pole / fs;

    const double sqrt_A = sqrt(A);
    const double sw0 = sin(w0), cw0 = cos(w0);
    const double _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const double alpha = sw0 / 2.0 * _1_Q;

    const double m_1_a0 = 1.0 / ((A + 1.0) - (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
    p_filter->a1 = ((A - 1.0) - (A + 1.0) * cw0);
    p_filter->a2 = (A + 1.0) - (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;
    p_filter->b0 = (A + 1.0) + (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha;
    p_filter->b1 = (A - 1.0) + (A + 1.0) * cw0;
    p_filter->b2 = (A + 1.0) + (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;

    p_filter->a1 *= +m_1_a0 * 2.0;
    p_filter->a2 *= +m_1_a0;
    p_filter->b0 *= +m_1_a0 * A;
    p_filter->b1 *= -m_1_a0 * A * 2.0;
    p_filter->b2 *= +m_1_a0 * A;

    return p_filter;
}

IIRFilter *iir_init_LSF(
    const uint32_t fs,
    const double fc_low,
    const double low_dB,
    const double fc_high,
    const double high_dB,
    const double slope
) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double fc_pole = sqrt(fc_high * fc_low);
    const double g_dB_pole = 0.5 * (high_dB + low_dB);
    const double A = pow(10, 0.05 * g_dB_pole);
    const double w0 = M2PI * fc_pole / fs;

    const double sqrt_A = sqrt(A);
    const double sw0 = sin(w0), cw0 = cos(w0);
    const double _1_Q = sqrt((A + 1 / A) * (1 / slope - 1) + 2);
    const double alpha = sw0 / 2.0 * _1_Q;

    const double m_1_a0 = 1.0 / ((A + 1.0) + (A - 1.0) * cw0 + 2.0 * sqrt_A * alpha);
    p_filter->a1 = ((A - 1.0) + (A + 1.0) * cw0) * -2.0;
    p_filter->a2 = (A + 1.0) + (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;
    p_filter->b0 = (A + 1.0) - (A - 1.0) * cw0 + 2 * sqrt_A * alpha;
    p_filter->b1 = (A - 1.0) - (A + 1.0) * cw0;
    p_filter->b2 = (A + 1.0) - (A - 1.0) * cw0 - 2.0 * sqrt_A * alpha;

    p_filter->a1 *= -m_1_a0 * 2.0;
    p_filter->a2 *= +m_1_a0;
    p_filter->b0 *= +m_1_a0 * A;
    p_filter->b1 *= +m_1_a0 * A * 2.0;
    p_filter->b2 *= +m_1_a0 * A;

    return p_filter;
}

IIRFilter *iir_init_EQ(const uint32_t fs, const double fc, const double Q, const double G_dB) {
    const auto p_filter = static_cast<IIRFilter *>(malloc(sizeof(IIRFilter)));
    const double A = pow(10, G_dB / 20.0);
    const double w0 = M2PI * fc / fs;
    const double sinw0 = sin(w0), cosw0 = cos(w0);
    const double alpha_x_A = A * sinw0 / (2.0 * Q);
    const double alpha_A = sinw0 / (2.0 * Q * A);

    const double a0 = 1.0 + alpha_A;
    p_filter->a1 = -2.0 * cosw0 / a0;
    p_filter->a2 = 1.0 - alpha_A / a0;
    p_filter->b0 = 1.0 + alpha_x_A / a0;
    p_filter->b1 = -2.0 * cosw0 / a0;
    p_filter->b2 = 1.0 - alpha_x_A / a0;
    return p_filter;
}

double iir_process(IIRFilter *p_filter, const double x) {
    const double y = p_filter->b0 * x + p_filter->b1 * p_filter->x1
                     + p_filter->b2 * p_filter->x2
                     - p_filter->a1 * p_filter->y1
                     - p_filter->a2 * p_filter->y2;
    p_filter->x2 = p_filter->x1;
    p_filter->x1 = x;
    p_filter->y2 = p_filter->y1;
    p_filter->y1 = y;
    return y;
}

void iir_deinit(IIRFilter *p_filter) {
    free(p_filter);
}
