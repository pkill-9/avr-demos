/**
 *  DEMO OF I2C WRITE FUNCTIONALITY
 *
 *  This program will interact with an I2C I/O port expander (MCP-23008)
 *  and make an LED blink once per second (approx).
 *
 *  Key details:
 *  - ATmega328P on a breadboard, programmed via ISP port.
 *  - MCP-23008 is on I2C address 0100000 (0x40).
 *  - LED is connected to I/O port 0 of the MCP-23008 chip.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stddef.h>

#include "i2c.h"

//
// constants for interacting with the MCP-23008 chip
//
#define GPIO_I2C_ADDRESS    0x40
#define IODIR_REGISTER      0x00
#define GPIO_REGISTER       0x09

static volatile uint8_t led_state;
static uint8_t data_buffer [3];

/********************************************************************/

    int
main (void)
{
    i2c_init ();

    // set up timer 1 to interrupt roughly once per second.
    TCCR1B = (TCCR1B & 0xF8) | 0x04;
    TIMSK1 |= 0x01;

    led_state = 0x00;

    // we need to set the IODIR register in the MCP-23008 chip to configure
    // I/O pin 0 as output. Prepare the buffer as {register_num, value}.
    data_buffer [0] = IODIR_REGISTER;
    data_buffer [1] = 0xFE; // clear bit 0 sets pin 0 as output.

    i2c_send_to (GPIO_I2C_ADDRESS, data_buffer, 2);

    while (1)
    {
        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

/**
 *  Timer ISR is invoked once per second. Action performed is to check the
 *  LED state as kept in memory, invert it, and update the port expander.
 */
ISR (TIMER1_OVF_vect)
{
    led_state = ~led_state & 0x01;

    // Send the new LED state by writing the value to the GPIO register on
    // the MCP-23008 chip
    data_buffer [0] = GPIO_REGISTER;
    data_buffer [1] = led_state;

    i2c_send_to (GPIO_I2C_ADDRESS, data_buffer, 2);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
