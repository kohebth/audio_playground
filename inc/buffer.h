#ifndef BUFFER_H
#define BUFFER_H

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif //BUFFER_SIZE

#include <cstdint>

struct audio_buffer {
    uint8_t data[BUFFER_SIZE];
    uint32_t size;
    volatile int ready;
};

struct audio_pipeline {
    uint8_t data[BUFFER_SIZE];
    uint32_t size;
    volatile int order;
};

#endif //BUFFER_H
