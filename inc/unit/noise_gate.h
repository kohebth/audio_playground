#ifndef NOISE_GATE_H
#define NOISE_GATE_H
#include <stdint.h>

#include <env.h>
#include <unit.h>

UNIT(
    NoiseGate,
    Envelope *env;
    double threshold;
    double ratio;,
    uint32_t threshold_dB;
    uint32_t attack_ms;
    uint32_t release_ms;
)

#endif //NOISE_GATE_H
