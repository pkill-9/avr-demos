#include <avr/io.h>

#include "pwm.h"

/********************************************************************/

#define COMPARE_OUTPUT_MODE     0x03    // set counting up, clear counting down.
#define WAVEFORM_MODE           0x01    // phase correct pwm
#define PRESCALER_SELECT        0x05    // /1024 prescaler, ~61 Hz PWM.
#define PRESCALER_MASK          0x07

/********************************************************************/

uint8_t active_channels;

/********************************************************************/

/**
 *  Sets up the control registers for timer 0 for PWM on the specified
 *  channel.
 */
    void
pwm_init (channel)
    uint8_t channel;
{
    // Shift the compare output mode bits to the right place in the register
    // for either channel A or channel B.
    TCCR0A |= (COMPARE_OUTPUT_MODE << (channel == CHANNEL_A)? 6 : 4) | WAVEFORM_MODE;

    // set the prescaler
    TCCR0B |= PRESCALER_SELECT;

    // set the compare register to zero for the respective channel
    switch (channel)
    {
    case CHANNEL_A:
        OCR0A = 0;
        break;

    case CHANNEL_B:
        OCR0B = 0;
        break;
    }

    // record which channel is activated
    active_channels |= channel;
}

/********************************************************************/

/**
 *  Updates the value in the compare register, for the specified channel.
 */
    void
pwm_update_value (channel, value)
    uint8_t channel;
    uint8_t value;
{
    switch (channel)
    {
    case CHANNEL_A:
        OCR0A = value;
        break;

    case CHANNEL_B:
        OCR0B = value;
        break;
    }
}

/********************************************************************/

/**
 *  Stops PWM output on the specified channel. If both channels are switched
 *  off, this function will also stop the timer.
 */
    void
pwm_end (channel)
    uint8_t channel;
{
    TCCR0A &= ~(COMPARE_OUTPUT_MODE << (channel == CHANNEL_A)? 6 : 4);
    active_channels &= ~channel;

    // if both channels are off, disable the timer.
    if (active_channels == 0)
        TCCR0B &= ~PRESCALER_MASK;
}

/********************************************************************/

// vim: ts=4 sw=4 et
