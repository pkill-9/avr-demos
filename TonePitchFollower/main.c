#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "analog.h"
#include "tone.h"

/********************************************************************/

static volatile uint8_t refresh_reading;

/********************************************************************/

/**
 *  ARDUINO TONE PITCH FOLLOWER DEMO IN C
 *
 *  This code reads an analog signal from A0, and produces a square wave with
 *  a frequency proportional to the analog reading on OCOA.
 *
 *  We will use a timer to take regular readings.
 *
 *  The following resources are used:
 *  Pins:
 *      A0          Pin 23
 *      OCOA        Pin 12
 *
 *  Timers:
 *      Timer 0:    used for generating the tone
 *      Timer 2:    used to trigger an ADC read
 */
    int
main (void)
{
    analog_init (0x01);
    tone_init (CHANNEL_A);
    refresh_reading = 0;

    // Set the prescaler bits for timer 2. 0x07 selects the /1024 prescaler,
    // which results in 16,000,000 / (1024 * 256) = 61 interrupts per second.
    TCCR2B |= 0x07;
    TIMSK2 |= 0x01;

    while (1)
    {
        sei ();
        sleep_mode ();

        if (refresh_reading)
            set_frequency (CHANNEL_A, analog_read (0));
    }

    return 0;
}

/********************************************************************/

/**
 *  Timer 2 overflow ISR.
 *
 *  Action to be performed is to set a flag so that the main loop will take an
 *  analog reading and update the tone frequency. We cannot take the analog
 *  reading from the ISR because analog_read has to enter ADC sleep mode.
 */
ISR (TIMER2_OVF_vect)
{
    refresh_reading = 1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
