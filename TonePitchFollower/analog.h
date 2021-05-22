/**
 *  analog.h
 *
 *  Functions to read signals from analog input pins, via the built in ADC.
 */

#ifndef _ANALOG_H
#define _ANALOG_H

void analog_init (uint8_t channels_mask);
unsigned int analog_read (unsigned int channel);

#endif // _ANALOG_H

// vim: ts=4 sw=4 et
