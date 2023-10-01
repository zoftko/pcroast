#ifndef PCROAST_PID_H
#define PCROAST_PID_H

struct PidController {
    float output;
    float gainPro;
    float gainInt;
    float gainDer;
    float error;
    float sumError;
    float lastError;
};

void pid_compute_error(float reading, float target, struct PidController *controller);

#endif  // PCROAST_PID_H
