#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <stdio.h>

#include "board_ST7735.h"
#include "logging.h"

void init_board(__unused GDisplay *g) {
    LOG_DEBUG("init tft gpio");
    gpio_init(TFT_DC);
    gpio_put(TFT_DC, 1);
    gpio_set_dir(TFT_DC, GPIO_OUT);

    LOG_DEBUG("init spi0");
    spi_init(spi0, TFT_BAUD);
    gpio_set_function(TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_SDA, GPIO_FUNC_SPI);
    gpio_init(TFT_CS);
    gpio_put(TFT_CS, 1);
    gpio_set_dir(TFT_CS, GPIO_OUT);
}

void post_init_board(__unused GDisplay *g) {}

void set_backlight(__unused GDisplay *g, gU16 __unused value) {}

// No hardware set. Software reset used instead.
void setpin_reset(__unused GDisplay *g, gU8 value) {}

void acquire_bus(__unused GDisplay *g) { gpio_put(TFT_CS, 0); }

void release_bus(__unused GDisplay *g) { gpio_put(TFT_CS, 1); }

void write_cmd(__unused GDisplay *g, gU8 cmd) {
    gpio_put(TFT_DC, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(TFT_DC, 1);
}

void write_data(__unused GDisplay *g, gU16 data) {
    uint8_t byte = data >> 8;
    gpio_put(TFT_DC, 1);
    spi_write_blocking(spi0, &byte, 1);

    byte = data;
    spi_write_blocking(spi0, &byte, 1);
}

void write_data_byte(__unused GDisplay *g, gU8 data) {
    gpio_put(TFT_DC, 1);
    spi_write_blocking(spi0, &data, 1);
}
