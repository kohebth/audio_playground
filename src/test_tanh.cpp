#include <cmath>
#include<fast_math.h>
#include <string>

void elapse(void (*func)(), std::string func_name) {
    long start, stop;
    start = clock();
    func();
    stop = clock();
    printf("f=%s time=%ldns\n", &func_name[0], stop - start);
}

void test_tanh() {
    fast_tanh(10);
}

void std_tanh() {
    std::tanh(10);
}

void test_fst() {
    double xx_diff = 0.0;
    for (double x = -32.0; x < 32.0; x += 0.000001) {
        xx_diff += fast_tanh(x);
    }
    printf("%0.12f ", xx_diff);
}

void test_std() {
    double xx_diff = 0.0;
    for (double x = -32.0; x < 32.0; x += 0.000001) {
        xx_diff += tanh(x);
    }
    printf("%0.12f ", xx_diff);
}

// int main() {
//     elapse(test_tanh, "fast_tanh");
//     elapse(std_tanh, "std_tanh");
//
//     elapse(test_std, "test_std");
//     elapse(test_fst, "test_fst");
// }

int main(int argc, char *argv[]) {
    double k;
    k = fast_atan(-1.0);
    printf("%lf\n", k);
    k = fast_atan(1.0);
    printf("%lf\n", k);
}
