#include <buffer.h>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstdio>

typedef double Buffer_data;

RingBuffer *buffer_init(const size_t size) {
    const auto p_buffer = static_cast<RingBuffer *>(malloc(sizeof(RingBuffer)));
    p_buffer->begin = static_cast<double *>(malloc(size * sizeof(double)));
    p_buffer->end = p_buffer->begin + size;
    p_buffer->length = size;
    p_buffer->cursor = p_buffer->begin;
    return p_buffer;
}

void buffer_deinit(RingBuffer *p_buffer) {
    free(p_buffer->begin);
    free(p_buffer);
}

double buffer_get(const RingBuffer *p_buffer, const int32_t offset) {
    volatile double *read_cursor = p_buffer->cursor + offset;
    if (read_cursor < p_buffer->begin) {
        return *(read_cursor + p_buffer->length);
    }
    if (read_cursor >= p_buffer->end) {
        return *(read_cursor - p_buffer->length);
    }
    return *read_cursor;
}

double buffer_add(RingBuffer *p_buffer, const double value) {
    ++p_buffer->cursor;
    if (p_buffer->cursor >= p_buffer->end) {
        p_buffer->cursor -= p_buffer->length;
    }
    return *p_buffer->cursor = value;
}
