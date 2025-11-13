#include "noise_gate.h"

#include <stdlib.h>
#include <reverse_delay.h>

extern uint32_t fs;

NoiseGate *init_NoiseGate() {
    NoiseGate *unit = malloc(sizeof(NoiseGate));
    unit->env = init_Envelope();
    return unit;
}

float apply_NoiseGate(NoiseGate *p_unit, const float x) {

}

void tune_NoiseGate(NoiseGate *p_unit, const NoiseGateTune *p_tune) {

}

void deinit_NoiseGate(NoiseGate *p_unit) {
    free(p_unit);
}
