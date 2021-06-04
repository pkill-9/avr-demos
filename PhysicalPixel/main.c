#include <avr/io.h>

#include "uart.h"

/********************************************************************/

/**
 *  ARDUINO PHYSICAL PIXEL DEMO TRANSLATED TO C
 *
 *  This program demonstrates the capability of reading data from the UART,
 *  and then switching an LED on or off in response.
 *
 *  When the letter 'H' is received, the LED will be switched on; when 'L' is
 *  received the LED is switched off. The received character is idempotent, in
 *  case of multiple 'H' or 'L' chars in succession.
 */
    int
main (void)
{
    uart_init (9600);

    // Set port B pin 5 to output, compatible with the Arduino D13 (LED) pin
    DDRB |= 0x20;
    PORTB = 0;

    transmit_string ("Type H or L:\r\n");

    // enter an infinite loop, fetching data from the UART.
    while (1)
    {
        switch (uart_getchar ())
        {
        case 'H':
            // switch on the LED
            PORTB |= 0x20;
            transmit_string ("LED on.\r\n");
            break;

        case 'L':
            // switch off the LED
            PORTB &= ~0x20;
            transmit_string ("LED off.\r\n");
            break;

        default:
            transmit_string ("Type H or L:\r\n");
        }
    }

    return 0;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
