/**
 *  Logic to operate a graphical LCD display
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>

#include "lcd.h"

/********************************************************************/

#define SWRESET             0x01
#define SLPOUT              0x11
#define COLMOD              0x3A
#define MADCTL              0x36
#define CASET               0x2A
#define RASET               0x2B
#define RAMWR               0x2C
#define INVON               0x21
#define NORON               0x13
#define DISPON              0x29
#define CMD_DELAY           0x80

#define DCX_CMD                 0
#define DCX_DATA                1

#define SPI_QUEUE_LENGTH        64


/********************************************************************/

static void display_init (const uint8_t *cmd_list);
static void send_command (uint8_t cmd, const uint8_t *params, uint8_t num_params);
static void spi_enqueue (uint8_t message, unsigned int dcx_pin);


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
 *  Send the display initialisation commands over the SPI. Note that this
 *  code is borrowed from the Adafruit ST7789 library by Limor Fried/Ladyada.
 */
    static void
display_init (cmd_list)
    const uint8_t *cmd_list;
{
    uint8_t command, num_args, delay_ms;

    for (uint8_t num_commands = *(cmd_list ++); num_commands > 0; num_commands --)
    {
        command = *(cmd_list ++);
        num_args = *(cmd_list ++);
        delay_ms = num_args & CMD_DELAY;   // check if the flag is set to indicate a delay
        num_args &= ~CMD_DELAY;
        send_command (command, cmd_list, num_args);
        cmd_list += num_args;

        if (delay_ms != 0)
        {
            delay_ms = *(cmd_list ++);
            _delay_ms (150);
        }
    }
}

/********************************************************************/

/**
 *  Send a command followed by zero or more parameter bytes over the SPI.
 */
    static void
send_command (cmd, params, num_params)
    uint8_t cmd;
    const uint8_t *params;
    uint8_t num_params;
{
    // send the command first
    spi_enqueue (cmd, DCX_CMD);

    // send the parameters
    for (; num_params > 0; num_params --)
        spi_enqueue (*(params ++), DCX_DATA);
}

/********************************************************************/

/**
 *  Fill the entire panel with a given colour, erasing any other graphics
 *  previously on the display.
 */
    void
lcd_fill_colour (colour)
    uint16_t colour;
{
    spi_enqueue (CASET, DCX_CMD);
    spi_enqueue (0x00, DCX_DATA);
    spi_enqueue (0x00, DCX_DATA);
    spi_enqueue (0, DCX_DATA);
    spi_enqueue (SCREEN_COLUMNS - 1, DCX_DATA);

    spi_enqueue (RASET, DCX_CMD);
    spi_enqueue (0x00, DCX_DATA);
    spi_enqueue (0x00, DCX_DATA);
    spi_enqueue ((SCREEN_ROWS - 1) >> 8, DCX_DATA);
    spi_enqueue ((SCREEN_ROWS - 1) & 0xFF, DCX_DATA);

    spi_enqueue (RAMWR, DCX_CMD);

    for (int row = 0; row < SCREEN_ROWS; row ++)
    {
        for (int col = 0; col < SCREEN_COLUMNS; col ++)
        {
            spi_enqueue (colour >> 8, DCX_DATA);
            spi_enqueue (colour & 0x00FF, DCX_DATA);
        }
    }
}

/********************************************************************/

/**
 *  Accept data to be sent over the SPI bus.
 *
 *  If there's no SPI transfer in progress, this function will enable the
 *  SPI and place the message in the data register; bypassing the queue.
 *  Otherwise, this function will place the message on the tail of the
 *  queue.
 *
 *  If the queue is full, this function will wait until a queue slot is
 *  available.
 *
 *  Due to blocking on full queue, this function cannot be called with
 *  interrupts disabled (such as within an ISR).
 */
    static void
spi_enqueue (message, dcx_pin)
    uint8_t message;
    unsigned int dcx_pin;
{
    // Set the value on the DCX line. This is connected to port D
    // pin 2.
    PORTD = (dcx_pin == 1)? (PORTD | 0x04) : (PORTD & 0xFB);

    // Pull the CS line LOW
    PORTD &= ~0x08;

    // no transfer in progress.
    SPCR |= (_BV (SPE) |  _BV (MSTR));
    SPDR = message;

    // wait until the SPI transfer is complete
    while ((SPSR & _BV (SPIF)) == 0)
        ;

    PORTD |= 0x08;
    SPCR &= ~_BV (SPE);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
