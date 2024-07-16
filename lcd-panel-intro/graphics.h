/**
 *  Functions for drawing on a graphical display panel.
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H


void lcd_fill_colour (uint16_t colour);
void write_pixel (const vector_t *position, uint16_t colour);
void write_line (const vector_t *start, const vector_t *end, uint16_t colour);
void draw_triangle (const vector_t *a, const vector_t *b, const vector_t *c, uint16_t colour);
void draw_circle (const vector_t *center, int16_t radius, uint16_t colour);
void draw_rectangle (const vector_t *ll, const vector_t *ur, uint16_t colour);
void filled_rectangle (const vector_t *ll, const vector_t *ur, uint16_t colour);

#endif // _GRAPHICS_H

/** vim: set ts=4 sw=4 et : */
