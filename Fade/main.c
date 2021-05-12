#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "pwm.h"

/********************************************************************/

static volatile int led_value;
static volatile int fade_amount;

/********************************************************************/

/**
 *  ARDUINO FADE EXAMPLE, TRANSLATED TO C
 *
 *  Demonstrates using PWM to vary the brightness of an LED.
 *
 *  The circuit is very simple, consisting of an LED and current limiting
 *  resistor (220 Ohm) in series. This circuit is connected between pin 12 
 *  (OC0A) of the ATmega 328P chip and ground.
 */
    int
main (void)
{
    pwm_init (CHANNEL_A);
    pwm_update_value (CHANNEL_A, 0);
    led_value = 0x00;
    fade_amount = 1;

    // Set the 8 bit timer 2. The timer overflow interrupts will be used to
    // update the LED value
    TCCR2B = (TCCR2B & 0xF8) | 0x07;
    TIMSK2 |= 0x01;

    // enter idle sleep.
    while (1)
    {
        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

/**
 *  Timer 2 overflow ISR.
 *
 *  Action to perform is to increment (or decrement) the led value, then update
 *  the value in the timer's PWM register. If the led value is at either min or
 *  max value, reverse the direction, ie switch from increment to decrement or
 *  vice versa.
 */
ISR (TIMER2_OVF_vect)
{
    led_value += fade_amount;
    pwm_update_value (CHANNEL_A, led_value);

    if (led_value <= 0x00 || led_value >= 0xFF)
        fade_amount *= -1;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
