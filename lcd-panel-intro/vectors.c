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
    uint8_t temp = v->x;
    v->x = v->y;
    v->y = temp;
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
    uint8_t temp;

    temp = a->x;
    a->x = b->x;
    b->x = temp;

    temp = a->y;
    a->y = b->y;
    b->y = temp;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
