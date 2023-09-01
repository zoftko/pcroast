#include "max6675.h"

static uint16_t LOW_BITS_MASK = (1 << 15) | (1 << 2) | (1 << 1);

uint8_t max6675_process(uint16_t reading, float *temp) {
    if (reading & LOW_BITS_MASK) { return 1; }

    reading = reading >> 3;
    *temp = (float)(reading * 0.25);

    return 0;
}
