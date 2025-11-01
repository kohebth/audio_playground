#ifndef WAH_C_H
#define WAH_C_H

#include <env.h>
#include <iir.h>
#include <unit.h>


UNIT(
    Wah
    ,
    double gain;
    double threshold;
    double low;
    double high;
    Envelope *enveloper;
    IIR *sweeper;
    ,
    double gain;
    uint32_t threshold_dB;
    EnvelopeTune *enveloper_tune;
    double low_Hz;
    double high_Hz;
)

#endif //WAH_C_H
