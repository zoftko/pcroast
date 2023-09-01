#ifndef PCROAST_MAX6675_H
#define PCROAST_MAX6675_H

#include <stdint.h>

/**
 * Process a given reading and obtain the detected temperature.
 * @param reading Reading, as provided by a MAX6675 IC.
 * @param temp Address where the reading will be stored.
 * @return 0 if the reading is valid, otherwise non-zero.
 */
uint8_t max6675_process(uint16_t reading, float *temp);

#endif  // PCROAST_MAX6675_H
