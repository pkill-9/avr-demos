#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

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

// global variable to store the conversion results, and indicate if the results
// are ready
static volatile unsigned int conversion_results;

// when the conversion is done, the ISR will set the most significant bit of
// the above variable (after all, the conversion result itself only uses 10
// bits). This mask isolates our results_ready flag bit.
#define RESULTS_READY_MASK          0x8000


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

    // Configure the ADCSRA register. Write the prescaler select bits, and
    // also enable the IRQ.
    ADCSRA |= (ADCSRA_AD_ENABLE | ADCSRA_IRQ_ENABLE | ADCSRA_PRESCALER);
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
    // Set the ADMUX register to indicate which channel we're reading from
    ADMUX &= ~ADMUX_MASK;
    ADMUX |= channel & ADMUX_MASK;

    conversion_results = 0;

    // Start conversion by setting the ADC start bit in ADCSRA
    ADCSRA |= ADCSRA_START_CONVERSION;

    // Now enter ADC noise reduction sleep. When we wake up, check if the
    // conversion results are available, and if they're not (since any other
    // interrupt will wake the MCU from sleep) go back to sleep.
    while (conversion_results == 0)
    {
        set_sleep_mode (0x01);
        sleep_mode ();
    }

    return conversion_results & ~RESULTS_READY_MASK;
}

/********************************************************************/

/**
 *  ADC complete interrupt handler.
 *
 *  Action to perform (in single shot mode) is to fetch the conversion results
 *  and place them in a variable. The analog_read function will then return
 *  that value back to it's caller.
 */
ISR (ADC_vect)
{
    conversion_results |= ADCL;
    conversion_results |= ADCH << 8;
    conversion_results |= RESULTS_READY_MASK;
}

/********************************************************************/

// vim: ts=4 sw=4 et
