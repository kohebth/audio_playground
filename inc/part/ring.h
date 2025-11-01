#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct Ring Ring;

Ring *ring_init(size_t size);

void ring_deinit(Ring *p_buffer);

double ring_get(const Ring *p_buffer, int32_t offset);

double ring_add(Ring *p_buffer, double value);

#endif //BUFFER_H
