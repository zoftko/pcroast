#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "graphics.h"
#include "logging.h"
#include "max6675.h"
#include "pid.h"
#include "pinout.h"

enum ProfileStage { RAMP_TO_SOAK, SOAKING, RAMP_TO_REFLOW, IDLE };

struct Profile {
    float soakTemp;
    float reflowTemp;
    uint8_t soakTime;
};

struct Profile lowTempProfile = {.soakTemp = 90, .soakTime = 60, .reflowTemp = 150};

struct PidController controller = {
    .output = 0,
    .gainK1 = 66.42f,
    .gainK2 = -57.33f,
    .gainK3 = 27.5f,
    .error2 = 0,
    .error1 = 0,
    .error = 0,
};

static volatile uint16_t seconds = 0;
static volatile uint16_t soakSeconds = 0;
static volatile enum ProfileStage stage = IDLE;

static volatile uint8_t dutyCycle = 0;
static volatile uint32_t zeroCrossEvents = 0;

static uint16_t reading;
static float temperature;
static float targetTemperature = 0;

extern TaskHandle_t controlReflowTaskHandle;

void vStartControl() {
    LOG_INFO("ramp to soak");
    stage = RAMP_TO_SOAK;
}

void vControlReflowTask(__unused void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

        switch (stage) {
            case IDLE:
                continue;
            case RAMP_TO_SOAK:
                targetTemperature = lowTempProfile.soakTemp;
                if (temperature >= lowTempProfile.soakTemp) {
                    LOG_INFO("soaking");
                    stage = SOAKING;
                }
                break;
            case SOAKING:
                soakSeconds++;
                if (soakSeconds >= lowTempProfile.soakTime) {
                    LOG_INFO("ramp to reflow");
                    controller.error2 = 0;
                    controller.error1 = 0;
                    controller.error = 0;

                    stage = RAMP_TO_REFLOW;
                }
                break;
            case RAMP_TO_REFLOW:
                targetTemperature = lowTempProfile.reflowTemp;
                if (temperature >= lowTempProfile.reflowTemp) {
                    LOG_INFO("reflow ended");
                    stage = IDLE;
                    dutyCycle = 0;
                    continue;
                }
                break;
        }

        pid_compute_error(temperature, targetTemperature, &controller);
        if (controller.error <= 0) {
            dutyCycle = 0;
        } else if (controller.error > 100) {
            dutyCycle = 100;
        } else {
            dutyCycle = (uint8_t)controller.error;
        }
        seconds++;
    }
}

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
            xTaskNotify(controlReflowTaskHandle, 0, eNoAction);
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
