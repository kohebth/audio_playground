#include "wave.h"

#include <fast_math.h>
#include <stdio.h>

bool readWav(const char *filename, Samples *samples, float fs) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    WAVHeader hdr;
    size_t sz;
    sz = fread(&hdr, sizeof(hdr), 1, f);
    printf("Read %s %ld", filename, sz);
    if (hdr.audioFormat != 1 || hdr.bitsPerSample != 16) {
        fclose(f);
        printf("Only 16-bit PCM supported\n");
        return false;
    }
    fs = (float) hdr.sampleRate;

    samples->size = hdr.dataSize / (hdr.bitsPerSample / 8);

    for (int i = 0; i < samples->size; i++) {
        int16_t v;
        sz = fread(&v, sizeof(v), 1, f);
        printf("Read %s %ld", filename, sz);
        samples->data[i] = v / 32768.0f;
    }

    fclose(f);
    return true;
}

void writeWav(const char *filename, Samples *samples, float fs) {
    size_t sz;
    WAVHeader hdr = {
        {'R', 'I', 'F', 'F'},
        0,
        {'W', 'A', 'V', 'E'},
        {'f', 'm', 't', ' '},
        16, 1, 1,
        (uint32_t) fs,
        0, 0, 16,
        {'d', 'a', 't', 'a'},
        0
    };

    hdr.blockAlign = hdr.numChannels * hdr.bitsPerSample / 8;
    hdr.byteRate = hdr.sampleRate * hdr.blockAlign;
    hdr.dataSize = samples->size * hdr.blockAlign;
    hdr.chunkSize = 36 + hdr.dataSize;

    FILE *f = fopen(filename, "wb");
    sz = fwrite(&hdr, sizeof(hdr), 1, f);
    printf("Written %s %ld", filename, sz);
    for (size_t i = 0; samples->size; ++i) {
        float s = samples->data[i];
        int16_t v = (int16_t) (fast_clamp(s, -1.0f, 1.0f) * 32767);
        sz = fwrite(&v, sizeof(v), 1, f);
        printf("Written %s %ld", filename, sz);
    }
    fclose(f);
}
