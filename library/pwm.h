/**
 *  pwm.h
 *
 *  Declares functions used for PWM output.
 *
 *  8 bit PWM is done using timer 0, which has two built in output compare
 *  registers. When the timer's counter increments passed (or decrements) the
 *  state of the corresponding pin on the MCU is automatically set or cleared,
 *  creating a PWM square wave.
 *
 *  Pins:
 *      Channel A:      pin 12
 *      Channel B:      pin 11
 */

#ifndef _PWM_H
#define _PWM_H

// constants to indicate which channel is being used.
#define CHANNEL_A       0x01
#define CHANNEL_B       0x02


void pwm_init (uint8_t channel);
void pwm_update_value (uint8_t channel, uint8_t value);
void pwm_end (uint8_t channel);

#endif // _PWM_H

// vim: ts=4 sw=4 et
