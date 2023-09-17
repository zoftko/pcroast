#include <hardware/gpio.h>

#include "pinout.h"

static volatile uint8_t dutyCycle = 100;
static volatile uint32_t zeroCrossEvents = 0;

void vZeroCrossCallback(void) {
    if (gpio_get_irq_event_mask(ZERO_CROSS_GPIO) & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(ZERO_CROSS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    } else {
        return;
    }

    zeroCrossEvents++;
    if (zeroCrossEvents == 99) {
        zeroCrossEvents = 0;
        gpio_put(SSR_CONTROL_GPIO, 1);
    } else if (zeroCrossEvents > dutyCycle) {
        gpio_put(SSR_CONTROL_GPIO, 0);
    }
}
