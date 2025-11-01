#ifndef EFFECT_H
#define EFFECT_H

#include <time.h>
#include <fast_chunk.h>

typedef struct {
    double gain;
    double threshold;
    double ratio;
    time_t attack;
    time_t release;
} Dynamic;

typedef struct {
    double gain;
    double tone;
    double volume;
} Gain;

typedef struct {
    double freq;
    double depth;
    double mix;
} Modulate;

typedef struct {
    double feedback;
    time_t time;
    double mix;
} Delay;

typedef struct  {
    double damp;
    time_t time;
    double mix;
} Reverb;

void chain_procces(LiveChunk *chunk);

void chain_init();

void chain_deinit();
#endif //EFFECT_H
