/**
 *  I2C DIGITAL READ DEMO
 *
 *  This program demonstrates writing to and reading from an MCP23008 GPIO
 *  expander chip over the I2C bus.
 *
 *  The port expander chip will have an LED on one IO pin, and a push button
 *  on another pin. The push button pin will be set up as an input, and will
 *  have a pull-up resistor. When the button is pressed, it will trigger an
 *  interrupt which will be used to prompt the MCU to read the pin states
 *  and write a value to the LED pin, so that the LED is on when the button
 *  is pressed and off when the button is released.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "i2c.h"


/********************************************************************/

//
// constants for interacting with the MCP-23008 chip
//
#define MCP23008_ADDRESS        0x20
#define IODIR_REGISTER          0x00
#define GPIO_REGISTER           0x09
#define GPINTEN                 0x02
#define INTCON                  0x04
#define GPPULLUP                0x06
#define INTCAPTURE              0x08


static volatile int pin_changed;


/********************************************************************/

    int
main (void)
{
    // IODIR - pin 0 output, all the rest input
    // GPINTEN - enable interrupt on pin 1
    // GPPULLUP - enable internal pull up on pin 1.
    uint8_t mcp23008_setup [] = {IODIR_REGISTER, 0xFE, GPINTEN, 0x02, 
        GPPULLUP, 0x02};
    uint8_t pin_states;
    uint8_t buffer [2];

    pin_changed = 0;

    i2c_init ();

    i2c_send_to (MCP23008_ADDRESS, mcp23008_setup, 2);
    i2c_send_to (MCP23008_ADDRESS, &(mcp23008_setup [2]), 2);
    i2c_send_to (MCP23008_ADDRESS, &(mcp23008_setup [4]), 2);

    // enable pin change interrupt for port D pin 5.
    PCMSK2 |= 0x20;
    PCICR |= 0x04;

    while (1)
    {
        if (pin_changed)
        {
            pin_states = i2c_read_register (MCP23008_ADDRESS, INTCAPTURE);

            buffer [0] = GPIO_REGISTER;
            buffer [1] = (pin_states & 0x02)? 0x01 : 0x00;
            i2c_send_to (MCP23008_ADDRESS, buffer, 2);

            pin_changed = 0;
        }

        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

ISR (PCINT2_vect)
{
    // record that we've got an interrupt from the pin change. The main
    // loop can also be woken when there's any other interrupt (eg TWI
    // interrupt).
    //
    // Note that this interrupt handler is invoked on rising or falling edge,
    // but we're only interested in the rising edge (MCP23008 signals an
    // interrupt by bringing the interrupt line high, and de-asserts it by
    // bringing the line low).
    if ((PIND & 0x20) == 0)
        pin_changed = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
