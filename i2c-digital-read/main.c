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
#include "uart.h"


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

#define REGISTER_FILE_SIZE      11

static volatile int pin_changed;
static volatile uint8_t mcp23008_registers [REGISTER_FILE_SIZE];


/********************************************************************/

    int
main (void)
{
    uint8_t mcp23008_setup [] = {IODIR_REGISTER, 0xFE, GPINTEN, 0x00, 
        GPPULLUP, 0x02};
    uint8_t pin_states;
    uint8_t buffer [2];

    pin_changed = 0;

    i2c_init ();
    uart_init (9600);

    transmit_string ("Initialising mcp23008 chip\r\n");
    i2c_send_to (MCP23008_ADDRESS, mcp23008_setup, 2);
    i2c_send_to (MCP23008_ADDRESS, &(mcp23008_setup [2]), 2);
    i2c_send_to (MCP23008_ADDRESS, &(mcp23008_setup [4]), 2);
    transmit_string ("done mcp23008 configuration\r\n");

    // enable pin change interrupt for port D pin 6.
    //PCMSK2 |= 0x20;
    //PCICR |= 0x04;

    while (1)
    {
        //buffer [0] = 0;

        //if (tx_slots_free () > REGISTER_FILE_SIZE * 2 + 1)
        //{
        //    i2c_send_to (MCP23008_ADDRESS, buffer, 1);
        //    i2c_receive_from (MCP23008_ADDRESS, mcp23008_registers, REGISTER_FILE_SIZE);

        //    for (int i = 0; i < REGISTER_FILE_SIZE; i ++)
        //    {
        //        transmit_int (mcp23008_registers [i]);
        //        transmit_string (" ");
        //    }

        //    transmit_string ("\r\n");
        //}
        //if (pin_changed)
        //{
            //transmit_string ("reading pin states\r\n");
            pin_states = i2c_read_register (MCP23008_ADDRESS, GPIO_REGISTER);
            //if (tx_slots_free () > 2)
            //{
            //    transmit_int (pin_states);
            //    transmit_string ("\r\n");
            //}

            buffer [0] = GPIO_REGISTER;
            buffer [1] = (pin_states & 0x02)? 0x01 : 0x00;
            //transmit_string ("sending LED state\r\n");
            i2c_send_to (MCP23008_ADDRESS, buffer, 2);

            //pin_changed = 0;
        //}

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
    //if ((PIND & 0x20) == 0)
        pin_changed = 1;

    transmit_string ("got pin change\r\n");
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
