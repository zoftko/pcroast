#ifndef PCROAST_BEEPER_H
#define PCROAST_BEEPER_H

#include <stdint.h>

struct BeeperConfig {
    uint8_t beeps  : 8;
    uint16_t msOn  : 12;
    uint16_t msOff : 12;
};

void beeper_parse(uint32_t value, struct BeeperConfig *config);
uint32_t beeper_dump(const struct BeeperConfig *config);

#endif  // PCROAST_BEEPER_H
