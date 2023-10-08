#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "beeper.h"
#include "graphics.h"
#include "logging.h"
#include "max6675.h"
#include "pid.h"
#include "pinout.h"

enum ProfileStage { RAMP_TO_SOAK, SOAKING, RAMP_TO_REFLOW, COOLING, IDLE };

struct Profile {
    float soakTemp;
    float reflowTemp;
    uint8_t soakTime;
};

struct Profile lowTempProfile = {.soakTemp = 90, .soakTime = 60, .reflowTemp = 150};

struct PidController controller = {
    .output = 0,
    .gainPro = 3.0f,
    .gainInt = 0.25f,
    .gainDer = -100.0f,
    .sumError = 0,
    .lastError = 0,
    .error = 0,
};

const struct BeeperConfig beeperAlertEnd = {.beeps = 5, .msOn = 600, .msOff = 400};

extern volatile uint8_t reflowStarted;

static volatile uint16_t seconds = 0;
static volatile uint16_t soakSeconds = 0;
static volatile enum ProfileStage stage = IDLE;

static volatile uint8_t dutyCycle = 0;
static volatile uint32_t zeroCrossEvents = 0;

static uint16_t reading;
static float temperature;
static float targetTemperature = 0;

extern TaskHandle_t controlReflowTaskHandle;
extern TaskHandle_t beepTaskHandle;

static void clearControllerError() {
    controller.sumError = 0;
    controller.lastError = 0;
    controller.error = 0;
}

static void setStageLED(uint8_t gpio) {
    switch (gpio) {
        case LED_IDLE_GPIO:
            gpio_put(LED_IDLE_GPIO, 1);
            gpio_put_masked((1 << LED_WORKING_GPIO) | (1 << LED_COOLING_GPIO), 0);
            break;
        case LED_WORKING_GPIO:
            gpio_put(LED_WORKING_GPIO, 1);
            gpio_put_masked((1 << LED_IDLE_GPIO) | (1 << LED_COOLING_GPIO), 0);
            break;
        case LED_COOLING_GPIO:
            gpio_put(LED_COOLING_GPIO, 1);
            gpio_put_masked((1 << LED_IDLE_GPIO) | (1 << LED_WORKING_GPIO), 0);
            break;
        default:
            gpio_put_masked(LED_MASK, 0);
    }
}

void vInitControl() {
    setStageLED(LED_IDLE_GPIO);
    stage = IDLE;
}

void vStartControl() {
    LOG_INFO("ramp to soak");
    setStageLED(LED_WORKING_GPIO);
    clearControllerError();
    stage = RAMP_TO_SOAK;
}

void vStopControl() {
    stage = COOLING;
    dutyCycle = 0;
    gpio_put(SSR_CONTROL_GPIO, 0);
    graphicsClearDutyCycle();
    setStageLED(LED_COOLING_GPIO);
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
                    clearControllerError();
                    stage = RAMP_TO_REFLOW;
                }
                break;
            case RAMP_TO_REFLOW:
                targetTemperature = lowTempProfile.reflowTemp;
                if (temperature >= lowTempProfile.reflowTemp) {
                    LOG_INFO("cooling");
                    xTaskNotify(
                        beepTaskHandle, beeper_dump(&beeperAlertEnd), eSetValueWithOverwrite
                    );
                    vStopControl();
                    gpio_put(LED_COOLING_GPIO, 1);
                    continue;
                }
                break;
            case COOLING:
                if (temperature <= 50) {
                    setStageLED(LED_IDLE_GPIO);
                    reflowStarted = false;
                    stage = IDLE;
                }
                continue;
        }

        pid_compute_error(temperature, targetTemperature, &controller);
        dutyCycle = (uint8_t)controller.error;
        graphicsSetDutyCycle(dutyCycle);
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
