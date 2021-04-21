#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "uart.h"

#define BUFFER_LENGTH 32

struct buffer {
    char data [BUFFER_LENGTH];
    int data_length;
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

    // Initialise the head and tail positions for the tx and rx queues, and
    // make sure their lengths are zero to begin with.
    transmit_queue.head_pos = 0;
    transmit_queue.tail_pos = 0;
    transmit_queue.data_length = 0;
    receive_queue.head_pos = 0;
    receive_queue.tail_pos = 0;
    receive_queue.data_length = 0;

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
 *  Copies a message into the transmit queue, and begins the process of
 *  sending it via the USART hardware.
 *
 *  If there is insufficient space in the transmit buffer, then this function
 *  will transmit as many bytes as it can.
 *
 *  Return value is the number of bytes copied to the transmit queue.
 */
size_t transmit_string (const char *message) {
    size_t message_length = strlen (message);

    // check if the message length is greater than the available space in the
    // buffer.
    if (message_length > BUFFER_LENGTH - transmit_queue.data_length) {
        message_length = BUFFER_LENGTH - transmit_queue.data_length;
    }

    // iterate through the bytes to copy into the buffer. We will copy them
    // into the slot pointed to by the tail index and increment the tail index.
    // Once the tail index reaches the end of the buffer, it wraps around to
    // the start.
    for (int i = 0; i < message_length; i ++) {
        transmit_queue.data [transmit_queue.tail_pos] = message [i];
        transmit_queue.tail_pos ++;
        transmit_queue.tail_pos %= BUFFER_LENGTH;
    }

    // update the transmit queue length
    transmit_queue.data_length += message_length;

    // enable the UDRE interrupt by setting bit 5 in the UCSR0B register,
    // since it would be disabled if transmission isn't in progress.
    UCSR0B |= 0x20;

    return message_length;
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
    // Check if there's data available in the transmit queue.
    if (transmit_queue.data_length > 0) {
        // Copy the next byte from the transmit queue buffer into the USART
        // data register.
        UDR0 = transmit_queue.data [transmit_queue.head_pos];

        // Advance the position of the transmit queue head to the next byte
        // in the buffer (wrapping around if it passes the end).
        transmit_queue.head_pos ++;
        transmit_queue.head_pos %= BUFFER_LENGTH;

        // update the data length, now there's one less byte in the queue.
        transmit_queue.data_length --;
    } else {
        // nothing to transmit, so disable the UDRE interrupt.
        UCSR0B &= ~0x20;
    }
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
