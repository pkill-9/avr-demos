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


#include "lcd.h"
#include "vectors.h"

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

/********************************************************************/

static void demo_lines (void);
static void demo_triangles (void);

/********************************************************************/

    int
main (void)
{
    lcd_init ();

    while (1)
    {
        demo_lines ();
        //demo_triangles ();
    }

    return 0;
}

/********************************************************************/

/**
 *  Demo triangle drawing function.
 */
    static void
demo_triangles (void)
{
    vector_t a, b, c;
    uint8_t current_colour = 0;

    lcd_fill_colour (colours_list [0]);

    for (int16_t i = 0; i <= SCREEN_COLUMNS; i += 24)
    {
        a.x = i;
        a.y = 0;
        b.x = 0;
        b.y = SCREEN_ROWS - i;
        c.x = SCREEN_COLUMNS - i;
        c.y = SCREEN_ROWS;
        draw_triangle (&a, &b, &c, colours_list [current_colour]);
        current_colour = (++ current_colour > NUM_COLOURS)? 1 : current_colour;
    }

    for (int16_t i = 0; i < SCREEN_COLUMNS; i += 24)
    {
        a.x = SCREEN_ROWS;
        a.y = i * 4 / 3;
        b.x = 0;
        b.y = SCREEN_COLUMNS - i * 4 / 3;
        c.x = i;
        c.y = 0;
        draw_triangle (&a, &b, &c, colours_list [current_colour]);
        current_colour = (++ current_colour > NUM_COLOURS)? 1 : current_colour;
    }

    for (int16_t i = 0; i < SCREEN_COLUMNS; i += 24)
    {
        a.x = SCREEN_ROWS;
        a.y = i * 4 / 3;
        b.x = i;
        b.y = 0;
        c.x = SCREEN_ROWS - i;
        c.y = SCREEN_COLUMNS;
        draw_triangle (&a, &b, &c, colours_list [current_colour]);
        current_colour = (++ current_colour > NUM_COLOURS)? 1 : current_colour;
    }
}

/********************************************************************/

/**
 *  Demo line drawing. Draws multiple lines from each corner of the screen to
 *  an opposite edge, using a selection of colours.
 */
    static void
demo_lines (void)
{
    vector_t start_point, end_point;
    int current_colour = 1;

    lcd_fill_colour (colours_list [0]);

    start_point.x = 0;
    start_point.y = 0;
    end_point.x = SCREEN_COLUMNS - 1;
    end_point.y = SCREEN_ROWS - 1;

    for (int start_row = 0; start_row < SCREEN_ROWS; start_row += 5)
    {
        start_point.y = start_row;
        write_line (&start_point, &end_point, colours_list [current_colour]);
    }

    // start is now at (0, MAX_ROWS), move the end to (0,0)
    start_point.y = SCREEN_ROWS - 1;
    end_point.x = 0;
    end_point.y = 0;
    current_colour = (++ current_colour < NUM_COLOURS) ? current_colour : 1;

    for (int start_column = 0; start_column < SCREEN_COLUMNS; start_column += 5)
    {
        start_point.x = start_column;
        write_line (&start_point, &end_point, colours_list [current_colour]);
    }

    // start is now at (MAX_COLUMNS, MAX_ROWS)
    start_point.x = SCREEN_COLUMNS - 1;
    start_point.y = SCREEN_ROWS - 1;
    end_point.x = 0;
    end_point.y = 0;
    current_colour = (++ current_colour < NUM_COLOURS) ? current_colour : 1;

    for (int start_row = start_point.y; start_row >= 0; start_row -= 5)
    {
        start_point.y = start_row;
        write_line (&start_point, &end_point, colours_list [current_colour]);
    }

    // start is now at (MAX_COLUMNS, 0), move the end to (MAX_COLUMNS, MAX_ROWS)
    start_point.x = SCREEN_COLUMNS - 1;
    start_point.y = 0;
    end_point.x = SCREEN_COLUMNS - 1;
    end_point.y = SCREEN_ROWS - 1;
    current_colour = (++ current_colour < NUM_COLOURS) ? current_colour : 1;

    for (int start_column = start_point.x; start_column >= 0; start_column -= 5)
    {
        start_point.x = start_column;
        write_line (&start_point, &end_point, colours_list [current_colour]);
    }

    current_colour = (++ current_colour < NUM_COLOURS) ? current_colour : 1;

    // clear the screen and start again.
    lcd_fill_colour (colours_list [0]);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
