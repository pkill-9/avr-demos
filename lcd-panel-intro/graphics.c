/**
 *  Logic to operate a graphical LCD display
 */

#include <stddef.h>
#include <stdlib.h>

#include "lcd.h"
#include "graphics.h"
#include "vectors.h"
#include "utils.h"


/********************************************************************/

static void put_circle_pixels (const vector_t *center, int16_t column_offset, int16_t row_offset, uint16_t colour, char quadrants);

/********************************************************************/

/**
 *  Fill the entire panel with a given colour, erasing any other graphics
 *  previously on the display.
 */
    void
lcd_fill_colour (colour)
    uint16_t colour;
{
    vector_t origin, top;

    origin.row = 0;
    origin.column = 0;
    top.row = SCREEN_ROWS - 1;
    top.column = SCREEN_COLUMNS - 1;

    set_display_window (&origin, &top);
    write_colour (colour, SCREEN_PIXELS);
}

/********************************************************************/

/**
 *  Draw a triangle, given the 3 vertex coordinates. Not filled with solid
 *  colour.
 */
    void
draw_triangle (a, b, c, colour)
    const vector_t *a, *b, *c;
    uint16_t colour;
{
    ///////////////////////////////////
    // Draw 3 lines, a to b, b to c, and c to a.
    //
    write_line (a, b, colour);
    write_line (b, c, colour);
    write_line (c, a, colour);
}

/********************************************************************/

/**
 *  Draw a circle with the center at the specified coordinates and a specified
 *  radius.
 *
 *  This is an implementation of Bresenham's algorithm for circles.
 */
    void
draw_circle (center, radius, colour)
    const vector_t *center;
    int16_t radius;
    uint16_t colour;
{
    int16_t column = -1 * radius, row = 0, error = 2 - 2 * radius;

    do
    {
        put_circle_pixels (center, column, row, colour, 0x0F);

        radius = error;

        if (radius <= row)
            error += (++ row) * 2 + 1;

        if (radius > column || error > row)
            error += (++ column) * 2 + 1;
    }
    while (column < 0);
}

/********************************************************************/

/**
 *  Write the pixels for a circle.
 */
    static void
put_circle_pixels (center, column_offset, row_offset, colour, quadrants)
    const vector_t *center;
    int16_t column_offset, row_offset;
    uint16_t colour;
    char quadrants;
{
    vector_t cursor;

    if (quadrants & 0x01)
    {
        cursor.column = center->column - column_offset;
        cursor.row = center->row + row_offset;
        write_pixel (&cursor, colour);
    }

    if (quadrants & 0x02)
    {
        cursor.column = center->column - row_offset;
        cursor.row = center->row - column_offset;
        write_pixel (&cursor, colour);
    }

    if (quadrants & 0x04)
    {
        cursor.column = center->column + column_offset;
        cursor.row = center->row - row_offset;
        write_pixel (&cursor, colour);
    }

    if (quadrants & 0x08)
    {
        cursor.column = center->column + row_offset;
        cursor.row = center->row + column_offset;
        write_pixel (&cursor, colour);
    }
}

/********************************************************************/

/**
 *  Print a line on the LCD panel from the start coordinate to the end, with
 *  the line coloured with the specified 16 bit value (RGB-565). This function
 *  does not change the background colour of the panel. If the line crosses
 *  any other lines, this line will overwrite the other feature.
 *
 *  This function uses Bresenham's algorithm to draw a straight line on a
 *  raster graphics display. https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
 */
    void
write_line (start, end, colour)
    const vector_t *start;
    const vector_t *end;
    uint16_t colour;
{
    vector_t cursor;
    int16_t column_interval = abs (start->column - end->column);
    int8_t column_step = start->column < end->column ? 1 : -1;
    int16_t row_interval = -1 * abs (start->row - end->row);
    int8_t row_step = start->row < end->row ? 1 : -1;
    int16_t error = column_interval + row_interval;
    int16_t e2;

    cursor.row = start->row;
    cursor.column = start->column;

    for (;;)
    {
        write_pixel (&cursor, colour);

        // check if we've reached the end of the line, and terminate the loop.
        if (cursor.column == end->column && cursor.row == end->row)
            break;

        e2 = error << 1;

        if (e2 >= row_interval)
        {
            if (cursor.column == end->column)
                break;

            error += row_interval;
            cursor.column += column_step;
        }

        if (e2 <= column_interval)
        {
            if (cursor.row == end->row)
                break;

            error += column_interval;
            cursor.row += row_step;
        }
    }
}

/********************************************************************/

/**
 *  Set the pixel at the given x and y values to the given colour.
 */
    void
write_pixel (position, colour)
    const vector_t *position;
    uint16_t colour;
{
    // check that the coordinates are within the limits of the screen.
    if (position->column > SCREEN_COLUMNS || position->row > SCREEN_ROWS)
        return;

    set_display_window (position, position);
    write_colour (colour, 1);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
