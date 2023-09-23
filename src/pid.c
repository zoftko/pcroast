#include "pid.h"

void pid_compute_error(float reading, float target, struct PidController *controller) {
    float error = target - reading;

    controller->error2 = controller->error1;
    controller->error1 = controller->error;
    controller->error = error;

    float deltaOut = (controller->gainK1 * controller->error) +
                     (controller->gainK2 * controller->error1) +
                     (controller->gainK3 * controller->error2);
    controller->output += deltaOut;
}
