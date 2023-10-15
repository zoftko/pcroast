#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "beeper.h"
#include "graphics.h"
#include "kernel.h"
#include "logging.h"
#include "max6675.h"
#include "pid.h"
#include "pinout.h"

enum ProfileStage { RAMP_TO_SOAK, SOAKING, RAMP_TO_REFLOW, COOLING, IDLE };
enum RequestType { CREATE_SESSION, POST_MEASUREMENTS };

struct Profile {
    float soakTemp;
    float reflowTemp;
    uint8_t soakTime;
};

struct Measurement {
    int sequence;
    float temperature;
};

struct Profile lowTempProfile = {.soakTemp = 110, .soakTime = 70, .reflowTemp = 165};

static char sessionPayload[256];
struct HttpRequest newSessionRequest = {
    .request_line = "POST /api/session HTTP/1.1\r\n", .payload = sessionPayload};

static volatile struct Measurement measurements[20];
static char measurementPayload[512];
struct HttpRequest postMeasurementRequest = {
    .request_line = "POST /api/measurement HTTP/1.1\r\n", .payload = measurementPayload};

struct PidController soakController = {
    .output = 0,
    .gainPro = 2.5f,
    .gainInt = 0.25f,
    .gainDer = -66.5f,
    .sumError = 0,
    .lastError = 0,
    .error = 0,
};

struct PidController reflowController = {
    .output = 0,
    .gainPro = 3.5f,
    .gainInt = 0.35f,
    .gainDer = -20.0f,
    .sumError = 0,
    .lastError = 0,
    .error = 0,
};

struct PidController *currentController = &soakController;

const struct BeeperConfig beeperAlertEnd = {.beeps = 5, .msOn = 600, .msOff = 400};
const struct BeeperConfig beeperSoakStart = {.beeps = 1, .msOn = 1000, .msOff = 0};
const struct BeeperConfig beeperSoakEnd = {.beeps = 2, .msOn = 500, .msOff = 300};

extern volatile uint8_t reflowStarted;

static volatile uint16_t seconds = 0;
static volatile uint16_t soakSeconds = 0;
static volatile enum ProfileStage stage = IDLE;

static volatile uint8_t dutyCycle = 0;
static volatile uint32_t zeroCrossEvents = 0;

static uint16_t reading;
static float tempReading;
static float tempAccumulator;
static float temperature = 0;
static float targetTemperature = 0;

extern TaskHandle_t httpRequestTaskHandle;
extern TaskHandle_t controlHttpLoggerHandle;
extern TaskHandle_t controlReflowTaskHandle;
extern TaskHandle_t beepTaskHandle;

static void clearControllerError() {
    soakController.sumError = 0;
    soakController.lastError = 0;
    soakController.error = 0;

    reflowController.sumError = 0;
    reflowController.lastError = 0;
    reflowController.error = 0;
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
    xTaskNotify(controlHttpLoggerHandle, CREATE_SESSION, eSetValueWithOverwrite);
    setStageLED(LED_WORKING_GPIO);
    clearControllerError();
    currentController = &soakController;
    seconds = 0;
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
                    xTaskNotify(
                        beepTaskHandle, beeper_dump(&beeperSoakStart), eSetValueWithOverwrite
                    );
                    stage = SOAKING;
                }
                break;
            case SOAKING:
                soakSeconds++;
                if (soakSeconds >= lowTempProfile.soakTime) {
                    LOG_INFO("ramp to reflow");
                    xTaskNotify(
                        beepTaskHandle, beeper_dump(&beeperSoakEnd), eSetValueWithOverwrite
                    );
                    clearControllerError();
                    currentController = &reflowController;
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
                    LOG_INFO("idle");
                    setStageLED(LED_IDLE_GPIO);
                    reflowStarted = false;
                    stage = IDLE;
                    continue;
                }
        }

        if (stage != COOLING) {
            pid_compute_error(temperature, targetTemperature, currentController);
            dutyCycle = (uint8_t)currentController->error;
            graphicsSetDutyCycle(dutyCycle);
        }

        volatile struct Measurement *measurement = &(measurements[seconds % 20]);
        measurement->temperature = temperature;
        measurement->sequence = seconds;
        seconds++;

        if ((seconds != 0) && ((seconds % 10) == 0)) {
            xTaskNotify(controlHttpLoggerHandle, POST_MEASUREMENTS, eSetValueWithOverwrite);
        }
    }
}

void vReadTemperatureTask(__unused void *pvParameters) {
    static uint8_t temperatureReads = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000 / TEMP_READING_AMOUNT));
        gpio_put(SPI1_6675_CS, 0);
        vTaskDelay(pdMS_TO_TICKS(5));
        gpio_put(SPI1_6675_CS, 1);
        vTaskDelay(pdMS_TO_TICKS(5));

        gpio_put(SPI1_6675_CS, 0);
        spi_read16_blocking(spi1, 0, &reading, 1);
        gpio_put(SPI1_6675_CS, 1);

        if (max6675_process(reading, &tempReading)) {
            LOG_WARNING("incorrect reading from spi1");
        } else {
            temperatureReads++;
            tempAccumulator += tempReading;
            if (temperatureReads != TEMP_READING_AMOUNT) { continue; }

            temperatureReads = 0;
            temperature = tempAccumulator / TEMP_READING_AMOUNT;
            tempAccumulator = 0;
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

void vControlHttpLogger(__unused void *pvParameters) {
    int bytes;
    int bytes_left;
    int data_offset = 0;
    char *payloadOffset;
    enum RequestType request;
    snprintf(
        sessionPayload, 256,
        "{\"soak_temperature\": %g, \"soak_time\": %d, \"reflow_peak_temp\": %g}",
        lowTempProfile.soakTemp, lowTempProfile.soakTime, lowTempProfile.reflowTemp
    );
    while (1) {
        request = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        switch (request) {
            case CREATE_SESSION:
                xTaskNotify(
                    httpRequestTaskHandle, (uint32_t)&newSessionRequest, eSetValueWithOverwrite
                );
                break;
            case POST_MEASUREMENTS:
                bytes_left = 512;
                payloadOffset = measurementPayload;

                bytes = snprintf(payloadOffset, bytes_left, "{\"measurements\": [");
                payloadOffset += bytes;
                bytes_left -= bytes;
                for (int count = data_offset; count < (data_offset + 10); count++) {
                    bytes = snprintf(
                        payloadOffset, bytes_left, "{\"temp\": %f, \"sequence\": %d},",
                        measurements[count].temperature, measurements[count].sequence
                    );
                    payloadOffset += bytes;
                    bytes_left -= bytes;
                }
                // Remove trailing comma by overriding it
                payloadOffset--;
                bytes_left--;
                snprintf(payloadOffset, bytes_left, "]}");
                data_offset = data_offset == 0 ? 10 : 0;
                xTaskNotify(
                    httpRequestTaskHandle, (uint32_t)&postMeasurementRequest, eSetValueWithOverwrite
                );
                break;
        }
    }
}
