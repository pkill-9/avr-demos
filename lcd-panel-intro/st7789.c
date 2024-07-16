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

#define SWRESET             0x01
#define SLPOUT              0x11
#define COLMOD              0x3A
#define CASET               0x2A
#define RASET               0x2B
#define RAMWR               0x2C
#define MADCTL              0x36
#define INVON               0x21
#define NORON               0x13
#define DISPON              0x29
#define CMD_DELAY           0x80

#define DCX_CMD                 0
#define DCX_DATA                1


/********************************************************************/

/**
 *  Global variables to define the screen dimensions and number of pixels.
 */
const uint16_t screen_rows = 320;
const uint16_t screen_columns = 240;
const uint32_t screen_pixels = 76800;

/********************************************************************/


/**
 *  LCD PANEL INITIALISATION CMD SEQUENCE
 *
 *  This list of commands is borrowed from the Adafruit ST7789 Arduino library
 *  which was written by Limor Fried/Ladyada.
 */
static const uint8_t st7789_init_cmds [] = {
    9,                          // 9 commands.
    SWRESET, CMD_DELAY, 150,    // software reset, 150 ms delay
    SLPOUT, CMD_DELAY, 10,      // out of sleep mode, 10 ms delay
    COLMOD, 1 + CMD_DELAY,      // colour mode, 1 arg + delay
        0x55,                   // 16 bit colour (rgb 565)
        10,                     // 10 ms delay
    MADCTL, 1,                  // memory access ctrl
        0x00,
    CASET, 4,                   // column addr set, 4 args
        0,                      // xstart high bits
        0,                      // xstart low bits
        0,                      // xend high bits
        240,                    // xend low bits
    RASET, 4,                   // row addr set
        0,                      // ystart high bits
        0,                      // ystart low bits
        320 >> 8,               // yend high bits
        320 & 0xFF,             // yend low bits
    INVON, CMD_DELAY, 10,       // invert display
    NORON, CMD_DELAY, 10,       // normal (non-inverted) display
    DISPON, CMD_DELAY, 10       // main screen on.
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
    PORTD |= 0x08 | 0x10;

    display_init (st7789_init_cmds);
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
    for (uint32_t i = 0; i < pixel_count; i ++)
        spi_write16 (colour);
}

/** vim: set ts=4 sw=4 et : */
