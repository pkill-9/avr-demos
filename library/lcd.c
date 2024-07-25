/**
 *  Common code for all graphical LCD panels.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "lcd.h"

#define CASET               0x2A
#define RASET               0x2B
#define RAMWR               0x2C
#define CMD_DELAY           0x80


static void send_command (uint8_t cmd, const uint8_t *params, uint8_t num_params);


/********************************************************************/

/**
 *  Send the display initialisation commands over the SPI. Note that this
 *  code is borrowed from the Adafruit ST7789 library by Limor Fried/Ladyada.
 */
    void
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
    write_command (cmd);

    // send the parameters
    for (; num_params > 0; num_params --)
        spi_transfer_byte (*(params ++));
}

/********************************************************************/

    void
write_command (command)
    uint8_t command;
{
    // pulling the DCX line low indicates to the controller that we're sending a
    // command.
    PORTD &= ~0x04;
    spi_transfer_byte (command);
    PORTD |= 0x04;
}

/********************************************************************/

/**
 *  Set the area of the display being used. Two points must be provided,
 *  which define a rectangular area of the display.
 */
    void
set_display_window (lower_left, upper_right)
    const vector_t *lower_left, *upper_right;
{
    // get the range of columns being used from the x values.
    // Starting column is from lower left, end column from upper right.
    write_command (CASET);
    spi_write16 (lower_left->column);
    spi_write16 (upper_right->column);

    // Same principle to get the window of rows we're using; it comes from the
    // y values in the specified points.
    write_command (RASET);
    spi_write16 (lower_left->row);
    spi_write16 (upper_right->row);

    write_command (RAMWR);
}

/********************************************************************/

/**
 *  Test if a point is within the screen area.
 */
    bool
is_within_screen (point)
    const vector_t *point;
{
    // Note: vector_t structure uses unsigned integers, so the row and column values
    // cannot be less than zero.
    //
    if (point->row > screen_rows || point->column > screen_columns)
        return false;

    return true;
}

/********************************************************************/

/**
 *  Accept data to be sent over the SPI bus.
 */
    void
spi_transfer_byte (message)
    uint8_t message;
{
    // Pull the CS line LOW
    PORTD &= ~0x08;

    SPCR |= (_BV (SPE) |  _BV (MSTR));
    SPDR = message;

    // wait until the SPI transfer is complete
    while ((SPSR & _BV (SPIF)) == 0)
        ;

    PORTD |= 0x08;
    SPCR &= ~_BV (SPE);
}

/********************************************************************/

    void
spi_write32 (data)
    uint32_t data;
{
    spi_transfer_byte (data >> 24);
    spi_transfer_byte (data >> 16);
    spi_transfer_byte (data >> 8);
    spi_transfer_byte (data);
}

/********************************************************************/

    void
spi_write16 (data)
    uint16_t data;
{
    spi_transfer_byte (data >> 8);
    spi_transfer_byte (data);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
