#include "beeper.h"

void beeper_parse(uint32_t value, struct BeeperConfig *config) {
    config->beeps = value;
    config->msOn = (value >> 8) & 0x3FF;
    config->msOff = (value >> 20);
}

uint32_t beeper_dump(const struct BeeperConfig *config) {
    return ((config->msOff << 20) | (config->msOn << 8) | config->beeps);
}
