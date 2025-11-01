#ifndef MODULATION_H
#define MODULATION_H

#include <stdint.h>

typedef struct Modulation Modulation;

Modulation *mod_init(
    uint32_t fs,
    double delay,
    double fdback,
    double rate,
    double depth
);

void mod_deinit(Modulation *p_mod);

double mod_process(const Modulation *p_mod, double x);

#endif //MODULATION_H
