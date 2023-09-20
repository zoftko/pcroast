#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "graphics.h"
#include "logging.h"
#include "max6675.h"
#include "pinout.h"

static volatile uint8_t dutyCycle = 100;
static volatile uint32_t zeroCrossEvents = 0;

static uint16_t reading;
static float temperature;

void vReadTemperatureTask(__unused void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_put(SPI1_6675_CS, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_put(SPI1_6675_CS, 1);
        vTaskDelay(pdMS_TO_TICKS(5));

        gpio_put(SPI1_6675_CS, 0);
        spi_read16_blocking(spi1, 0, &reading, 1);
        gpio_put(SPI1_6675_CS, 1);

        if (max6675_process(reading, &temperature)) {
            LOG_WARNING("incorrect reading from spi1");
        } else {
            graphicsSetTemperature(temperature);
        }
    }
}

void vZeroCrossCallback(void) {
    if (gpio_get_irq_event_mask(ZERO_CROSS_GPIO) & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(ZERO_CROSS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    } else {
        return;
    }

    if (zeroCrossEvents == 99) {
        zeroCrossEvents = 0;
        gpio_put(SSR_CONTROL_GPIO, 1);
    } else if (zeroCrossEvents > dutyCycle) {
        gpio_put(SSR_CONTROL_GPIO, 0);
    }
    zeroCrossEvents++;
}
