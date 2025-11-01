#ifndef NOISE_GATE_H
#define NOISE_GATE_H
#include <stdint.h>

#include <env.h>

#define UNIT(name, members, ...) \
    typedef struct name { \
    members \
    } name; \
\
name *name##_init(void); \
double name##_process(name *p_##name); \
void name##_tune(name *p_##name, __VA_ARGS__); \
void name##_deinit(name *p_##name);

UNIT(
    NoiseGate
    ,
    double threshold;
    Envelope *env;
    double ratio;
    ,
    uint32_t threshold_dB,
    uint32_t attack_ms,
    uint32_t release_ms,
    double ratio
)

#endif //NOISE_GATE_H
