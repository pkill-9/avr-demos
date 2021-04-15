#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


/**
 *  ARDUINO BLINK EXAMPLE
 *
 *  Switches an LED on and off, once per second (roughly).
 *
 *  This demonstrates a couple of improvements on the Arduino IDE version:
 *  -- Use of the hardware timer & interrupt means we can avoid busy-waiting,
 *      which reduces power consumption, and would also allow the CPU to perform other
 *      tasks.
 *  -- Use of the AVR built in sleep mode minimises power use when there's
 *      nothing for the CPU to do.
 *  -- Compiled binary takes up minimal flash memory; about 260 bytes compared to
 *      1050 bytes for the Arduino blink sketch.
 */
int main (void) {
    // Set port B pin 5 to output mode (this is the same pin as Arduino D13)
    // also, set it to HIGH to start with.
    DDRB |= 0x20;
    PORTB = 0x20;

    // setup timer 1 to use the /256 prescaler, TCCR1B register bits: x x x x x 1 0 0
    // By using the /256 prescaler, there are 62,500 ticks per second (16 million / 256).
    // One interrupt per 2^16 ticks, or approx 1.05 seconds.
    TCCR1B = (TCCR1B & 0xF8) | 0x04;

    // enable the timer interrupt
    TIMSK1 |= 0x01;

    // now enter an infinite loop of sleep, wake due to clock interrupt,
    // and go back to sleep.
    while (1) {
        sei();
        sleep_mode ();
    }

    return 0;
}

/**
 *  This ISR is triggered on every clock overflow interrupt (when the 16 bit
 *  clock count register increments from 0xFFFF to 0x0000).
 *
 *  Action performed is to flip the state of the LED pin; LOW if it was HIGH,
 *  HIGH if it was LOW.
 */
ISR(TIMER1_OVF_vect) {
    PORTB = ~PORTB & 0x20;
}

// vim: ts=4 sw=4 et
