/**
 *  Simple program to draw rectangles on a display screen
 */

#include "lcd.h"
#include "vectors.h"

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>

/********************************************************************/

//
// List of colours to cycle through
//
const uint16_t colours_list [] = {
    COLOUR_BLACK, COLOUR_NAVY, COLOUR_DARK_GREEN, COLOUR_DARK_CYAN,
    COLOUR_MAROON, COLOUR_PURPLE, COLOUR_OLIVE, COLOUR_LIGHT_GREY,
    COLOUR_DARK_GREY, COLOUR_BLUE, COLOUR_GREEN, COLOUR_CYAN, COLOUR_RED,
    COLOUR_MAGENTA, COLOUR_YELLOW, COLOUR_ORANGE, COLOUR_WHITE, COLOUR_PINK,
    COLOUR_SKY_BLUE
};

#define NUM_COLOURS     19

static volatile int timer_interrupt;

/********************************************************************/

    int
main (void)
{
    int current_colour = 1;

    lcd_init ();
    lcd_fill_colour (0);

    // setup timer 1 to use the /256 prescaler, TCCR1B register bits: x x x x x 1 0 0
    // By using the /256 prescaler, there are 62,500 ticks per second (16 million / 256).
    // One interrupt per 2^16 ticks, or approx 1.05 seconds.
    TCCR1B = (TCCR1B & 0xF8) | 0x04;

    // enable the timer interrupt
    TIMSK1 |= 0x01;

    timer_interrupt = 0;

    for (;;)
    {
        if (timer_interrupt == 0)
        {
            sei ();
            sleep_mode ();
            continue;
        }
        else
        {
            timer_interrupt = 0;
        }

        // cycle through the list of colours.
        if (++ current_colour >= NUM_COLOURS)
            current_colour = 1;

        lcd_fill_colour (colours_list [current_colour]);
    }

    return 0;
}

/********************************************************************/

/**
 *  Timer interrupt, invoked roughly once per second.
 */
ISR (TIMER1_OVF_vect)
{
    timer_interrupt = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
