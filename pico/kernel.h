#ifndef PCROAST_KERNEL_H
#define PCROAST_KERNEL_H

#include <pico/cyw43_arch.h>

/**
 * Collection of all tasks, timers or callbacks that run in the application. Structs for task
 * intercommunication  are also included here.
 */

/**
 * The following three control tasks internally use the same variables.
 * In a SMP environment care must be taken to synchronize access. The easiest is to
 * ensure they only run on a certain core, this can be achieved with Free RTOS core affinity.
 */

void vStartControl();
void vStopControl();
void vControlReflowTask(void *pvParameters);

void vReadTemperatureTask(void *pvParameters);
void vInitControl();
void vControlHttpLogger(void *pvParameters);

struct HttpRequest {
    char *request_line;
    char *payload;
};

void vWifiTask(void *pvParameters);

/**
 * Perform a HTTP 1.1 request.
 * This task waits permanently to be notified for running. A 32 bit pointer to a HttpRequest
 * struct must be passed through the notification. Said HttpRequest specifies the URL, METHOD
 * and payload to use.
 * @param pvParameters
 */
void vHttpRequestTask(void *pvParameters);

void vNetifStatusCallback(struct netif *netif);

/**
 * Callback executed on edge rises and falls. These signify the oven's AC voltage has crossed
 * the zero line. Used to set the oven's duty cycle accordingly.
 * @param gpio GPIO that triggered the callback
 * @param events List of events in said GPIO that triggered the callback.
 */
void vZeroCrossCallback(void);

#endif  // PCROAST_KERNEL_H
