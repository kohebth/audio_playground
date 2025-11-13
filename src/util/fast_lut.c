#include <fast_lut.h>


// Applied to i = [1.0,2.0], step = 1/8 where [1,9/8] uses the approximation of log2p1(x) or poly(x + 1) = x * poly(x)
TaylorLog2 log2coeffs[8] = {
    {-0x1.64f7c0p-2, 0x1.efbb3cp-2, -0x1.715f54p-1, 0x1.715426p+0}, // max error = 5.296e-06
    {0x1.28d114p-2, -0x1.8b7fbap+0, 0x1.d433bcp+1, -0x1.3386fap+1}, // max error = 9.239e-07
    {0x1.b831acp-3, -0x1.43f620p+0, 0x1.a7b6c4p+1, -0x1.21134ap+1}, // max error = 7.153e-07
    {0x1.4ebba0p-3, -0x1.0de472p+0, 0x1.82bd60p+1, -0x1.103694p+1}, // max error = 5.364e-07
    {0x1.05f106p-3, -0x1.ca5feap-1, 0x1.646202p+1, -0x1.011ddap+1}, // max error = 4.768e-07
    {0x1.9a87e8p-4, -0x1.859da0p-1, 0x1.489212p+1, -0x1.e43a1cp+0}, // max error = 5.960e-07
    {0x1.4bf17cp-4, -0x1.521b0ep-1, 0x1.320f2ep+1, -0x1.c9fd06p+0}, // max error = 4.768e-07
    {0x1.0f8c04p-4, -0x1.27b4f4p-1, 0x1.1e3634p+1, -0x1.b134e8p+0}, // max error = 4.768e-07
};

// Applied to i = [-1.0,1.0], step = 1/2
TaylorExp2 exp2coeffs[4] = {
{ 0x1.10e5a00000000p-5, 0x1.bedd3a0000000p-3, 0x1.5dced00000000p-1, 0x1.ff25760000000p-1 }, // max error = 6.192e-06
{ 0x1.7c6e6e0000000p-5, 0x1.e6191e0000000p-3, 0x1.62bcbe0000000p-1, 0x1.ffff540000000p-1 }, // max error = 5.169e-06
{ 0x1.0ecbbc0000000p-4, 0x1.e50bc00000000p-3, 0x1.63102c0000000p-1, 0x1.ffff560000000p-1 }, // max error = 5.149e-06
{ 0x1.7e652c0000000p-4, 0x1.8f54000000000p-3, 0x1.6e72b40000000p-1, 0x1.fdeab60000000p-1 }, // max error = 4.953e-06
};
