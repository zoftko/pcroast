#ifndef PCROAST_PINOUT_H
#define PCROAST_PINOUT_H

/**
 * Pin out for the application
 * This is a collection of all pins used by the application, labeled with a self explaining name.
 * Numbers refer to the GPIO number, and no the physical pin number.
 */

/**
 * TFT LCD
 * SPI0 is used solely for interacting with TFT LCDs. An extra GPIO pin is required for specifying
 * whether data is meant as a command or as display data.
 */
#define SPI0_TFT_CS 17
#define SPI0_TFT_SCK 18
#define SPI0_TFT_SDA 19

#define TFT_DC 20
#define TFT_BAUD 16000000

/**
 * MAX6675
 * SPI1 is used for reading data from MAX6675 sensors. No transmission is required, only
 * reception.
 */
#define SPI1_6675_SCK 14
#define SPI1_6675_CS 13
#define SPI1_6675_MISO 12
#define SPI1_TX 15  // Not used but need to be defined to a known GPIO

#define STOP_BTN 22
#define START_BTN 21
#define BUZZER_GPIO 2
#define BUZZER_PWM_SLICE 1
#define BUZZER_PWM_CHANNEL 0

#endif  // PCROAST_PINOUT_H
