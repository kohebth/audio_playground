#include <fast_math.h>
#include <mod.h>

#include <lfo.h>
#include <ring.h>
#include <stdlib.h>

typedef struct {
    double sample_delay; // 0.00512 - 0.05120
    double fdback; // 0.0 - 1.0
    double rate; // 0.25 - 3.5 Hz
    double depth; // 0.00512 < depth < delay
    uint32_t sample_rate;
} Setting;

struct Modulation {
    Setting setting;
    Ring *delay_line;
    LFO *lfo;
};

Modulation *init_Modulation(
    const uint32_t fs,
    const double delay,
    const double fdback,
    const double rate,
    const double depth
) {
    Modulation *m = malloc(sizeof(Modulation));
    const uint32_t delay_line_size = 4 * (size_t) (fs * delay);

    m->setting.sample_rate = fs;
    m->setting.sample_delay = delay * fs;
    m->setting.fdback = fdback;
    m->setting.rate = rate;
    m->setting.depth = depth;
    m->delay_line = ring_init(delay_line_size);
    m->lfo = lfo_init_triangle(fs, rate, depth);
    return m;
}

double apply_Modulation(const Modulation *p_mod, const double x) {
    Ring *delay_line = p_mod->delay_line;
    LFO *lfo = p_mod->lfo;

    double delay = p_mod->setting.sample_delay + lfo_step(lfo);
    int32_t delay_samples = delay;
    double d0 = ring_get(delay_line, -delay_samples);
    double d1 = ring_get(delay_line, -delay_samples + 1);

    double wet = fast_ipol_linear(d1, d0, delay - delay_samples);
    double fdback = p_mod->setting.fdback;
    ring_add(delay_line, fdback * wet + (1.0 - fdback) * x);
    return wet;
}

void deinit_Modulation(Modulation *p_mod) {
    ring_deinit(p_mod->delay_line);
    free(p_mod);
}
