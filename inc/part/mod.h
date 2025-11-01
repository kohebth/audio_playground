#ifndef MODULATION_H
#define MODULATION_H

#include <stdint.h>
#include <unit.h>


typedef struct Modulation Modulation;

Modulation *init_Modulation(
    uint32_t fs,
    double delay,
    double fdback,
    double rate,
    double depth
);

void deinit_Modulation(Modulation *p_mod);

double apply_Modulation(const Modulation *p_mod, double x);

#endif //MODULATION_H
