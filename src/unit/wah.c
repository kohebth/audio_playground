#include <fast_math.h>
#include <math.h>
#include <stdlib.h>
#include <wah.h>

extern uint32_t fs;

Wah *init_Wah(void) {
    Wah *p_Wah = malloc(sizeof(Wah));
    p_Wah->enveloper = init_Envelope();
    p_Wah->sweeper = init_IIR();
    p_Wah->low = 200;
    p_Wah->low = 2000;
    p_Wah->threshold = -10;
    p_Wah->gain = 1.0;
    return p_Wah;
}

double apply_Wah(Wah *p_unit, double x) {

}

void tune_Wah(
    Wah *p_unit,
    const WahTune *p_tune
) {
    tune_Envelope(p_unit->enveloper, p_tune->enveloper_tune);
    p_unit->gain = p_tune->gain;
    p_unit->threshold = fast_db2ratio(p_tune->threshold_dB);
    p_unit->low = p_tune->low_Hz;
    p_unit->high = p_tune->high_Hz;
}

void Wah_deinit(Wah *p_Wah) {
    deinit_Envelope(p_Wah->enveloper);
    deinit_IIR(p_Wah->sweeper);
    free(p_Wah);
}
