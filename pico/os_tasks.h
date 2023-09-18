#ifndef PCROAST_OS_TASKS_H
#define PCROAST_OS_TASKS_H

/**
 * Collection of all tasks, timers or callbacks that run in the application.
 */

void vReadTemperatureTask(void *pvParameters);

void vWifiTask(void *pvParameters);
void vNetifStatusCallback(struct netif *netif);

/**
 * Callback executed on edge rises and falls. These signify the oven's AC voltage has crossed
 * the zero line. Used to set the oven's duty cycle accordingly.
 * @param gpio GPIO that triggered the callback
 * @param events List of events in said GPIO that triggered the callback.
 */
void vZeroCrossCallback(void);

#endif  // PCROAST_OS_TASKS_H
