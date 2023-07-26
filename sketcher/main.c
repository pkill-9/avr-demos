/**
 *  SKETCHER DEMO
 *
 *  The hardware consists of a graphical LCD panel, and two rotary encoders.
 *  Turning the encoders will draw a line on the panel, with each dial
 *  controlling a set direction (x and y axes).
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stddef.h>

#include "vectors.h"
#include "uart.h"
#include "lcd.h"


static volatile int x_change = 0, y_change = 0;

/********************************************************************/

    int
main (void)
{
    vector_t cursor, origin;
    uint8_t x_rotary, y_rotary;

    uart_init (9600);

    lcd_init ();
    lcd_fill_colour (COLOUR_BLACK);

    cursor.x = 120;
    cursor.y = 160;
    origin.x = 0;
    origin.y = 0;

    DDRD &= ~0xC0;
    PORTD |= 0xC0;
    DDRB &= ~0x06;
    PORTB |= 0x06;

    PCICR |= (_BV (PCINT2) | _BV (PCINT0));
    PCMSK2 |= _BV (PCINT23);
    PCMSK0 |= _BV (PCINT1);

    for (;;)
    {
        if (x_change == 0 && y_change == 0)
        {
            sei ();
            sleep_mode ();
            continue;
        }

        if (x_change)
        {
            // delay for debouncing
            _delay_ms (5);
            x_change = 0;

            x_rotary = PIND;

            // check if it's turned clockwise or counter clockwise by XORing
            // the two channels.
            if ((x_rotary & 0x80) != (x_rotary & 0x40) << 1)
            {
                // counter clockwise
                cursor.x --;
                cursor.y --;
            }
            else
            {
                // clockwise
                cursor.x ++;
                cursor.y ++;
            }
        }

        if (y_change)
        {
            _delay_ms (5);
            y_change = 0;

            y_rotary = PINB;

            // check if it's turned clockwise or counter clockwise by XORing
            // the two channels.
            if ((y_rotary & 0x04) != (y_rotary & 0x02) << 1)
            {
                // counter clockwise
                cursor.x ++;
                cursor.y --;
            }
            else
            {
                // clockwise
                cursor.x --;
                cursor.y ++;
            }
        }

        transmit_string ("x: ");
        transmit_int (cursor.x);
        transmit_string ("; y: ");
        transmit_int (cursor.y);
        transmit_string ("\r\n");

        //write_pixel (&cursor, COLOUR_CYAN);
        write_line (&origin, &cursor, COLOUR_CYAN);
    }

    return 0;
}

/********************************************************************/

ISR (PCINT0_vect)
{
    y_change = 1;
}

/********************************************************************/

ISR (PCINT2_vect)
{
    x_change = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
