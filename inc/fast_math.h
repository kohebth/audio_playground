#ifndef TANH_H
#define TANH_H

double fast_exp(double x);

double fast_sigmoid(double x);

double fast_tanh_safe(double x);

// double fast_interpolate_cubic4(double xm1, double x0, double x1, double x2, double mu);
double fast_interpolate_cubic4(int16_t xm1, int16_t x0, int16_t x1, int16_t x2, double mu);

double fast_interpolate_linear(int16_t x0, int16_t x1, double mu);

double fast_tanh(double x);

double fast_atan(double x);

#endif //TANH_H
