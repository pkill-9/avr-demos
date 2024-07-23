#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "uart.h"
#include "analog.h"


/********************************************************************/

static volatile int refresh_results = 0;

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
    int value;

    TCCR1B = (TCCR1B & 0xF8) | 0x04;
    TIMSK1 |= 0x01;

    // Enter an infinite sleep loop. Note that we will take the analog reading
    // in this loop, not the ISR, because the analog_read function will put
    // the MCU in noise reduction sleep, which probably shouldn't be done in
    // an ISR (ie, we don't want to wait for an interrupt while we're handling
    // an interrupt).
    while (1)
    {
        if (refresh_results)
        {
            value = analog_read (0);
            uart_printf ("Got analog reading: %x\r\n", value);
            refresh_results = 0;
        }

        sei ();
        sleep_mode ();
    }

    return 0;
}

/********************************************************************/

ISR (TIMER1_OVF_vect)
{
    refresh_results = 1;
}

/********************************************************************/

// vim: ts=4 sw=4 et
