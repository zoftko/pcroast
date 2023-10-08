#ifndef PCROAST_OS_TASKS_H
#define PCROAST_OS_TASKS_H

/**
 * Collection of all tasks, timers or callbacks that run in the application.
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
