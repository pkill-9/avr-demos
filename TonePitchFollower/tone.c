#include <avr/io.h>
#include <avr/interrupt.h>

#include "tone.h"

/********************************************************************/

static uint8_t active_channels;

/********************************************************************/

/**
 *  Prepare the timer 0 hardware to generate tone output. This involves setting
 *  the timer to CTC mode (Clear on Timer Compare match).
 */
    void
tone_init (channel)
    uint8_t channel;
{
    // non-PWM mode, so we need to set the COM0x1:0 to 01, to toggle OC0A/OC0B
    // respectively on compare match. Also, the waveform mode bits are set to
    // 0x02, for CTC mode.
    TCCR0A |= (0x01 << (channel == CHANNEL_A? 6 : 4)) | 0x02;

    // Set the prescaler to /1024. If the compare register is set to 1, the
    // output frequency will be 15.625kHz (close to the upper limit of human
    // hearing). If the compare register is set to 0xFF, the frequency will be
    // approx 61 Hz, which is well above the lower limit of hearing.
    TCCR0B |= 0x05;

    // initialise the compare register to zero, and set the channel's pin mode
    // to output.
    switch (channel)
    {
    case CHANNEL_A:
        OCR0A = 0;
        DDRD |= 0x40;
        break;

    case CHANNEL_B:
        OCR0B = 0;
        DDRD |= 0x20;
        break;
    }

    active_channels |= channel;
}

/********************************************************************/

/**
 *  Set the frequency being produced on the specified channel.
 *
 *  Note, higher value corresponds to lower frequency:
 *      0xFF is the lowest available frequency (approx 61 Hz)
 *      0x01 is the highest frequency (approx 15.625 kHz)
 */
    void
set_frequency (channel, frequency_level)
    uint8_t channel;
    uint8_t frequency_level;
{
    switch (channel)
    {
    case CHANNEL_A:
        OCR0A = frequency_level;
        break;

    case CHANNEL_B:
        OCR0B = frequency_level;
        break;
    }
}

/********************************************************************/

/**
 *  Disable tone generation on the specified channel. If both channels are
 *  disabled, this function will disable the timer as well.
 */
    void
no_tone (channel)
    uint8_t channel;
{
    // unset the compare output mode bits for the respective channel
    TCCR0A &= ~(0x03 << (channel == CHANNEL_A? 6 : 4));
    active_channels &= ~channel;

    // disable the timer if both channels are disabled.
    if (active_channels == 0)
    {
        // set the waveform mode bits to 0
        TCCR0A &= ~0x03;

        // set the prescaler select bits to 0
        TCCR0B &= ~0x07;
    }
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
