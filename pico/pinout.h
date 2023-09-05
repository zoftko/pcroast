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

#define TFT_DC 21
#define TFT_BAUD 8000000

/**
 * MAX6675
 * SPI1 is used for reading data from MAX6675 sensors. No transmission is required, only
 * reception.
 */
#define SPI1_6675_SCK 10
#define SPI1_6675_CS 9
#define SPI1_6675_MISO 8
#define SPI1_6675_GPIO_MASK ((1 << SPI1_6675_SCK) | (1 << SPI1_6675_CS) | (1 << SPI1_6675_MISO))

#endif  // PCROAST_PINOUT_H
