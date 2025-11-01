#ifndef REVERSE_DELAY_H
#define REVERSE_DELAY_H
#include <stdint.h>

typedef struct ReverseDelay RevDelay;

RevDelay *init_RevDelay();

RevDelay *tune_RevDelay(RevDelay *p_RevDelay, uint32_t fs, uint32_t threshold_dB, uint32_t time_ms, double fdback_dB, double mix);

double apply_RevDelay(RevDelay *delay, double dry);

void deinit_RevDelay(RevDelay *delay);

#endif //REVERSE_DELAY_H
