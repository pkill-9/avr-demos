/**
 *  vectors.h
 *
 *  Defines types and functions to work with 2 dimensional coordinates.
 */

#ifndef _VECTORS_H
#define _VECTORS_H

typedef struct
{
    uint16_t row, column;
}
vector_t;


void swap_axes (vector_t *v);
void swap_vectors (vector_t *a, vector_t *b);

#endif // _VECTORS_H

/** vim: set ts=4 sw=4 et : */
