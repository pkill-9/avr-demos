#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "uart.h"
#include "analog.h"

/********************************************************************/

static volatile unsigned int reading;

/********************************************************************/

/**
 *  ARDUINO ANALOG READ SERIAL DEMO IN C
 *
 *  The hardware circuit consists of a 10k pull up resistor in series with an
 *  NTC thermistor, which is nominally 10k @ 25C (298 Kelvin). The varying
 *  resistance of the thermistor results in a varying voltage, which is wired
 *  to pin 23 (A0 on Arduino).
 *
 *  For simplified testing purposes, you may also use a simple 10k / 10k
 *  voltage divider network, which should deliver 2.5V to the analog input,
 *  resulting in analog readings of 512.
 *
 *  This implementation uses a hardware timer to take a reading of the analog
 *  input once per second, and then transmits the reading in a brief message
 *  over the UART line.
 */
    int
main (void)
{
    analog_init (0x01);
    uart_init (9600);

    // set up timer 1, as per the blink example and enable the IRQ.
    TCCR1B = (TCCR1B & 0xF8) | 0x04;
    TIMSK1 |= 0x01;

    // Enter an infinite sleep loop. Note that we will take the analog reading
    // in this loop, not the ISR, because the analog_read function will put
    // the MCU in noise reduction sleep, which probably shouldn't be done in
    // an ISR (ie, we don't want to wait for an interrupt while we're handling
    // an interrupt).
    while (1)
    {
        sei ();
        sleep_mode ();

        reading = analog_read (0);
    }

    return 0;
}

/********************************************************************/

/**
 *  This ISR is triggered on every clock overflow interrupt.
 *
 *  It will transmit the message containing the most recently obtained
 *  reading via the UART hardware. Calling the transmit functions is safe
 *  from inside an ISR, because those functions don't need to put the MCU in
 *  sleep mode.
 */
ISR (TIMER1_OVF_vect)
{
    transmit_string ("Got reading: ");
    transmit_int (reading);
    transmit_string ("\r\n");
}

/********************************************************************/

// vim: ts=4 sw=4 et
