#include <reverse_delay.h>

#include <env.h>
#include <fast_math.h>
#include <ring.h>

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

struct ReverseDelay {
    double threshold;
    double fdback;
    double mix;
    uint32_t delayed_sample;
    uint32_t taped_sample;
    bool taped_record;
    double *tape;
    Ring *ring;
    Envelope *envelope;
};

RevDelay *init_RevDelay() {
    RevDelay *p_RevDelay = malloc(sizeof(RevDelay));
    return p_RevDelay;
}

RevDelay *tune_RevDelay(
    RevDelay *p_RevDelay,
    const uint32_t fs,
    const uint32_t threshold_dB,
    const uint32_t time_ms,
    const double fdback_dB,
    const double mix
) {
    const uint32_t delayed_sample = fs * time_ms / 1000;

    p_RevDelay->threshold = fast_exp(M_LN10 * 0.05 * threshold_dB);
    p_RevDelay->fdback = fast_exp(M_LN10 * 0.05 * fdback_dB);
    p_RevDelay->mix = mix;
    p_RevDelay->delayed_sample = delayed_sample;
    p_RevDelay->ring = ring_init(p_RevDelay->delayed_sample);
    p_RevDelay->tape = (double *) malloc(sizeof(double) * delayed_sample);
    p_RevDelay->taped_sample = 0;
    p_RevDelay->taped_record = false;
    p_RevDelay->envelope = init_Envelope();
    tune_Envelope(
        p_RevDelay->envelope,
        &(EnvelopeTune){.fs = fs, .attack_ms = 0.025, .release_ms = 0.010});
    return p_RevDelay;
}

void deinit_RevDelay(RevDelay *delay) {
    deinit_Envelope(delay->envelope);
    ring_deinit(delay->ring);
    free(delay->tape);
    free(delay);
}

double apply_RevDelay(RevDelay *delay, const double dry) {
    if (!delay->taped_record) {
        if (apply_Envelope(delay->envelope, dry) > delay->threshold) {
            delay->taped_sample = 0;
            delay->taped_record = true;
        }
    } else if (delay->taped_sample < delay->delayed_sample) {
        delay->tape[delay->taped_sample++] = dry;
    } else {
        for (uint32_t i = 1; i <= delay->delayed_sample; ++i) {
            const double exist = ring_get(delay->ring, 1);
            ring_add(delay->ring, 0.5 * (exist + delay->tape[delay->delayed_sample - i]));
        }
        delay->taped_record = false;
    }

    const double wet = ring_get(delay->ring, 1);
    ring_add(delay->ring, wet * delay->fdback);
    return delay->mix * dry + (1.0 - delay->mix) * wet;
}
