#ifndef BUFFER_H
#define BUFFER_H

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 2048
#endif //BUFFER_SIZE

#include <cstdint>
#include <cstddef>

struct audio_buffer {
    float data[BUFFER_SIZE];
    volatile uint32_t length;
    volatile int ready;
};

struct RingBuffer {
    size_t length;
    double *begin;
    double *end;
    volatile double *cursor;
};

RingBuffer *buffer_init(size_t size);

void buffer_deinit(RingBuffer *p_buffer);

double buffer_get(const RingBuffer *p_buffer, int32_t offset);

double buffer_add(RingBuffer *p_buffer, double value);

// struct audio_buffer_iter {
//     int16_t *iter;
//     int16_t *wrapLow;
//     int16_t *wrapHigh;
// };
//
// audio_buffer_iter audio_buffer_iterate(audio_buffer *buffer, size_t offset);
//
// void prev(audio_buffer_iter *it);
//
// void next(audio_buffer_iter *it);

#endif //BUFFER_H
