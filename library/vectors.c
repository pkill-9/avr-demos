/**
 *  Functions to work with 2D coordinate pairs.
 */

#include <stdint.h>

#include "vectors.h"

/********************************************************************/

/**
 *  Swap the x and y axes of a vector.
 */
    void
swap_axes (v)
    vector_t *v;
{
    uint16_t temp = v->row;
    v->row = v->column;
    v->column = temp;
}

/********************************************************************/

/**
 *  Swap two vectors. On return, a will contain b's components, and b will
 *  contain a's components.
 */
    void
swap_vectors (a, b)
    vector_t *a;
    vector_t *b;
{
    uint16_t temp;

    temp = a->row;
    a->row = b->row;
    b->row = temp;

    temp = a->column;
    a->column = b->column;
    b->column = temp;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
