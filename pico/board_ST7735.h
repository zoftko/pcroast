#ifndef PCROAST_BOARD_ST7735_H
#define PCROAST_BOARD_ST7735_H

#include <gfx.h>

#define TFT_CS 17
#define TFT_SCK 18
#define TFT_SDA 19

#define TFT_DC 21

#define TFT_BAUD 8000000

void init_board(GDisplay *g);
void post_init_board(GDisplay *g);
void set_backlight(GDisplay *g, gU16 value);
void setpin_reset(GDisplay *g, gU8 value);
void acquire_bus(GDisplay *g);
void release_bus(GDisplay *g);
void write_cmd(GDisplay *g, gU8 cmd);
void write_data(GDisplay *g, gU16 data);
void write_data_byte(GDisplay *g, gU8 data);

#endif  // PCROAST_BOARD_ST7735_H
