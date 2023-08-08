/**
 *  Logic to operate a graphical LCD display
 */

#include <stddef.h>
#include <stdlib.h>

#include "lcd.h"
#include "vectors.h"
#include "utils.h"


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

    for (int i = 0; i < SCREEN_ROWS; i ++)
    {
        write_colour (colour, SCREEN_COLUMNS);
    }
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
 *  Print a line on the LCD panel from the start coordinate to the end, with
 *  the line coloured with the specified 16 bit value (RGB-565). This function
 *  does not change the background colour of the panel. If the line crosses
 *  any other lines, this line will overwrite the other feature.
 */
    void
write_line (start, end, colour)
    const vector_t *start;
    const vector_t *end;
    uint16_t colour;
{
    int16_t dx, dy, err, ystep;
    vector_t cursor, steep_cursor, start_pos, end_pos;

    start_pos.row = start->row;
    start_pos.column = start->column;
    end_pos.row = end->row;
    end_pos.column = end->column;

    ///////////////////////////////////
    // The idea is that we will advance the x/column value one pixel at a time, and
    // every few pixels advance the y/row value. If the line is steeper than 1:1
    // there's a problem because we would need to advance the y value multiple
    // times for each x value. We solve this by interchanging x and y values
    // if the line is steep.
    //
    bool steep = abs (end_pos.row - start_pos.row) > abs (end_pos.column - start_pos.column);

    if (steep)
    {
        swap_axes (&start_pos);
        swap_axes (&end_pos);
        //
        // Astute readers will have noted that this swap changes the line.
        // Read on and you will see that we handle steep lines differently
        // when we draw the pixels, to effectively swap the axes back again.
    }

    ///////////////////////////////////
    // We will always increment the x value, so if we're trying to draw a
    // line with the provided start and end in the wrong direction, we will
    // swap the start and end so that we can increment x to get to the end
    // point.
    if (start_pos.column > end_pos.column)
        swap_vectors (&start_pos, &end_pos);

    // keep the change in x and y handy.
    dx = end_pos.column - start_pos.column;
    dy = abs (end_pos.row - start_pos.row);

    // handle positive/negative gradient
    ystep = (start_pos.row < end_pos.row)? 1 : -1;

    for (cursor.column = start_pos.column, cursor.row = start_pos.row; cursor.column <= end_pos.column; cursor.column ++)
    {
        ///////////////////////////////
        // handle the axes swap that we did earlier for steep lines.
        if (steep)
        {
            steep_cursor.column = cursor.row;
            steep_cursor.row = cursor.column;
            write_pixel (&steep_cursor, colour);
        }
        else
        {
            write_pixel (&cursor, colour);
        }

        err -= dy;

        if (err < 0)
        {
            cursor.row += ystep;
            err += dx;
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
    set_display_window (position, position);
    write_colour (colour, 1);
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
