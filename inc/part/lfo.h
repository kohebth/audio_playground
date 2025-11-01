#ifndef LFO_H
#define LFO_H

typedef struct LFO LFO;

LFO *lfo_init_sine(double fs, double fm, double ampl);

LFO *lfo_init_triangle(double fs, double fm, double ampl);

double lfo_step(LFO *lfo);

void lfo_deinit(LFO *lfo);

#endif //LFO_H
