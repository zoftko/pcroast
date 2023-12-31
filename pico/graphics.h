#ifndef PCROAST_GRAPHICS_H
#define PCROAST_GRAPHICS_H

#include <gfx.h>
#include <stdio.h>

void graphicsInit();
void graphicsSetTemperature(float temperature);
void graphicsSetDutyCycle(uint8_t duty);
void graphicsClearDutyCycle(void);

#endif
