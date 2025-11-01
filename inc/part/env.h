#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <unit.h>

UNIT(
    Envelope,
    double envelope;
    double attack_coef;
    double sup_attack_coef;
    double release_coef;
    double sup_release_coef;,
    double attack_ms; double release_ms;
)

#endif //ENVELOPE_H
