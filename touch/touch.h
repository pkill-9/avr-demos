/**
 *  touch.h
 *
 *  Declares functions used for interacting with a CAP1188 touch sensor module.
 */

#ifndef _TOUCH_H
#define _TOUCH_H

void touch_init (void);
void install_handler (void (*handler) (uint8_t), uint8_t channel);

#endif // _TOUCH_H

/** vim: set ts=4 sw=4 et : */
