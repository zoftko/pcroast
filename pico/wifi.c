#include "wifi.h"

#include <FreeRTOS.h>
#include <task.h>

#include "logging.h"

static char strIpAddress[16] = {0};

void vNetifStatusCallback(struct netif *netif) {
    ip4addr_ntoa_r(&netif->ip_addr, strIpAddress, 16);
    LOG_INFO("ip addr is %s", strIpAddress);
}

void vWifiTask(__unused void *pvParameters) {
    while (1) {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        LOG_INFO("connecting to %s", WIFI_SSID);
        LOG_DEBUG("using PSK %s and timeout %d ms", WIFI_PSK, WIFI_TIMEOUT);
        if (cyw43_arch_wifi_connect_timeout_ms(
                WIFI_SSID, WIFI_PSK, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT
            )) {
            LOG_WARNING("failed to connect");
        } else {
            LOG_INFO("connected");
        }
    }
}
