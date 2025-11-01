#ifndef TANH_H
#define TANH_H

#define fast_max(a,b) ((a > b) ? a : b)

#define fast_min(a,b) ((a < b) ? a : b)

#define fast_clamp(x,a,b) ((x < a) ? a : ((x > b) ? b: x))

double fast_exp(double x);

double fast_sigmoid(double x);

double fast_tanh_safe(double x);

// double fast_interpolate_cubic4(double xm1, double x0, double x1, double x2, double mu);
double fast_interpolate_cubic4(float xm1, float x0, float x1, float x2, double mu);

double fast_interpolate_linear(float x0, float x1, double mu);

double fast_tanh(double x);

double fast_atan(double x);

#endif //TANH_H
