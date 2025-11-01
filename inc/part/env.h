#ifndef ENVELOPE_H
#define ENVELOPE_H

typedef struct Envelope Envelope;

struct Envelope {
    volatile double envelope;
    double attack_coef;
    double sup_attack_coef;
    double release_coef;
    double sup_release_coef;
};

Envelope *env_init(double fs, double attack_ms, double release_ms);
double env_detect(Envelope *p_env, double x);
void envelope_deinit(Envelope *p_env);

#endif //ENVELOPE_H
