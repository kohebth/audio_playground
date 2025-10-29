#ifndef MODULATION_H
#define MODULATION_H
#include <buffer.h>
#include <cstdint>

struct Setting;

struct Modulation;

Modulation *modulation_init(
    uint32_t sample_rate,
    double delay,
    double fdback,
    double rate,
    double depth
);

void modulation_deinit(Modulation *p_modulation);

double modulation_process(Modulation *p_modulation, double x);

#endif //MODULATION_H
