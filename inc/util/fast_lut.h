#ifndef FAST_LUT_H
#define FAST_LUT_H

#include <stdint.h>

typedef struct {
    float coef[4];
} TaylorExp2;

typedef struct {
    float coef[4];
} TaylorLog2;

extern TaylorExp2 exp2coeffs[4];
extern TaylorLog2 log2coeffs[8];

#endif //FAST_LUT_H
