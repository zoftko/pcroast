#include <FreeRTOS.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <task.h>

#include "beeper.h"
#include "graphics.h"
#include "logging.h"
#include "os_tasks.h"
#include "pinout.h"

TaskHandle_t wifiTaskHandle;
TaskHandle_t startupTaskHandle;
TaskHandle_t blinkLedTaskHandle;
TaskHandle_t beepTaskHandle;
TaskHandle_t temperatureTaskHandle;
TaskHandle_t startControlTaskHandle;
TaskHandle_t controlReflowTaskHandle;

const struct BeeperConfig beeperStartup = {.beeps = 2, .msOn = 100, .msOff = 100};

static volatile uint8_t reflowStarted = false;

void startButtonCallback(void) {
    if (gpio_get_irq_event_mask(START_BTN) & GPIO_IRQ_EDGE_FALL) {
        gpio_acknowledge_irq(START_BTN, GPIO_IRQ_EDGE_FALL);
    } else {
        return;
    }
    if (reflowStarted == true) return;

    reflowStarted = true;
    xTaskNotifyFromISR(startControlTaskHandle, 0, eNoAction, NULL);
}

void vApplicationStackOverflowHook(__unused TaskHandle_t xTask, char *pcTaskName) {
    LOG_ERROR("stack overflow for task %s", pcTaskName);
    LOG_ERROR("restarting system");
    watchdog_enable(1, 1);
    while (1) continue;
}

void vBlinkLedTask(__unused void *pvParameters) {
    while (1) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(20));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1980));
    }
}

void vBeepTask(__unused void *pvParameters) {
    struct BeeperConfig config;
    while (1) {
        uint32_t data = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        beeper_parse(data, &config);
        LOG_DEBUG("beeping %d - %d - %d", config.beeps, config.msOn, config.msOff);

        gpio_set_function(BUZZER_GPIO, GPIO_FUNC_PWM);
        for (uint8_t count = 0; count < config.beeps; count++) {
            pwm_set_enabled(BUZZER_PWM_SLICE, true);
            vTaskDelay(pdMS_TO_TICKS(config.msOn));
            pwm_set_enabled(BUZZER_PWM_SLICE, false);
            vTaskDelay(pdMS_TO_TICKS(config.msOff));
        }

        gpio_set_function(BUZZER_GPIO, GPIO_FUNC_SIO);
        gpio_put(BUZZER_GPIO, 0);
    }
}

void vStartControlTask(__unused void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        LOG_INFO("starting Reflow Process");
        vStartControl();
        gpio_set_irq_enabled(ZERO_CROSS_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    }
}

void vStartupTask(__unused void *pvParameters) {
    LOG_INFO("initializing graphics");
    graphicsInit();

    LOG_INFO("initializing cyw43 module");
    if (cyw43_arch_init()) {
        LOG_ERROR("failed to initialize cyw43 module");
        while (1) continue;
    }
    cyw43_arch_enable_sta_mode();

    xTaskCreate(
        vBlinkLedTask, "BlinkLED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1,
        &blinkLedTaskHandle
    );
    configASSERT(&blinkLedTaskHandle);
    LOG_DEBUG("blink task created");

    xTaskCreate(
        vBeepTask, "Beeper", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &beepTaskHandle
    );
    configASSERT(beepTaskHandle);
    LOG_DEBUG("beeper task created");
    xTaskNotify(beepTaskHandle, beeper_dump(&beeperStartup), eSetValueWithOverwrite);

    netif_set_status_callback(netif_default, vNetifStatusCallback);
    xTaskCreate(
        vWifiTask, "Wifi", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &wifiTaskHandle
    );
    configASSERT(&wifiTaskHandle);
    LOG_DEBUG("wifi task created");
    xTaskNotify(wifiTaskHandle, 0, eNoAction);

    xTaskCreate(
        vReadTemperatureTask, "TempTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,
        &temperatureTaskHandle
    );
    configASSERT(&temperatureTaskHandle);
    LOG_DEBUG("temp reading task created");

    xTaskCreateAffinitySet(
        vStartControlTask, "StartTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, 0x01,
        &startControlTaskHandle
    );
    configASSERT(&startControlTaskHandle);
    LOG_DEBUG("start control task created");

    xTaskCreate(
        vControlReflowTask, "CtrlTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3,
        &controlReflowTaskHandle
    );
    configASSERT(&controlReflowTaskHandle);
    LOG_DEBUG("control task created");

    LOG_DEBUG("deleting startup task");
    irq_set_enabled(IO_IRQ_BANK0, true);
    vTaskDelete(NULL);
}

void vLaunch() {
    xTaskCreateAffinitySet(
        vStartupTask, "Startup", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, 0x01,
        &startupTaskHandle
    );
    configASSERT(&startupTaskHandle);

    LOG_DEBUG("startup task created");
    LOG_DEBUG("starting scheduler");
    vTaskStartScheduler();
}

void prvSetupHardware() {
    LOG_INFO("setting up hardware");

    spi_init(spi1, 1000000);
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI1_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_6675_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_6675_MISO, GPIO_FUNC_SPI);

    gpio_init(SPI1_6675_CS);
    gpio_set_dir(SPI1_6675_CS, GPIO_OUT);
    gpio_put(SPI1_6675_CS, GPIO_OUT);

    gpio_init(TFT_DC);
    gpio_put(TFT_DC, 1);
    gpio_set_dir(TFT_DC, GPIO_OUT);

    spi_init(spi0, TFT_BAUD);
    gpio_set_function(SPI0_TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_TFT_SDA, GPIO_FUNC_SPI);

    gpio_init(SPI0_TFT_CS);
    gpio_put(SPI0_TFT_CS, 1);
    gpio_set_dir(SPI0_TFT_CS, GPIO_OUT);

    gpio_init_mask((1 << START_BTN) | (1 << STOP_BTN));
    gpio_pull_up(START_BTN);
    gpio_pull_up(STOP_BTN);
    gpio_add_raw_irq_handler(START_BTN, &startButtonCallback);
    gpio_set_irq_enabled(START_BTN, GPIO_IRQ_EDGE_FALL, true);

    gpio_set_function(BUZZER_GPIO, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 255);
    pwm_config_set_wrap(&config, 250);
    pwm_init(BUZZER_PWM_SLICE, &config, false);
    pwm_set_gpio_level(BUZZER_GPIO, 125);

    gpio_init(SSR_CONTROL_GPIO);
    gpio_set_dir(SSR_CONTROL_GPIO, GPIO_OUT);
    gpio_put(SSR_CONTROL_GPIO, 0);

    gpio_init(ZERO_CROSS_GPIO);
    gpio_set_dir(ZERO_CROSS_GPIO, GPIO_IN);
    gpio_add_raw_irq_handler(ZERO_CROSS_GPIO, &vZeroCrossCallback);
}

int main(void) {
    stdio_init_all();
    LOG_INFO("starting %s version %s", PROJECT_NAME, PROJECT_VERSION);

    prvSetupHardware();
    vLaunch();
    while (1) continue;
}
