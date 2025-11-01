#include <env.h>

#include <math.h>
#include <stdlib.h>
#include <fast_math.h>


Envelope *env_init(double fs, double attack_ms, double release_ms) {
    Envelope *p_env = malloc(sizeof(Envelope));
    p_env->envelope = 0.0;
    p_env->attack_coef = fast_exp(-1.0 / (fs * attack_ms * 0.001));
    p_env->sup_attack_coef = 1.0 - p_env->attack_coef;
    p_env->release_coef = fast_exp(-1.0 / (fs * release_ms * 0.001));
    p_env->sup_release_coef = 1.0 - p_env->release_coef;
    return p_env;
}

double env_detect(Envelope *p_env, double x) {
    const double abs_x = fabs(x);
    p_env->envelope = abs_x > p_env->envelope
                               ? p_env->attack_coef * p_env->envelope + p_env->sup_attack_coef * abs_x
                               : p_env->release_coef * p_env->envelope + p_env->sup_release_coef * abs_x;
    return p_env->envelope;
}

void envelope_deinit(Envelope *p_env) {
    free(p_env);
}
