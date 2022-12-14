/**
 *  LCD PANEL INTRO
 *
 *  This is an AVR program that will talk to a colour (RGB) LCD panel over the
 *  SPI bus. This code is designed to work with an ATmega328P MCU and connect
 *  to a DFRobot 320x240px display (SKU: DFR0664 for reference) because that
 *  is what I have to work with. The display panel driver chip is an ST7789V.
 *
 *  This program will:
 *      - Initialise the SPI bus controller on the MCU side.
 *      - Initialise the LCD panel by sending commands to the ST7789V over the
 *        SPI bus.
 *      - Set up a timer on the MCU
 *      - At every timer alarm, change the screen fill colour (cycle through a
 *        list of colours).
 *
 */

#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "lcd.h"

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

static volatile int current_colour = 0;
static volatile int is_new_colour = 0;


/********************************************************************/

    int
main (void)
{
    lcd_init ();

    //
    // Set timer 1 to generate an interrupt every second
    //
    TCCR1B = (TCCR1B & 0xF8) | 0x04;
    TIMSK1 |= 0x01;

    while (1)
    {
        if (is_new_colour != 0)
        {
            lcd_fill_colour (colours_list [current_colour]);
            is_new_colour = 0;
        }

        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

ISR (TIMER1_OVF_vect)
{
    current_colour = (current_colour >= NUM_COLOURS - 1)? 0 : current_colour + 1;
    is_new_colour = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
