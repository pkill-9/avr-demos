#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "uart.h"

#define BUFFER_LENGTH 32

/********************************************************************/

// Each message could contain different data; either a string or an int.
union message_data
{
    const char *text;
    int number;
};

// each item in the transmit queue consists of the message data, and a
// pointer to a function to handle printing it. The function that prints the
// value will indicate if it has finished printing the data (all chars in a
// string, or all digits in an int) by returning 1 (finished) or 0 (more to
// go).
struct queue_item
{
    union message_data data;
    int (*transmit_function) (union message_data *data);
};

struct buffer
{
    struct queue_item items [BUFFER_LENGTH];
    int data_length;
    int head_pos;
    int tail_pos;
};

// global vars.
//
// First, the transmit queue structure.
static struct buffer transmit_queue;

// global int used as a mask to select the next digit to print.
static int digit_mask;

// This string is used to map a digit to a character
const char *digit_map = "0123456789ABCDEF";

/********************************************************************/

static int string_transmit_handler (union message_data *data);
static int integer_transmit_handler (union message_data *data);

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
 *  Adds a message to the next free slot in the transmit queue, for the USART
 *  hardware to send.
 *
 *  If the transmit queue is full, this function will return 0.
 */
    size_t
transmit_string (message)
    const char *message;        // pointer to the string to transmit
{
    struct queue_item *next_item;

    // First, check if the transmit queue is full.
    if (transmit_queue.data_length == BUFFER_LENGTH)
        return 0;

    // Add the new message to the tail of the queue, and advance the tail
    // index by one place.
    next_item = transmit_queue.items + transmit_queue.tail_pos;
    next_item->data.text = message;
    next_item->transmit_function = &(string_transmit_handler);
    transmit_queue.tail_pos ++;
    transmit_queue.tail_pos %= BUFFER_LENGTH;

    // increment the count of messages awaiting transmission.
    transmit_queue.data_length ++;

    // enable the UDRE interrupt by setting bit 5 in the UCSR0B register,
    // since it would be disabled if transmission isn't in progress.
    UCSR0B |= 0x20;

    return strlen (message);
}

/********************************************************************/

/**
 *  Convert an integer to a decimal string representation, and transmit the
 *  characters on the USART lines.
 */
    size_t
transmit_int (value)
    int value;
{
    return 0;
}

/********************************************************************/

/**
 *  This function is called from the UDRE ISR, and handles printing the next
 *  character of a string to the USART hardware. If we have reached the null
 *  byte at the end of the string, this function returns 1. If not, we
 *  return 0 to indicate there are more chars to print.
 *
 *  Note that we are given a pointer to the message data union, so that we
 *  can advance the string to the next character.
 */
    static int
string_transmit_handler (data)
    union message_data *data;   // pointer to the message data.
{
    // check if the current char is a null byte
    if (*(data->text) == '\0')
        return 1;

    // pass the next char to the USART hardware by writing to the UDR0
    // register and advance the string to the next char.
    UDR0 = *(data->text);
    data->text ++;

    return 0;
}

/********************************************************************/

/**
 *  This function is called from the UDRE ISR. It handles printing the next
 *  digit of the number, and updating the mask and number.
 *
 *  Return value is 1 if we have finished printing all digits.
 */
    static int
integer_transmit_handler (data)
    union message_data *data;
{
    uint8_t next_digit;

    // handle printing the - sign for a negative int.
    if (data->number < 0)
    {
        UDR0 = '-';
        data->number *= -1;
        return 0;
    }

    // the mask variable will be zero if this is the first digit being printed.
    // In that case, set it to select the left most decimal digit.
    // Note that ints are 16 bits long, range -32,768 to 32,767
    if (digit_mask == 0)
    {
        digit_mask = 10000;

        // find the most significant digit by repeatedly dividing the mask by
        // 10.
        while (data->number / digit_mask == 0)
            digit_mask /= 10;
    }

    // Get the next digit by integer division with the mask, then the
    // remaining digits still to be printed is obtained by modulo division.
    next_digit = data->number / digit_mask;
    data->number %= digit_mask;
    digit_mask /= 10;

    // convert the digit to a character, and store it in the USART data
    // register.
    UDR0 = digit_map [next_digit];

    return (digit_mask == 0? 1 : 0);
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
    struct queue_item *current_item;

    // Check if there's data available in the transmit queue.
    if (transmit_queue.data_length > 0)
    {
        current_item = transmit_queue.items + transmit_queue.head_pos;

        // Invoke the function pointer to print the next character of the
        // output, and check if the function indicates this item is finished.
        // The transmit_function is responsible for advancing to the next
        // char of the string, or next digit of an int.
        if (current_item->transmit_function (&(current_item->data)) == 1)
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

// vim: ts=4 sw=4 et
