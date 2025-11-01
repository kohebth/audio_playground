#ifndef IIR_H
#define IIR_H

#include <stdint.h>
#include <unit.h>

typedef enum {
    LPF = 0, HPF, LSF, HSF, EQ,
} IIRType;

UNIT(
    IIR
    ,
    double a1; double a2;
    double b0; double b1; double b2;
    double x1; double x2;
    double y1; double y2;
    ,
    IIRType type;
    union {
        struct IIR_Filter {
            double fc; double Q;
        } filter;
        struct IIR_EQ {
            double fc; double Q; double G_dB;
        } eq;
        struct IIR_SHELF {
            double fc_low; double fc_high; double low_dB; double high_dB; double slope;
        } shelf;
    } value;
)

#endif //IIR_H
