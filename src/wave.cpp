#include "wave.h"

#include <vector>

bool readWav(const char* filename, std::vector<float>& samples, float& fs) {
    FILE* f = fopen(filename, "rb");
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
    fs = (float)hdr.sampleRate;

    int numSamples = hdr.dataSize / (hdr.bitsPerSample / 8);
    samples.resize(numSamples);

    for (int i = 0; i < numSamples; i++) {
        int16_t v;
        sz = fread(&v, sizeof(v), 1, f);
        printf("Read %s %ld", filename, sz);
        samples[i] = v / 32768.0f;
    }

    fclose(f);
    return true;
}

void writeWav(const char* filename, const std::vector<float>& samples, float fs) {
    size_t sz;
    WAVHeader hdr = {
        {'R','I','F','F'},
        0,
        {'W','A','V','E'},
        {'f','m','t',' '},
        16, 1, 1,
        (uint32_t)fs,
        0, 0, 16,
        {'d','a','t','a'},
        0
    };

    hdr.blockAlign = hdr.numChannels * hdr.bitsPerSample / 8;
    hdr.byteRate = hdr.sampleRate * hdr.blockAlign;
    hdr.dataSize = samples.size() * hdr.blockAlign;
    hdr.chunkSize = 36 + hdr.dataSize;

    FILE* f = fopen(filename, "wb");
    sz = fwrite(&hdr, sizeof(hdr), 1, f);
    printf("Written %s %ld", filename, sz);
    for (float s : samples) {
        int16_t v = (int16_t)(std::clamp(s, -1.0f, 1.0f) * 32767);
        sz = fwrite(&v, sizeof(v), 1, f);
        printf("Written %s %ld", filename, sz);
    }
    fclose(f);
}
