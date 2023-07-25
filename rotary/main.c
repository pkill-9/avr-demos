/**
 *  ROTARY ENCODER DEMO
 *
 *  How do they work: Internally the rotary encoder has two switches that open
 *  and close out of phase with each other for each step that the dial turns.
 *  We can determine the direction it's turning based on which channel changes
 *  first. Detecting the changing switches is done with the usual pin change
 *  interrupts.
 *
 *  The encoder also has a third switch, if you press down on the dial it also
 *  has a push button.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stddef.h>

#include "uart.h"


static volatile int pin_changed = 0;

/********************************************************************/

/**
 *  Rotary encoder channel A is connected to PD7 and channel B is connected to
 *  PD6. We will enable the internal pull-up resistors in the M328P for those
 *  pins, and the rotary encoder will pull them to ground through its common pin.
 *
 *  PD6 => PCINT22
 *  PD7 => PCINT23
 */
    int
main (void)
{
    uint8_t pin_states, new_states;

    uart_init (9600);

    ///////////////////////////////////
    // Set port D pins 6 and 7 to output, and enable the pull-ups
    //
    DDRD &= ~0xC0;
    PORTD |= 0xC0;

    ///////////////////////////////////
    // Enable the two pin change interrupts, PCINT22 and PCINT23.
    //
    PCICR |= _BV (PCIE2);
    PCMSK2 |= (_BV (PCINT23));

    for (;;)
    {
        if (pin_changed == 0)
        {
            sei ();
            sleep_mode ();
            continue;
        }

        _delay_ms (5);
        pin_changed = 0;

        ///////////////////////////////
        // Get the states of the rotary encoder pins, and print the details
        // over the uart.
        //
        new_states = PIND;

        if (new_states == pin_states)
            continue;

        pin_states = new_states;

        if ((pin_states & 0x80) != (pin_states & 0x40) << 1)
        {
            transmit_string ("CLOCKWISE\r\n");
        }
        else
        {
            transmit_string ("COUNTER-CLOCKWISE\r\n");
        }
    }

    return 0;
}

/********************************************************************/

/**
 *  Handle the interrupts generated as the rotary encoder is turned.
 */
ISR (PCINT2_vect)
{
    pin_changed = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
