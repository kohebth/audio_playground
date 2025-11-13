#include <effect.h>

#include <fast_chunk.h>
#include <fast_math.h>
#include <iir.h>
#include <mod.h>
#include <ring.h>
#include <reverse_delay.h>

#include <fast_math.h>
#include <math.h>
#include <env.h>

typedef struct Unit Unit;

struct Unit {
    union {
        Dynamic envelope;
        Gain gain;
        Modulate modulate;
        Delay delay;
        Reverb reverb;
    } effect;

    void (*apply)(LiveChunk *chunk, Unit *effect);
};

Unit effects[10];

static double apply_saturation(void *effect, double x) {
    return 2.0 * fast_sigmoid(x * 10) * 0.6;
}

static void apply_saturation2(LiveChunk *buffer, const double gain, const double volume) {
    float *x = buffer->data;
    float *end_x = x + buffer->length;
    while (x < end_x) {
        double xx = *x * gain;
        *x = (float) (xx / (0.5 + fabs(xx)) * volume);
        ++x;
    }
}

static void apply_saturation3(LiveChunk *chunk, const double gain, const double volume) {
    float *x = chunk->data;
    for (size_t i = 0; i < chunk->length; ++i) {
        double xx = gain * x[i];
        x[i] = 2.0 / M_PI * fast_atan(xx) * volume;
    }
}

static const uint32_t fs = 48000;
static Modulation *modulation;
static IIR *iir_emphaser;
static IIR *iir_de_emphaser;
static IIR *iir_anti_alias_low;
static IIR *iir_anti_alias_high;
static IIR *iir_reconstruct_low;
static IIR *iir_reconstruct_high;

static double apply_modulation(void *m, double dry) {
    double emphasized = apply_IIR(iir_emphaser, dry);
    double anti_alias = emphasized;
    anti_alias = apply_IIR(iir_anti_alias_low, anti_alias);
    anti_alias = apply_IIR(iir_anti_alias_high, anti_alias);

    double modulated = apply_Modulation(m, anti_alias);

    double reconstructed = modulated;
    reconstructed = apply_IIR(iir_reconstruct_low, reconstructed);
    reconstructed = apply_IIR(iir_reconstruct_high, reconstructed);

    double deemphasized = apply_IIR(iir_de_emphaser, reconstructed);
    return 0.5f * deemphasized + 0.5 * emphasized;
    // return 0.5 * dry + 0.5 * apply_Modulation(m, dry);
}

static RevDelay *reverse_delay;

static double apply_delay_reverse(void *unit, double dry) {
    RevDelay *d = (unit);
    return apply_RevDelay(d, dry);
}

static void chain_apply(LiveChunk *chunk) {
    // fast_chunk_apply(chunk, NULL, apply_saturation);
    // apply_noise_reduction(chunk, noiseThreshold, 0.1, 320, 0.02);
    fast_chunk_apply(chunk, modulation, apply_modulation);

    // fast_chunk_apply(chunk, reverse_delay, apply_delay_reverse);
}

void chain_procces(LiveChunk *chunk) {
#ifdef LATENCY
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif // LATENCY
    chain_apply(chunk);
#ifdef LATENCY
    clock_gettime(CLOCK_MONOTONIC, &end);
    long sec = end.tv_sec - start.tv_sec;
    long nsec = std::max(nsec, end.tv_nsec - start.tv_nsec);
    if (nsec < 0) {
        sec--;
        nsec += 1000000000;
    }
    static long sec_max = (sec, sec_max)/2;
    static long nsec_max = (nsec + nsec_max)/2;
    printf("\rElapsed: %ld.%09ld seconds", sec_max, nsec_max);
#endif // LATENCY
}

void chain_tune() {
    tune_IIR(iir_emphaser,
             &(IIRTune){
                 .fs = fs,
                 .type = HSF,
                 .value = {.shelf = (struct IIR_Shelf){410.0, 2341.0, 3.0, 12.0, 0.468}}
             });
    tune_IIR(iir_de_emphaser,
             &(IIRTune){
                 .fs = fs, .type = HSF, .value = {.shelf = {410.0, 2341.0, -3.0, -12.0, 0.468}}
             });
    tune_IIR(iir_anti_alias_low,
             &(IIRTune){.fs = fs, .type = HPF, .value = {.filter = {48.2, M_SQRT1_2}}});

    tune_IIR(iir_anti_alias_high,
             &(IIRTune){.fs = fs, .type = LPF, .value = {.filter = {6600, M_SQRT1_2}}});

    tune_IIR(iir_reconstruct_low,
             &(IIRTune){.fs = fs, .type = HPF, .value = {.filter = {18.6, M_SQRT1_2}}});

    tune_IIR(iir_reconstruct_high,
             &(IIRTune){.fs = fs, .type = LPF, .value = {.filter = {6600, M_SQRT1_2}}});
}

void chain_init() {
    modulation = init_Modulation(fs, 23.04 / 1000, 0.2, 3.4, 12.0 / 1000);
    iir_emphaser = init_IIR();
    iir_de_emphaser = init_IIR();
    iir_anti_alias_low = init_IIR();
    iir_anti_alias_high = init_IIR();
    iir_reconstruct_low = init_IIR();
    iir_reconstruct_high = init_IIR();
    // tune_RevDelay(reverse_delay, fs, -20, 412, -3.2, 0.4);
    chain_tune();
}

void chain_deinit() {
    deinit_Modulation(modulation);
    deinit_IIR(iir_emphaser);
    deinit_IIR(iir_de_emphaser);
    deinit_IIR(iir_anti_alias_low);
    deinit_IIR(iir_anti_alias_high);
    deinit_IIR(iir_reconstruct_low);
    deinit_IIR(iir_reconstruct_high);
    // deinit_RevDelay(reverse_delay);
}
