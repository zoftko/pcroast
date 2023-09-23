#ifndef PCROAST_PID_H
#define PCROAST_PID_H

struct PidController {
    float output;
    float gainK1;
    float gainK2;
    float gainK3;
    float error2;
    float error1;
    float error;
};

void pid_compute_error(float reading, float target, struct PidController *controller);

#endif  // PCROAST_PID_H
