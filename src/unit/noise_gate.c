#include "noise_gate.h"

#include <stdlib.h>
#include <effect.h>
#include <reverse_delay.h>

NoiseGate *NoiseGate_init() {
    NoiseGate * unit = malloc(sizeof(NoiseGate));
    unit->env = (Envelope *)malloc(sizeof(Envelope));
    return unit;
}

double NoiseGate_process(NoiseGate *p_NoiseGate) {

}

void NoiseGate_tune(NoiseGate *p_NoiseGate, uint32_t threshold_dB, uint32_t attack_ms, uint32_t release_ms, double ratio) {

}

void NoiseGate_deinit(NoiseGate *p_NoiseGate) {

}



