#ifndef IIR_H
#define IIR_H

#include <stdint.h>


typedef struct IIRFilter IIRFilter;

IIRFilter *iir_init_HPF(uint32_t fs, double fc, double Q);

IIRFilter *iir_init_LPF(uint32_t fs, double fc, double Q);

IIRFilter *iir_init_BPF(uint32_t fs, double fc, double Q);

IIRFilter *iir_init_EQ(uint32_t fs, double fc, double Q, double G_dB);

IIRFilter *iir_init_LSF(uint32_t fs, double fc_low, double low_dB, double fc_high, double high_dB, double slope);

IIRFilter *iir_init_HSF(uint32_t fs, double fc_low, double low_dB, double fc_high, double high_dB, double slope);

void iir_deinit(IIRFilter *p_filter);

double iir_process(IIRFilter *p_filter, double x);

#endif //IIR_H
