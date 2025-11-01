#include <ring.h>

#include <stdlib.h>

struct Ring {
    size_t length;
    double *begin;
    double *end;
    volatile double *cursor;
};

Ring *ring_init(const size_t size) {
    Ring * p_buffer = (Ring *)(malloc(sizeof(Ring)));
    p_buffer->begin = (double *)(malloc(size * sizeof(double)));
    p_buffer->end = p_buffer->begin + size;
    p_buffer->length = size;
    p_buffer->cursor = p_buffer->begin;
    return p_buffer;
}

void ring_deinit(Ring *p_buffer) {
    free(p_buffer->begin);
    free(p_buffer);
}

double ring_get(const Ring *p_buffer, const int32_t offset) {
    volatile double *read_cursor = p_buffer->cursor + offset;
    if (read_cursor < p_buffer->begin) {
        return *(read_cursor + p_buffer->length);
    }
    if (read_cursor >= p_buffer->end) {
        return *(read_cursor - p_buffer->length);
    }
    return *read_cursor;
}

double ring_add(Ring *p_buffer, const double value) {
    ++p_buffer->cursor;
    if (p_buffer->cursor >= p_buffer->end) {
        p_buffer->cursor -= p_buffer->length;
    }
    return *p_buffer->cursor = value;
}
