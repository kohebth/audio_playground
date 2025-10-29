#ifndef EFFECT_H
#define EFFECT_H

#include <buffer.h>
#include <ctime>

struct Envelope {
    double gain;
    double threshold;
    double ratio;
    time_t attack;
    time_t release;
};

struct Gain {
    double gain;
    double tone;
    double volume;
};

struct Modulate {
    double freq;
    double depth;
    double mix;
};

struct Delay {
    double feedback;
    time_t time;
    double mix;
};

struct Reverb {
    double damp;
    time_t time;
    double mix;
};

void chain_procces(audio_buffer *chunk);
void chain_init();
void chain_deinit();
#endif //EFFECT_H
