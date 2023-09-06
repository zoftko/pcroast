#ifndef PCROAST_OS_TASKS_H
#define PCROAST_OS_TASKS_H

/**
 * Collection of all tasks, timers or callbacks that run in the application.
 */

void vReadTemperatureTask(void *pvParameters);

void vWifiTask(__unused void *pvParameters);
void vNetifStatusCallback(struct netif *netif);

#endif  // PCROAST_OS_TASKS_H
