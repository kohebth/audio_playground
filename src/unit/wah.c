#include <fast_math.h>
#include <math.h>
#include <stdlib.h>
#include <wah.h>

extern uint32_t fs;

Wah *init_Wah(void) {
    Wah *p_unit = malloc(sizeof(Wah));
    p_unit->enveloper = init_Envelope();
    p_unit->sweeper = init_IIR();
    p_unit->low_ln = fast_log2(200);
    p_unit->high_ln = fast_log2(2000);
    p_unit->threshold = -10;
    p_unit->gain = 1.0;
    return p_unit;
}

float apply_Wah(Wah *p_unit, const float x) {

}

void tune_Wah(
    Wah *p_unit,
    const WahTune *p_tune
) {
    p_unit->threshold = fast_db2ratio(p_tune->threshold_dB);
    p_unit->gain = p_tune->gain;
    p_unit->low_ln = log2(p_tune->low_Hz);
    p_unit->high_ln = log2(p_tune->high_Hz);
    tune_Envelope(p_unit->enveloper, p_tune->enveloper_tune);
    tune_IIR(p_unit->sweeper, p_tune->sweeper_tune);
}

void Wah_deinit(Wah *p_unit) {
    deinit_Envelope(p_unit->enveloper);
    deinit_IIR(p_unit->sweeper);
    free(p_unit);
}
