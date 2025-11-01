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

ReverseDelay *reverse_delay_init(
    const uint32_t fs,
    const uint32_t threshold_dB,
    const uint32_t time_ms,
    const double fdback_dB,
    const double mix
) {
    const uint32_t delayed_sample = fs * time_ms / 1000;
    ReverseDelay * delay = malloc(sizeof(ReverseDelay));
    delay->threshold = fast_exp(M_LN10 * 0.05 * threshold_dB);
    delay->fdback = fast_exp(M_LN10 * 0.05 * fdback_dB);
    delay->mix = mix;
    delay->delayed_sample = delayed_sample;
    delay->ring = ring_init(delay->delayed_sample);
    delay->tape = (double *)malloc(sizeof(double) * delayed_sample);
    delay->taped_sample = 0;
    delay->taped_record = false;
    delay->envelope = env_init(fs, 0.025, 0.001);
    return delay;
}

void reverse_delay_deinit(ReverseDelay *delay) {
    envelope_deinit(delay->envelope);
    ring_deinit(delay->ring);
    free(delay->tape);
    free(delay);
}

double reverse_delay_process(ReverseDelay *delay, const double dry) {
    if (!delay->taped_record) {
        if (env_detect(delay->envelope, dry) > delay->threshold) {
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
