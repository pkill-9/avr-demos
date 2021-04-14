#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 16000000UL
#include <util/delay.h>

static volatile uint8_t led_state = 1;


int main (void) {
    DDRB |= 0x20;
    PORTB = 0x20;

    // setup timer 1, TCCR1B register to bits: x x x x x 1 0 1
    // By using the /1024 prescaler, there are 15,625 ticks per second.
    // One interrupt per 32,768 ticks, or approx 2 seconds.
    TCCR1B = (TCCR1B & 0xF8) | 0x04;

    // enable the timer interrupt
    TIMSK1 |= 0x01;

    // now enter an infinite loop of sleep, wake due to clock interrupt,
    // and go back to sleep.
    while (1) {
        /*
        _delay_ms (1000);
        PORTB = 0x00;
        _delay_ms (1000);
        PORTB = 0x20;
        */
        sei();
        sleep_mode ();
    }

    return 0;
}

ISR(TIMER1_OVF_vect) {
    if (led_state == 1) {
        PORTB = 0x00;
        led_state = 0;
    } else {
        PORTB = 0x20;
        led_state = 1;
    }
}
