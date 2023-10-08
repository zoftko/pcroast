#include "pid.h"

static inline void limit_float(float *target, float max, float min) {
    if (*target > max) {
        *target = max;
    } else if (*target < min) {
        *target = min;
    }
}

void pid_compute_error(float reading, float target, struct PidController *controller) {
    float error = target - reading;
    float pterm, iterm, dterm;

    pterm = error * controller->gainPro;
    limit_float(&pterm, 200, -200);

    iterm = controller->sumError + error;
    iterm *= controller->gainInt;
    limit_float(&iterm, 100, -20);

    dterm = controller->gainDer * (controller->lastError - error);
    controller->lastError = error;
    limit_float(&dterm, 0, -100);

    controller->error = pterm + iterm + dterm;
    limit_float(&(controller->error), 100, 0);
}
