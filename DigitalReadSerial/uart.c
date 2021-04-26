#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "uart.h"

#define BUFFER_LENGTH 32

struct buffer
{
    const char *messages [BUFFER_LENGTH];
    int data_length;
    int head_pos;
    int tail_pos;
};

struct buffer transmit_queue;

/********************************************************************/

/**
 *  Initialise the UART hardware, as per the AVR datasheet.
 *
 *  This consists of setting the baud rate, frame format, and enabling the
 *  transmitter/receiver.
 */
    void
uart_init (baud_rate)
    unsigned long baud_rate;    // baud rate, in bits/sec
{
    // disabling interrupts is required during initialisation for interrupt
    // driven UART operation.
    cli ();

    // As per the ATmega328P datasheet (section 24.4.1, USART internal clock
    // generation), the baud rate clock is derived from the system clock via
    // a prescaling down-counter. Each tick of the system clock, the baud
    // counter is decremented, and when it reaches 0 it produces a tick of
    // the baud clock.
    unsigned long baud_counter = F_CPU / (16 * baud_rate) - 1;

    // The baud rate is 12 bits, split across two 8 bit registers. We have
    // to write the high bits first, because updating the low bit register
    // triggers an immediate update of the baud rate prescaler.
    UBRR0H = (unsigned char) (baud_counter >> 8);
    UBRR0L = (unsigned char) (baud_counter);

    // USART Control Register B bits:
    // 0 0 0 0 1 0 0 0
    // - don't enable interrupts. Only UDRE is used, and it doesn't need to
    //   be enabled yet.
    // - enable the transmitter to send data, leave the receiver disabled.
    UCSR0B = 0x08;

    // The reset value for UCSR0C is set to 8 bit frames, which we will use.
    // Set it to send two stop bits.
    UCSR0C |= 0x08;

    // Initialise the head and tail positions for the tx and rx queues, and
    // make sure their lengths are zero to begin with.
    transmit_queue.head_pos = 0;
    transmit_queue.tail_pos = 0;
    transmit_queue.data_length = 0;

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
    void
transmit_byte (byte)
    char byte;
{
    // not implemented
}

/********************************************************************/

/**
 *  Adds a message to the next free slot in the transmit queue, for the USART
 *  hardware to send.
 *
 *  If the transmit queue is full, this function will return 0.
 */
    size_t
transmit_string (message)
    const char *message;        // pointer to the string to transmit
{
    // First, check if the transmit queue is full.
    if (transmit_queue.data_length == BUFFER_LENGTH)
        return 0;

    // Add the new message to the tail of the queue, and advance the tail
    // index by one place.
    transmit_queue.messages [transmit_queue.tail_pos] = message;
    transmit_queue.tail_pos ++;

    // increment the count of messages awaiting transmission.
    transmit_queue.data_length ++;

    // enable the UDRE interrupt by setting bit 5 in the UCSR0B register,
    // since it would be disabled if transmission isn't in progress.
    UCSR0B |= 0x20;

    return strlen (message);
}

/********************************************************************/

/**
 *  Receive a byte from the USART hardware.
 *
 *  If the receive queue is empty (no data yet received), this function will
 *  put the MCU into a low power mode until data becomes available in the
 *  buffer.
 */
    char
receive_byte (void)
{
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
ISR (USART_UDRE_vect)
{
    // Check if there's data available in the transmit queue.
    if (transmit_queue.data_length > 0)
    {
        // Check if we've reached the end of the string for the current message
        // being transmitted.
        //
        // Note that when we reach the null byte, we just advance the head_pos
        // to the next slot. That slot may have another message to transmit,
        // or it may be empty, depending on the data_length. After we exit the
        // ISR, the ISR will be invoked again straight away (because the UDRE
        // flag is still set), and then we can sort out if there's another
        // message to send or not.
        if (*(transmit_queue.messages [transmit_queue.head_pos]) != '\0')
        {
            // copy the next byte from the message into the USART data
            // register
            UDR0 = *(transmit_queue.messages [transmit_queue.head_pos]);

            // Advance the pointer to the next byte of the string.
            transmit_queue.messages [transmit_queue.head_pos] ++;
        }
        else
        {
            transmit_queue.head_pos ++;
            transmit_queue.head_pos %= BUFFER_LENGTH;
            transmit_queue.data_length --;
        }
    }
    else
    {
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
ISR (USART_RX_vect)
{
}

/********************************************************************/

// vim: ts=4 sw=4 et
