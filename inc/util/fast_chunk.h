#ifndef CHUNK_H
#define CHUNK_H
#include <spa/buffer/buffer.h>


typedef struct {
    float data[512];
    volatile uint32_t length;
    volatile int ready;
} LiveChunk;

void fast_chunk_queue(LiveChunk *chunk, struct spa_chunk *s_spa_chunk);

void fast_chunk_flush(LiveChunk *chunk, struct spa_chunk *s_spa_chunk);

bool fast_chunk_is_ready(LiveChunk * chunk);

void fast_chunk_apply(LiveChunk *chunk, void *effect, double (*callback)(void *, double));

#endif //CHUNK_H
