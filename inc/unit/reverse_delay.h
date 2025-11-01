#ifndef REVERSE_DELAY_H
#define REVERSE_DELAY_H
#include <stdint.h>

typedef struct ReverseDelay ReverseDelay;

ReverseDelay *reverse_delay_init(uint32_t fs, uint32_t threshold_dB, uint32_t time_ms, double fdback_dB, double mix);
double reverse_delay_process(ReverseDelay *delay, double dry);
void reverse_delay_deinit(ReverseDelay *delay);

#endif //REVERSE_DELAY_H
