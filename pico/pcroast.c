#include <FreeRTOS.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <task.h>
#include <timers.h>

#include "graphics.h"
#include "logging.h"
#include "pinout.h"
#include "temperature.h"
#include "wifi.h"

TaskHandle_t wifiTaskHandle;
TaskHandle_t startupTaskHandle;
TaskHandle_t blinkLedTaskHandle;
TaskHandle_t temperatureTaskHandle;

TimerHandle_t temperatureTimerHandle;

void vApplicationMallocFailedHook(void) { LOG_ERROR("malloc has failed"); }

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

void vTemperatureTimerCallback(__unused TimerHandle_t handle) {
    xTaskNotify(temperatureTaskHandle, 0, eNoAction);
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

    temperatureTimerHandle = xTimerCreate(
        "TempTimer", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, vTemperatureTimerCallback
    );
    configASSERT(&temperatureTimerHandle);
    xTimerStart(temperatureTimerHandle, 0);
    LOG_DEBUG("temperature timer started");

    LOG_DEBUG("deleting startup task");
    vTaskDelete(NULL);
}

void vLaunch() {
    xTaskCreate(
        vStartupTask, "Startup", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2,
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
    LOG_INFO("spi1 setup successful");

    gpio_init(TFT_DC);
    gpio_put(TFT_DC, 1);
    gpio_set_dir(TFT_DC, GPIO_OUT);
    LOG_INFO("tft gpio setup successful");

    spi_init(spi0, TFT_BAUD);
    gpio_set_function(SPI0_TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_TFT_SDA, GPIO_FUNC_SPI);

    gpio_init(SPI0_TFT_CS);
    gpio_put(SPI0_TFT_CS, 1);
    gpio_set_dir(SPI0_TFT_CS, GPIO_OUT);
    LOG_INFO("spio0 gpio setup successful");
}

int main(void) {
    stdio_init_all();
    LOG_INFO("starting %s version %s", PROJECT_NAME, PROJECT_VERSION);

    prvSetupHardware();
    vLaunch();
    while (1) continue;
}
