#include <fast_chunk.h>

#include <string.h>

void fast_chunk_apply(LiveChunk *chunk, void *effect, double (*callback)(void *, double)) {
    float *x = chunk->data;
    float *end_x = x + chunk->length;
    while (x < end_x) {
        *x = (float) callback(effect, *x);
        ++x;
    }
}

void fast_chunk_queue(LiveChunk *chunk, struct spa_chunk *s_spa_chunk) {
    const size_t byte_size = s_spa_chunk->size;
    memcpy(chunk->data, s_spa_chunk, byte_size);
    chunk->length = byte_size / sizeof(float);
    __sync_synchronize();
    chunk->ready = 1;
}

void fast_chunk_flush(LiveChunk *chunk, struct spa_chunk *s_spa_chunk) {
    size_t byte_size = chunk->length * sizeof(float);

    memcpy(s_spa_chunk, chunk->data, byte_size);
    s_spa_chunk->size = byte_size;
    s_spa_chunk->stride = 4;
    __sync_synchronize();
    chunk->ready = 0;
}

bool fast_chunk_is_ready(LiveChunk *chunk) {
    __sync_synchronize();
    return chunk->ready;
}
