#include <FreeRTOS.h>
#include <gfx.h>
#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <task.h>

#include "logging.h"
#include "wifi.h"

TaskHandle_t wifiTaskHandle;
TaskHandle_t startupTaskHandle;
TaskHandle_t blinkLedTaskHandle;

static char prv_mac_address[13];
const char *mac_address = prv_mac_address;

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

void vStartupTask(__unused void *pvParameters) {
    LOG_INFO("initializing ugfx");
    gfxInit();
    gFont font = gdispOpenFont("DejaVuSans12");
    gdispDrawString(32, 80, "PCROAST", font, White);
    gdispCloseFont(font);

    LOG_INFO("initializing cyw43 module");
    if (cyw43_arch_init()) {
        LOG_ERROR("failed to initialize cyw43 module");
        while (1) continue;
    }
    cyw43_arch_enable_sta_mode();
    sprintf(
        prv_mac_address,
        "%x%x%x%x%x%x",
        cyw43_state.mac[0],
        cyw43_state.mac[1],
        cyw43_state.mac[2],
        cyw43_state.mac[3],
        cyw43_state.mac[4],
        cyw43_state.mac[5]
    );
    LOG_DEBUG("Using MAC %s as unique identifier", mac_address);

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

int main(void) {
    stdio_init_all();
    LOG_INFO("starting %s version %s", PROJECT_NAME, PROJECT_VERSION);

    vLaunch();
    while (1) continue;
}
