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
#include "graphics.h"
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
static void demo_circles (void);

/********************************************************************/

    int
main (void)
{
    lcd_init ();

    lcd_fill_colour (colours_list [0]);

    while (1)
    {
        demo_lines ();
        demo_triangles ();
        demo_circles ();

        // clear the screen and start again.
        lcd_fill_colour (colours_list [0]);
    }

    return 0;
}

/********************************************************************/

/**
 *  Draw a series of concentric circles
 */
    static void
demo_circles (void)
{
    vector_t center;
    uint16_t colour = 0x00FF;

    // center is at half the maximum rows/cols
    center.row = SCREEN_ROWS >> 1;
    center.column = SCREEN_COLUMNS >> 1;

    for (int radius = 10; radius < 200; radius += 6)
        draw_circle (&center, radius, colour += 0x0700);

    lcd_fill_colour (0x0000);
}

/********************************************************************/

/**
 *  Demo triangle drawing function.
 */
    static void
demo_triangles (void)
{
    vector_t a, b, c;
    uint16_t colour = 0x00FF;

    for (int16_t column = 0; column <= SCREEN_COLUMNS; column += 12)
    {
        a.column = column;
        a.row = 0;
        b.column = 0;
        b.row = SCREEN_ROWS - column * 4 / 3;
        c.column = SCREEN_COLUMNS - column;
        c.row = SCREEN_ROWS;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < SCREEN_COLUMNS; column += 12)
    {
        a.column = SCREEN_COLUMNS;
        a.row = column * 4 / 3;
        b.column = 0;
        b.row = SCREEN_ROWS - column * 4 / 3;
        c.column = column;
        c.row = 0;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < SCREEN_COLUMNS; column += 12)
    {
        a.column = SCREEN_COLUMNS;
        a.row = column * 4 / 3;
        b.column = column;
        b.row = 0;
        c.column = SCREEN_COLUMNS - column;
        c.row = SCREEN_ROWS;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < SCREEN_COLUMNS; column += 12)
    {
        a.column = 0;
        a.row = SCREEN_ROWS - column * 4 / 3;
        b.column = SCREEN_COLUMNS;
        b.row = column * 4 / 3;
        c.column = SCREEN_COLUMNS - column;
        c.row = SCREEN_ROWS;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    // clear the screen.
    lcd_fill_colour (colours_list [0]);
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
    uint16_t colour = 0x00FF;

    start_point.column = SCREEN_COLUMNS >> 1;
    start_point.row = SCREEN_ROWS >> 1;
    end_point.column = 0;
    end_point.row = 0;

    for (end_point.row = 0; end_point.row < SCREEN_ROWS; end_point.row += 5)
        write_line (&start_point, &end_point, colour += 0x0700);

    for (end_point.column = 0; end_point.column < SCREEN_COLUMNS; end_point.column += 5)
        write_line (&start_point, &end_point, colour += 0x0700);

    for (; end_point.row > 0; end_point.row -= 5)
        write_line (&start_point, &end_point, colour += 0x0700);

    // Just in case the above loop leaves the row index as a negative number,
    // reset the row to zero.
    end_point.row = 0;

    for (; end_point.column > 0; end_point.column -= 5)
        write_line (&start_point, &end_point, colour += 0x0700);

    // clear the screen.
    lcd_fill_colour (colours_list [0]);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
