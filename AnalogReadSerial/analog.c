#include <avr/io.h>
#include <avr/interrupt.h>

#include "analog.h"

// mask for the ADC MUX selection bits in the ADMUX register
#define ADMUX_MASK 0x0F

// masks for the ADCSRA register
#define ADCSRA_AD_ENABLE            0x80
#define ADCSRA_START_CONVERSION     0x40
#define ADCSRA_IRQ_ENABLE           0x08
#define ADCSRA_PRESCALER            0x07
// note: prescaler selects the /128 prescaler, which will provide an ADC clock
// source at 125kHz, which is within the reccommended range of 50 to 200kHz.


/********************************************************************/

/**
 *  Set up specified analog input channels.
 *
 *  This function will disable the digital input hardware on pins that are
 *  being used for analog inputs, which saves power (digital input ports don't
 *  work well if the signal is halfway between 0 and 1).
 *
 *  Parameter is a bitmap of all 8 analog input channels.
 */
    void
analog_init (channels_mask)
    uint8_t channels_mask;  // bitmap of which analog channels are in use
{
    DIDR0 = channels_mask;
}

/********************************************************************/

/**
 *  Read an analog value off the specified ADC channel.
 *
 *  This function will set up the ADC hardware to begin converting, and will
 *  then place the MCU into a low power state until the conversion result is
 *  available.
 *
 *  Return value is a number between 0 and 1023, where 0 represents 0V read
 *  on the analog channel, and 1023 corresponds to AREF (typically VCC) read.
 */
    unsigned int
analog_read (channel)
    unsigned int channel;   // analog channel num; 0 to 7 for the 328P
{
    return 0;
}

/********************************************************************/

// vim: ts=4 sw=4 et
