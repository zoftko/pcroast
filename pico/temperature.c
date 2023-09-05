#include "temperature.h"

#include <FreeRTOS.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <task.h>

#include "graphics.h"
#include "logging.h"
#include "max6675.h"
#include "pinout.h"

static uint16_t reading;
static float temperature;

void vReadTemperatureTask(__unused void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
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
