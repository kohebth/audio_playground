#ifndef FAST_Q28_H
#define FAST_Q28_H

#include <stdint.h>

#define Q28_SHIFT 28
#define Q28_ONE 268435456  // 2^28
#define Q28_MAX 0x7FFFFFFF
#define Q28_MIN 0xFFFFFFFF
#define Q28_INV 3.725290298e-09f  // 1 / 2^28

int32_t float_to_q28(float f);
float q28_to_float(int32_t q);
int32_t q28_mul(int64_t q1, int32_t q2);

#endif //FAST_Q28_H
