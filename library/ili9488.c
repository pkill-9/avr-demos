/**
 *  Hardware specific code for the ST7789 graphical LCD panel controller,
 *  specifically for a 320 x 240 display.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "lcd.h"
#include "vectors.h"

/********************************************************************/


#define CMD_DELAY               0x80
#define DCX_CMD                 0
#define DCX_DATA                1


/********************************************************************/

/**
 *  Global variables to define the screen dimensions and number of pixels.
 */
const uint16_t screen_rows = 480;
const uint16_t screen_columns = 320;
const uint32_t screen_pixels = 153600;

/********************************************************************/

static const uint8_t ili9488_init_cmds [] = {
    17,
    0xF7, 4, 0xA9, 0x51, 0x2C, 0x82,
    0xC0, 2, 0x11, 0x09,
    0xC1, 1, 0x41,
    0xC5, 3, 0x00, 0x0A, 0x80,
    0xB1, 2, 0xB0, 0x11,
    0xB4, 1, 0x02,
    0xB6, 2, 0x02, 0x22,
    0xB7, 1, 0xC6,
    0xBE, 2, 0x00, 0x04,
    0xE9, 1, 0x00,
    0x36, 1, 0x08,
    0x3A, 1, 0x66,
    0xE0, 15, 0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F,
    0xE1, 15, 0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F,
    0x11, CMD_DELAY, 200,
    0x20, 0,
    0x29, CMD_DELAY, 10
};

/********************************************************************/

/**
 *  Initialise the SPI module in the ATmega328P so that we can talk to the
 *  LCD panel. Then initialise the LCD panel.
 *
 *  Note: Looking at the schematic for the DFRobot LCD panel breakout that
 *  I've got, it appears that the controller chip is only connected to be
 *  written to by the MCU, so I don't think I can read the value of any
 *  status registers. At least, not using the conventional SPI bus MOSI and
 *  MISO signals. The LCD controller only receives the MOSI from the MCU;
 *  MISO isn't connected.
 */
    void
lcd_init (void)
{
    // Set the DCX pin and CS pin to output mode.
    DDRD |= 0x04 | 0x08 | 0x10;

    // Set the pin mode on the MCU SPI MOSI and SCK to OUTPUT. Also set the
    // SS pin to OUTPUT.
    DDRB |= (0x04 | 0x08 | 0x20);

    // Set the SPI CS pin to HIGH. Once we begin a transfer we will pull it
    // low.
    PORTD |= 0x08;
    PORTD &= ~0x10;
    _delay_ms (200);
    PORTD |= 0x10;
    _delay_ms (200);

    display_init (ili9488_init_cmds);
}

/********************************************************************/

/**
 *  Write colour pixels to the display.
 */
    void
write_colour (colour, pixel_count)
    uint16_t colour;
    uint32_t pixel_count;
{
    uint8_t red, green, blue;

    // get the red channel from the 16 bit colour and convert it to a 3 byte
    // 18 bit colour.
    red = colour >> 11;
    green = (colour << 5) >> 10;
    blue = colour & 0x001F;

    red = (red << 1) | 0x01;
    blue = (blue << 1) | 0x01;

    red <<= 2;
    green <<= 2;
    blue <<= 2;

    for (uint32_t i = 0; i < pixel_count; i ++)
    {
        spi_transfer_byte (red);
        spi_transfer_byte (green);
        spi_transfer_byte (blue);
    }
}

/** vim: set ts=4 sw=4 et : */
