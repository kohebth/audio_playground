#ifndef IIR_H
#define IIR_H

#include <stdint.h>
#include <unit.h>

typedef enum {
    LPF = 0, HPF, LSF, HSF, EQ,
} IIRType;

typedef struct IIR_Filter IIRTune_Filter;

typedef struct IIR_Shelf IIRTune_Shelf;

typedef struct IIR_Eq IIRTune_Eq;

struct IIR_Filter {
    float fc;
    float Q;
};

struct IIR_Shelf {
    float fc_low;
    float fc_high;
    float low_dB;
    float high_dB;
    float slope;
};

struct IIR_Eq {
    float fc;
    float Q;
    float G_dB;
};

UNIT(
    IIR
    ,
    float a1; float a2;
    float b0; float b1; float b2;
    float x1; float x2;
    float y1; float y2;
    ,
    IIRType type;
    union {
        IIRTune_Filter filter;
        IIRTune_Shelf shelf;
        IIRTune_Eq eq;
    } value;
)

#endif //IIR_H
