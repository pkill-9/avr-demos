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


#include <util/delay.h>

#include "lcd.h"
#include "graphics.h"
#include "vectors.h"
#include "utils.h"

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

static void demo_fill (void);
static void demo_lines (void);
static void demo_triangles (void);
static void demo_concentric (void);
static void demo_circles (void);
static void demo_rectangles (bool filled);
static void demo_round_rectangles (void);
static void demo_filled_round_rectangles (void);

static void select_full_display (void);
static uint16_t rgb888_to_rgb565 (uint8_t red, uint8_t green, uint8_t blue);

/********************************************************************/

    int
main (void)
{
    lcd_init ();

    lcd_fill_colour (colours_list [0]);

    while (1)
    {
        demo_fill ();
        demo_lines ();
        demo_triangles ();
        demo_concentric ();
        demo_circles ();
        demo_rectangles (false);
        demo_round_rectangles ();
        demo_filled_round_rectangles ();
        //demo_rectangles (true);
    }

    return 0;
}

/********************************************************************/

/**
 *  Draw rectangles, outline only
 */
    static void
demo_rectangles (filled)
    bool filled;        // if true, draws filled rects.
{
    vector_t ll, ur;
    uint16_t colour = 0x00FF;

    ////////////////////
    // Set the starting corners
    //
    ll.row = 0;
    ll.column = 0;
    ur.row = screen_rows;
    ur.column = screen_columns;

    while (ll.row < ur.row && ll.column < ur.column)
    {
        if (filled)
        {
            filled_rectangle (&ll, &ur, colour);
        }
        else
        {
            draw_rectangle (&ll, &ur, colour);
        }

        ll.row += 5;
        ll.column += 5;
        ur.row -= 5;
        ur.column -= 5;

        colour += 0x0700;
    }

    lcd_fill_colour (0x0000);
}

/********************************************************************/

    static void
demo_round_rectangles (void)
{
    uint16_t colour = 0xF00F;
    vector_t ll;
    vector_t ur;

    ll.row = 0;
    ll.column = 0;
    ur.row = screen_rows - 3;
    ur.column = screen_columns - 3;

    for (int i = 0; i <= 16; i ++)
    {
        draw_round_rectangle (&ll, &ur, 20, colour);
        ll.row += 5;
        ll.column += 5;
        ur.row -= 5;
        ur.column -= 5;
        colour += 0x0100;
    }

    lcd_fill_colour (0x0000);
}

/********************************************************************/

    static void
demo_filled_round_rectangles (void)
{
    uint16_t colour = 0xF00F;
    vector_t ll;
    vector_t ur;

    ll.row = 3;
    ll.column = 3;
    ur.row = screen_rows - 3;
    ur.column = screen_columns - 3;

    for (int i = 0; i <= 16; i += 2)
    {
        draw_round_rectangle (&ll, &ur, 20, colour);
        ll.row += 5;
        ll.column += 5;
        ur.row -= 5;
        ur.column -= 5;
        colour += 0x0100;
    }

    for (int i = 0; i <= 16; i += 2)
    {
        filled_round_rectangle (&ll, &ur, 20, colour);
        ll.row += 5;
        ll.column += 5;
        ur.row -= 5;
        ur.column -= 5;
        colour += 0x0500;
    }

    lcd_fill_colour (0x0000);
}

/********************************************************************/

/**
 *  Demo screen fill with solid colour
 */
    static void
demo_fill (void)
{
    vector_t pixel;
    uint16_t colour;

    select_full_display ();

    for (pixel.row = 0; pixel.row < screen_rows; pixel.row ++)
    {
        for (pixel.column = 0; pixel.column < screen_columns; pixel.column ++)
        {
            colour = rgb888_to_rgb565 (pixel.column << 3, pixel.row << 3, pixel.column * pixel.row);
            write_colour (colour, 1);
        }
    }

    lcd_fill_colour (0x0000);
}

/********************************************************************/

    static void
select_full_display (void)
{
    vector_t origin, limit;
    origin.row = 0;
    origin.column = 0;
    limit.row = screen_rows - 1;
    limit.column = screen_columns - 1;

    set_display_window (&origin, &limit);
}

/********************************************************************/

    static uint16_t
rgb888_to_rgb565 (red, green, blue)
    uint8_t red, green, blue;
{
    uint16_t colour = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
    return colour;
}

/********************************************************************/

    static void
demo_circles (void)
{
    vector_t center;
    uint8_t radius = 12;
    uint16_t colour = COLOUR_PINK;

    for (center.column = radius; center.column < screen_columns - radius; center.column += radius * 2)
    {
        for (center.row = radius; center.row < screen_rows - radius; center.row += radius * 2)
        {
            draw_circle (&center, radius, colour);

            if (center.column == center.row || center.column == center.row + radius * 2)
                fill_circle (&center, radius, colour);

            colour += 0x0700;
            _delay_ms (10);
        }
    }

    lcd_fill_colour (0x0000);
}

/********************************************************************/

/**
 *  Draw a series of concentric circles
 */
    static void
demo_concentric (void)
{
    vector_t center;
    uint16_t colour = 0x00FF;

    // center is at half the maximum rows/cols
    center.row = screen_rows >> 1;
    center.column = screen_columns >> 1;

    for (int radius = 10; radius < 290; radius += 6)
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

    for (int16_t column = 0; column <= screen_columns; column += 12)
    {
        a.column = column;
        a.row = 0;
        b.column = 0;
        b.row = screen_rows - column * (screen_rows >> 4) / (screen_columns >> 4);
        c.column = screen_columns - column;
        c.row = screen_rows;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < screen_columns; column += 12)
    {
        a.column = screen_columns;
        a.row = column * (screen_rows >> 4) / (screen_columns >> 4);
        b.column = 0;
        b.row = screen_rows - column * (screen_rows >> 4) / (screen_columns >> 4);
        c.column = column;
        c.row = 0;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < screen_columns; column += 12)
    {
        a.column = screen_columns;
        a.row = column * (screen_rows >> 4) / (screen_columns >> 4);
        b.column = column;
        b.row = 0;
        c.column = screen_columns - column;
        c.row = screen_rows;
        draw_triangle (&a, &b, &c, colour += 0x0700);
    }

    for (int16_t column = 0; column < screen_columns; column += 12)
    {
        a.column = 0;
        a.row = screen_rows - column * (screen_rows >> 4) / (screen_columns >> 4);
        b.column = screen_columns;
        b.row = column * (screen_rows >> 4) / (screen_columns >> 4);
        c.column = screen_columns - column;
        c.row = screen_rows;
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

    start_point.column = screen_columns >> 1;
    start_point.row = screen_rows >> 1;
    end_point.column = 0;
    end_point.row = 0;

    for (end_point.row = 0; end_point.row < screen_rows; end_point.row += 5)
        write_line (&start_point, &end_point, colour += 0x0700);

    for (end_point.column = 0; end_point.column < screen_columns; end_point.column += 5)
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
