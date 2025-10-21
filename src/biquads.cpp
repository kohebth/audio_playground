#include <cmath>
#include <cstdlib>

typedef struct {
    double b0, b1, b2;
    double a1, a2;   // a0 assumed 1 after normalization
    double s1, s2;   // delay states for DF2T (transposed)
} Biquad;

void biquad_init_bandpass(Biquad *bz, double Fs, double f0, double Q)
{
    double omega = 2.0 * M_PI * f0 / Fs;
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn / (2.0 * Q);

    double b0 =  alpha;
    double b1 =  0.0;
    double b2 = -alpha;
    double a0 =  1.0 + alpha;
    double a1 = -2.0 * cs;
    double a2 =  1.0 - alpha;

    // normalize
    bz->b0 = b0 / a0;
    bz->b1 = b1 / a0;
    bz->b2 = b2 / a0;
    bz->a1 = a1 / a0;
    bz->a2 = a2 / a0;

    bz->s1 = bz->s2 = 0.0;
}

inline double biquad_apply(Biquad *bz, double x)
{
    // Direct Form II Transposed:
    double y = bz->b0 * x + bz->s1;
    bz->s1 = bz->b1 * x - bz->a1 * y + bz->s2;
    bz->s2 = bz->b2 * x - bz->a2 * y;
    return y;
}

// Process buffer in-place: samples[] length N
void biquad_process_inplace(Biquad *bz, int nSections, double samples[], size_t N)
{
    // For each section, run over the buffer (cascaded)
    for (int s = 0; s < nSections; ++s) {
        Biquad *sec = &bz[s];
        // reset states if you want per-block continuity? keep states between calls for streaming.
        // sec->s1 = sec->s2 = 0.0; // uncomment to reset
        for (size_t n = 0; n < N; ++n) {
            samples[n] = biquad_apply(sec, samples[n]);
        }
    }
}

int main() {
    double samples[48000] = {};
    size_t Nsamples = 48000;
    // Example: design a cascade of 2 biquads for 75Hz band
    double Fs = 48000.0;
    double f0 = 75.0;
    double Q = 6.0;
    int nSections = 2;
    Biquad *cascade;
    cascade = static_cast<Biquad *>(malloc(sizeof(Biquad) * nSections));
    for (int i = 0; i < nSections; ++i) biquad_init_bandpass(&cascade[i], Fs, f0, Q);
    // Fill samples[] with signal (double), then:
    biquad_process_inplace(cascade, nSections, samples, Nsamples);
    free(cascade);

}
