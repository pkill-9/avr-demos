/**
 *  lcd.h
 *
 *  Defines functions and constants to interact with a graphical LCD panel.
 */

#ifndef _LCD_H
#define _LCD_H

#include <stdint.h>

#include "vectors.h"

//
// constants for 16 bit (RGB 565) colours
//
#define COLOUR_BLACK            0x0000
#define COLOUR_NAVY             0x000F
#define COLOUR_DARK_GREEN       0x03E0
#define COLOUR_DARK_CYAN        0x03EF
#define COLOUR_MAROON           0x7800
#define COLOUR_PURPLE           0x780F
#define COLOUR_OLIVE            0x7BE0
#define COLOUR_LIGHT_GREY       0xC618
#define COLOUR_DARK_GREY        0x7BEF
#define COLOUR_BLUE             0x001F
#define COLOUR_GREEN            0x07E0
#define COLOUR_CYAN             0x07FF
#define COLOUR_RED              0xF800
#define COLOUR_MAGENTA          0xF81F
#define COLOUR_YELLOW           0xFFE0
#define COLOUR_ORANGE           0xFD20
#define COLOUR_WHITE            0xFFFF
#define COLOUR_PINK             0xFE19
#define COLOUR_SKY_BLUE         0x867D

#define SCREEN_ROWS             320
#define SCREEN_COLUMNS          240
#define SCREEN_PIXELS           76800


void lcd_init (void);
void set_display_window (const vector_t *lower_left, const vector_t *upper_right);
void write_colour (uint16_t colour, uint32_t pixel_count);


#endif // _LCD_H

/** vim: set ts=4 sw=4 et: */
