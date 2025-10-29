#include <effect.h>
#include <buffer.h>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fast_math.h>
#include <iir.h>
#include <mod.h>

struct Unit {
    union {
        Envelope envelope;
        Gain gain;
        Modulate modulate;
        Delay delay;
        Reverb reverb;
    } effect;

    void (*apply)(audio_buffer *chunk, Unit *effect);
};

std::vector<Unit> effects;

static void apply_saturation(audio_buffer *buffer, const double gain, const float volume) {
    float* x = buffer->data;
    float* end_x = x + buffer->length;
    while (x < end_x) {
        *x = static_cast<float>((2.0 * fast_sigmoid(*x * gain)) * volume);
        ++x;
    }
}

static void apply_saturation2(audio_buffer *buffer, const double gain, const double volume) {
    float *x = buffer->data;
    float *end_x = x + buffer->length;
    while (x < end_x) {
        double xx = *x * gain;
        *x = static_cast<float>(xx / (0.5 + fabs(xx)) * volume);
        ++x;
    }
}

static void apply_saturation3(audio_buffer *chunk, const double gain, const double volume) {
    auto *x = chunk->data;
    for (size_t i = 0; i < chunk->length; ++i) {
        double xx = gain * x[i];
        x[i] = 2.0 / M_PI * fast_atan(xx) * volume;
    }
}

static void apply_noise_reduction(audio_buffer *chunk, double threshold, double attack_ms, double release_ms,
                                  double ratio) {
    static double envelope = 0.0;
    static double attack = exp(-1.0 / (attack_ms * 48000 * 0.001));
    static double release = exp(-1.0 / (release_ms * 48000 * 0.001));
    static double gain = 0.0;

    auto *x = reinterpret_cast<int16_t *>(chunk->data);
    for (size_t i = 0; i < chunk->length; ++i) {
        auto xx = static_cast<double>(x[i]);
        double axx = fabs(xx);

        // Envelope smoothing (EMA)
        if (axx > envelope)
            envelope = attack * envelope + (1.0 - attack) * axx;
        else
            envelope = release * envelope + (1.0 - release) * axx;

        // Compute target gain
        double target = (envelope > threshold) ? 1.0 : (envelope / threshold) * ratio;

        // Smooth gain
        if (target > gain)
            gain = attack * gain + (1.0 - attack) * target;
        else
            gain = release * gain + (1.0 - release) * target;

        // Apply minimum gain floor
        gain = std::max(gain, 0.001);
        x[i] = static_cast<int16_t>(gain * x[i]);
    }
}

static uint32_t fs = 48000;

static Modulation *m = modulation_init(fs, 23.04 / 1000, 0.2, 0.4, 12.0 / 1000);
// static Modulation *m2 = modulation_init(fs, 23.04 / 1000, 0.2, 2.4, 3.5 / 1000);

static IIRFilter *iir_emphaser = iir_init_HSF(fs, 410, 3, 2341, 12, 0.468);
static IIRFilter *iir_de_emphaser = iir_init_HSF(fs, 410, -3, 2341, -12, 0.468);
static IIRFilter *iir_anti_alias_low = iir_init_HPF(fs, 48.2, M_SQRT1_2); // Butterworth 1/sqrt(2)
static IIRFilter *iir_anti_alias_high = iir_init_LPF(fs, 6600, M_SQRT1_2);
static IIRFilter *iir_reconstruct_low = iir_init_HPF(fs, 18.6, M_SQRT1_2); // Butterworth 1/sqrt(2)
static IIRFilter *iir_reconstruct_high = iir_init_LPF(fs, 6600, M_SQRT1_2);
static void apply_modulation(audio_buffer *buffer, Modulation *m) {
    float* x = buffer->data;
    float* end_x = x + buffer->length;
    while (x < end_x) {
        double dry = *x;
        double emphasized = iir_process(iir_emphaser, dry);
        double anti_alias = emphasized;
        anti_alias = iir_process(iir_anti_alias_low, anti_alias);
        anti_alias = iir_process(iir_anti_alias_high, anti_alias);

        double modulated = modulation_process(m, anti_alias);
        // modulated = modulation_process(m2, modulated);
        double reconstructed = modulated;
        reconstructed = iir_process(iir_reconstruct_low, reconstructed);
        reconstructed = iir_process(iir_reconstruct_high, reconstructed);

        double deemphasized = iir_process(iir_de_emphaser, reconstructed);
        *x = deemphasized *0.5 + 0.5*emphasized;
        ++x;
    }
}

static RingBuffer *ring_buffer = buffer_init(48000);
static void apply_delay(audio_buffer *buffer) {
    float* x = buffer->data;
    float* end_x = x + buffer->length;
    while (x < end_x) {
        double offset = -static_cast<int32_t>(0.380 * fs);
        double wet = *x * 0.8 + buffer_get(ring_buffer, offset) * 0.2;
        buffer_add(ring_buffer, wet);
        *x = wet;
        ++x;
    }
}

static RingBuffer *reverse_buffer = buffer_init();
static void apply_delay_reverse(audio_buffer *buffer) {
    float* x = buffer->data;
    float* end_x = x + buffer->length;
    while (x < end_x) {
        double offset = -static_cast<int32_t>(0.380 * fs);
        double wet = *x * 0.8 + buffer_get(ring_buffer, offset) * 0.2;
        buffer_add(ring_buffer, wet);
        *x = wet;
        ++x;
    }
}

void chain_procces(audio_buffer *chunk) {
    int8_t gainDb = 30;
    double gain = fast_exp(M_LN10 * gainDb / 20);
    int8_t noiseThresholdDb = -25;
    double noiseThreshold = fast_exp(M_LN10 * noiseThresholdDb / 20);
    // apply_noise_reduction(chunk, noiseThreshold, 0.1, 320, 0.02);
    apply_modulation(chunk, m);
    apply_delay(chunk);
    // audio_buffer chunk_1;
    // memcpy(&chunk_1, chunk, sizeof(chunk));
}

void chain_deinit() {
    modulation_deinit(m);
}

void chain_init() {
    // modulation_deinit(m);
}
