#ifndef PCROAST_WIFI_H
#define PCROAST_WIFI_H

#include <pico/cyw43_arch.h>
#include <sys/cdefs.h>

void vNetifStatusCallback(struct netif *netif);
void vWifiTask(__unused void *pvParameters);

#endif  // PCROAST_WIFI_H
