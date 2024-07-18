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

static void circle_helper (const vector_t *center, int16_t radius, uint16_t colour, bool filled);
static void circle_pixels (const vector_t *center, int16_t column_offset, int16_t row_offset, 
  uint16_t colour, char quadrants, bool filled);

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
    top.row = screen_rows - 1;
    top.column = screen_columns - 1;

    set_display_window (&origin, &top);
    write_colour (colour, screen_pixels);
}

/********************************************************************/

/**
 *  Draw a rectangle, outline only.
 */
    void
draw_rectangle (ll, ur, colour)
    const vector_t *ll;
    const vector_t *ur;
    uint16_t colour;
{
    vector_t ul, lr;

    //////////////
    // Derive the other two corners from the upper left and lower right.
    //
    ul.row = ur->row;
    ul.column = ll->column;
    lr.row = ll->row;
    lr.column = ur->column;

    write_line (&ul, ur, colour);
    write_line (ll, &lr, colour);
    write_line (&ul, ll, colour);
    write_line (ur, &lr, colour);
}

/********************************************************************/

/**
 *  Draw a rectangle filled with solid colour.
 */
    void
filled_rectangle (ll, ur, colour)
    const vector_t *ll;
    const vector_t *ur;
    uint16_t colour;
{
    set_display_window (ll, ur);
    write_colour (colour, (uint32_t) (ur->row - ll->row + 1) * (ur->column - ll->column + 1));
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
    circle_helper (center, radius, colour, false);
}

/********************************************************************/

/**
 *  Draw a circle, filled in with solid colour.
 */
    void
fill_circle (center, radius, colour)
    const vector_t *center;
    int16_t radius;
    uint16_t colour;
{
    circle_helper (center, radius, colour, true);
}

/********************************************************************/

/**
 *  Draw a circle with the center at the specified coordinates and a specified
 *  radius.
 *
 *  This is an implementation of Bresenham's algorithm for circles.
 */
    static void
circle_helper (center, radius, colour, filled)
    const vector_t *center;
    int16_t radius;
    uint16_t colour;
    bool filled;
{
    int16_t column = -1 * radius, row = 0, error = 2 - 2 * radius;

    do
    {
        circle_pixels (center, column, row, colour, 0x0F, filled);

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
circle_pixels (center, column_offset, row_offset, colour, quadrants, filled)
    const vector_t *center;
    int16_t column_offset, row_offset;
    uint16_t colour;
    char quadrants;
    bool filled;
{
    vector_t cursor;

    if (quadrants & 0x01)
    {
        cursor.column = center->column - column_offset;
        cursor.row = center->row + row_offset;
        filled? vertical_line (cursor.column, center->row, cursor.row, colour) : write_pixel (&cursor, colour);
    }

    if (quadrants & 0x02)
    {
        cursor.column = center->column - row_offset;
        cursor.row = center->row - column_offset;
        filled? vertical_line (cursor.column, cursor.row, center->row, colour) : write_pixel (&cursor, colour);
    }

    if (quadrants & 0x04)
    {
        cursor.column = center->column + column_offset;
        cursor.row = center->row - row_offset;
        filled? vertical_line (cursor.column, cursor.row, center->row, colour) : write_pixel (&cursor, colour);
    }

    if (quadrants & 0x08)
    {
        cursor.column = center->column + row_offset;
        cursor.row = center->row + column_offset;
        filled? vertical_line (cursor.column, center->row, cursor.row, colour) : write_pixel (&cursor, colour);
    }
}

/********************************************************************/

/**
 *  Draw a vertical line on the display. This function is more efficient than the
 *  general line drawing function.
 */
    void
vertical_line (column, start_row, end_row, colour)
    uint16_t column, start_row, end_row, colour;
{
    vector_t line_start, line_end;
    int16_t length;

    // make sure length is a positive number
    length = (end_row >= start_row)? (end_row - start_row) : (start_row - end_row);

    line_start.row = (start_row <= end_row)? start_row : end_row;
    line_start.column = column;
    line_end.row = (end_row >= start_row)? end_row : start_row;
    line_end.column = column;

    set_display_window (&line_start, &line_end);
    write_colour (colour, length);
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
    if (position->column > screen_columns || position->row > screen_rows)
        return;

    set_display_window (position, position);
    write_colour (colour, 1);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
