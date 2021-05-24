/**
 *  tone.h
 *
 *  Functions to generate square wave signals of variable frequency on 
 *  OCR0A/OCR0B pins.
 */

#ifndef _TONE_H
#define _TONE_H

#define CHANNEL_A       0x01
#define CHANNEL_B       0x02

void tone_init (uint8_t channel);
void set_frequency (uint8_t channel, uint8_t frequency_level);
void no_tone (uint8_t channel);

#endif // _TONE_H

/** vim: set ts=4 sw=4 et : */
