#ifndef UNIT_H
#define UNIT_H

#include <stdint.h>

#define UNIT(name,statements,settings) \
typedef struct { statements } name; \
typedef struct { uint32_t fs; settings } name##Tune; \
name *init_##name(void); \
float apply_##name(name *p_##unit, const float x); \
void tune_##name(name *p_##unit, const name##Tune *p_##tune); \
void deinit_##name(name *p_##unit);


#endif //UNIT_H
