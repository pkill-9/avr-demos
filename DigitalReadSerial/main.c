#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "uart.h"


/********************************************************************/

/**
 *  ARDUINO DIGITAL READ SERIAL DEMO IN C
 *
 *  The hardware circuit consists of a push button and a 10k pull up
 *  resistor, connected to a digital input pin. When the button is pressed,
 *  it will pull the input pin low, when the switch is open, the input pin is
 *  pulled high through the pull up resistor.
 *
 *  On the software side, we will configure an interrupt handler on the
 *  respective pin change interrupt vector. When invoked, it will transmit
 *  a short message through the USART (TX/RX pins) to an FTDI chip, which
 *  in turn passes the message to a computer via USB, where the message can
 *  be viewed on a serial console.
 *
 *  As a debugging tool, we will also echo the button state on an LED
 *  attached to port B pin 5.
 */
int main (void) {
    // initialise the USART hardware.
    uart_init (103);

    // We have the signal from the push button going to pin 4 on the 328P,
    // which corresponds to PCINT18 / port D pin 2.
    // The pin is set to input on reset.
    // First step is to enable the pin change interrupt in the PC mask
    // register.
    PCMSK2 |= 0x04;

    // now we need to enable pin change interrupt 2 (which handles PCINT 16 
    // to 23).
    PCICR |= 0x04;

    // Set port B pin 2 to output, to display the button state.
    DDRB |= 0x20;
    PORTB = 0x00;

    // now we go into low power state.
    while (1) {
        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

/**
 *  Interrupt handler for the pin change.
 *
 *  Action taken is to check what the pin state is, and transmit a message
 *  via the USART hardware.
 */
ISR (PCINT2_vect) {
    if ((PIND & 0x04) != 0) {
        // button is pressed
        PORTB |= 0x20;
        //transmit_string ("button pressed\r\n");

        // busy-wait until the USART hardware is ready to receive a byte to
        // send.
        while ((UCSR0A & 0x20) == 0)
            ;

        // send a single char
        UDR0 = '1';
    } else {
        // button has been released.
        PORTB &= ~0x20;
        //transmit_string ("button released\r\n");

        // busy-wait until the USART hardware is ready to receive a byte to
        // send.
        while ((UCSR0A & 0x20) == 0)
            ;

        // send a single char
        UDR0 = '0';
    }
}

/********************************************************************/

// vim: ts=4 sw=4 et
