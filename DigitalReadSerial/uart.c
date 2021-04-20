#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define BUFFER_LENGTH 32

struct buffer {
    char data [BUFFER_LENGTH];
    int head_pos;
    int tail_pos;
};

struct buffer transmit_queue;
struct buffer receive_queue;

/********************************************************************/

/**
 *  Initialise the UART hardware, as per the AVR datasheet.
 *
 *  This consists of setting the baud rate, frame format, and enabling the
 *  transmitter/receiver.
 */
void init_uart (int baud_rate) {
    // disabling interrupts is required during initialisation for interrupt
    // driven UART operation.
    cli ();

    // The baud rate is 12 bits, split across two 8 bit registers. We have
    // to write the high bits first, because updating the low bit register
    // triggers an immediate update of the baud rate prescaler.
    UBRR0H = (unsigned char) (baud_rate >> 8);
    UBRR0L = (unsigned char) (baud_rate);

    // USART Control Register B bits:
    // 1 0 1 1 1 0 0 0
    // - enable interrupts for RX complete, and Data Register Empty
    // - enable the receiver and transmitter.
    UCSR0B = 0xB8;

    // The reset value for UCSR0C is set to 8 bit frames, which we will use.
    // No need to change other bits in that register.

    // enable interrupts now that configuration is done.
    sei ();
}

/********************************************************************/

/**
 *  Transmit a byte of data from the MCU to another device via the USART.
 *  We use interrupt driven operation, so this function will simply add the
 *  byte to a transmit queue. The Data Register Empty ISR will handle the
 *  task of passing the data to the USART hardware.
 *
 *  If there is no other data in the transmit queue, this function will 
 *  enable the UDRE interrupt.
 */
void transmit_byte (char byte) {
}

/********************************************************************/

/**
 *  Receive a byte from the USART hardware.
 *
 *  If the receive queue is empty (no data yet received), this function will
 *  put the MCU into a low power mode until data becomes available in the
 *  buffer.
 */
char receive_byte (void) {
    return '0';
}

/********************************************************************/

/**
 *  USART Data Register Empty interrupt handler.
 *
 *  This is invoked once the USART hardware is ready to receive another byte
 *  of data to transmit. The action performed will be to either load another
 *  byte from our transmit buffer into the data register, or if there is no
 *  more data to be transmitted, disable the UDRE interrupt.
 */
ISR (USART_UDRE_vect) {
}

/********************************************************************/

/**
 *  USART Receive Complete interrupt handler.
 *
 *  Invoked when the USART hardware has finished receiving a frame.
 *  Action taken is to check the status register for any error flags, and
 *  if all is good transfer the received byte from the data register to our
 *  receive buffer.
 */
ISR (USART_RX_vect) {
}

/********************************************************************/

// vim: ts=4 sw=4 et
