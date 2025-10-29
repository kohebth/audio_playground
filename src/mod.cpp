#include <mod.h>
#include <buffer.h>
#include <lfo.h>
#include <cmath>
#include <iir.h>

#include <iostream>

struct Setting {
    double sample_delay; // 0.00512 - 0.05120
    double fdback; // 0.0 - 1.0
    double rate; // 0.25 - 3.5 Hz
    double depth; // 0.00512 < depth < delay
    uint32_t sample_rate;
};

struct Modulation {
    Setting setting;
    RingBuffer *delay_line;
    LFO *lfo;
};

Modulation *modulation_init(
    const uint32_t sample_rate,
    const double delay,
    const double fdback,
    const double rate,
    const double depth
) {
    auto *m = static_cast<Modulation *>(malloc(sizeof(Modulation)));
    if (!m) return nullptr;

    const auto delay_line_size = 4 * static_cast<size_t>(sample_rate * delay);

    m->setting.sample_rate = sample_rate;
    m->setting.sample_delay = delay * sample_rate;
    m->setting.fdback = fdback;
    m->setting.rate = rate;
    m->setting.depth = depth;
    m->delay_line = buffer_init(delay_line_size);
    m->lfo = lfo_init_triangle(sample_rate, rate, depth);
    return m;
}

double modulation_process(Modulation *p_modulation, double x) {
    RingBuffer *delay_line = p_modulation->delay_line;
    LFO *lfo = p_modulation->lfo;

    double delay = p_modulation->setting.sample_delay + lfo_step(lfo);
    auto delay_samples = static_cast<int32_t>(delay);
    auto frac = static_cast<float>(delay - delay_samples);
    double d0 = buffer_get(delay_line, -delay_samples);
    double d1 = buffer_get(delay_line, -delay_samples + 1);

    double wet = d1 * (1.0 - frac) + d0 * frac;
    double fdback = p_modulation->setting.fdback;
    buffer_add(delay_line, fdback * wet + (1.0 - fdback) * x);
    return wet;
}

void modulation_deinit(Modulation *p_modulation) {
    buffer_deinit(p_modulation->delay_line);
    free(p_modulation);
}
