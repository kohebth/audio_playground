#include <env.h>

#include <math.h>
#include <stdlib.h>
#include <fast_math.h>

Envelope *init_Envelope() {
    Envelope *p_unit = malloc(sizeof(Envelope));
    p_unit->envelope = 0.0;
    p_unit->attack_coef = 1.0;
    p_unit->sup_attack_coef = 0.0;
    p_unit->release_coef = 1.0;
    p_unit->sup_release_coef = 0.0;
    return p_unit;
}

float apply_Envelope(Envelope *p_unit, float x) {
    const float abs_x = fabs(x);
    p_unit->envelope = abs_x > p_unit->envelope
                               ? p_unit->attack_coef * p_unit->envelope + p_unit->sup_attack_coef * abs_x
                               : p_unit->release_coef * p_unit->envelope + p_unit->sup_release_coef * abs_x;
    return p_unit->envelope;
}

void tune_Envelope(Envelope *p_unit, const EnvelopeTune *p_tune) {
    p_unit->attack_coef = fast_exp2(-1.0 / (p_tune->fs * p_tune->attack_ms * 0.001));
    p_unit->sup_attack_coef = 1.0 - p_unit->attack_coef;
    p_unit->release_coef = fast_exp2(-1.0 / (p_tune->fs * p_tune->release_ms * 0.001));
    p_unit->sup_release_coef = 1.0 - p_unit->release_coef;
}

void deinit_Envelope(Envelope *p_unit) {
    free(p_unit);
}
