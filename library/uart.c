#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <string.h>
#include <stdarg.h>

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
    struct queue_item *next;
};


// global vars.
//
// First, the transmit queue structure.
static struct queue_item transmit_queue [BUFFER_LENGTH];

static struct queue_item *head, *tail;
static struct queue_item *free_list;

// global int used as a mask to select the next digit to print.
static volatile uint16_t digit_mask;
static volatile uint16_t shift_bits;

// This string is used to map a digit to a character
static const char *digit_map = "0123456789ABCDEF";
static const char *hexadecimal_digits_map = "0123456789ABCDEF";

// variable to hold a byte received from the UART hardware, and a flag variable
// tp indicate that data was received.
static volatile char received_data;
static volatile uint8_t got_char;

/********************************************************************/

static struct queue_item *allocate_item (void);
static int string_transmit_handler (union message_data *data);
static int integer_transmit_handler (union message_data *data);
static int hexadecimal_transmit_handler (union message_data *data);
static void enqueue (struct queue_item *item);
static struct queue_item *dequeue (void);

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
    // 1 0 0 1 1 0 0 0
    // - enable the RX complete interrupt, but leave the UDRE interrupt disabled.
    // - enable the transmitter and receiver.
    UCSR0B = 0x98;

    // The reset value for UCSR0C is set to 8 bit frames, which we will use.
    // Set it to send two stop bits.
    UCSR0C |= 0x08;

    // Initialise the transmit queue.
    head = NULL;
    tail = NULL;
    free_list = NULL;

    // all of the queue items are in the free list to begin with.
    for (int i = 0; i < BUFFER_LENGTH; i ++)
    {
        transmit_queue [i].next = free_list;
        free_list = transmit_queue + i;
    }

    // set the digit mask to zero
    digit_mask = 0;

    received_data = 0;
    got_char = 0;

    // enable interrupts now that configuration is done.
    sei ();
}

/********************************************************************/

/**
 *  Formatted printing over UART serial.
 */
    int
uart_printf (const char *format, ...)
{
    va_list args;
    const char *string_arg;
    int integer_arg;

    va_start (args, format);

    // Start printing the message, which will stop at the first format.
    transmit_string (format);

    /////////////////////////////////////////////////////////////////
    // Step through the format string and find any number codes to print
    //
    for (const char *current = format; *current != '\0'; current ++)
    {
        // Skip any char that isn't a % (format specifier)
        if (*current != '%')
            continue;

        // Check if the next character is also a %, which would indicate to
        // print a literal % sign, which we leave to the printing function
        if (*(++ current) == '%')
            continue;

        // handle the format code.
        switch (*current)
        {
        case 'd':
            integer_arg = va_arg (args, int);
            transmit_int (integer_arg, DECIMAL);
            break;

        case 'x':
            integer_arg = va_arg (args, int);
            transmit_int (integer_arg, HEX);
            break;

        case 's':
            string_arg = va_arg (args, const char *);
            transmit_string (string_arg);
            break;

        default:
            // invalid or unsupported format.
            break;
        }

        transmit_string (++ current);
    }

    va_end (args);

    return 0;
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
    struct queue_item *next_item = allocate_item ();

    // if the buffer is full, return 0.
    if (next_item == NULL)
        return 0;

    // Add the message string pointer, and set the correct function to handle
    // printing it.
    next_item->data.text = message;
    next_item->transmit_function = &(string_transmit_handler);

    // enqueue the new item to the tail.
    enqueue (next_item);

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
transmit_int (value, base)
    int value;
    int base;
{
    struct queue_item *next_item;

    if (base == HEX)
        transmit_string ("0x");

    next_item = allocate_item ();

    if (next_item == NULL)
        return 0;

    // add the transmit_int message to the end of the queue.
    next_item->data.number = value;

    if (base == HEX)
    {
        next_item->transmit_function = &(hexadecimal_transmit_handler);
    }
    else
    {
        next_item->transmit_function = &(integer_transmit_handler);
    }

    enqueue (next_item);

    return sizeof (int);
}

/********************************************************************/

/**
 *  Add an item to the end of the transmit queue. If the queue is empty, the
 *  new item becomes the head and tail, otherwise it becomes the new tail
 */
    static void
enqueue (item)
    struct queue_item *item;
{
    item->next = NULL;

    if (head == NULL)
    {
        head = item;
        tail = item;

        // No transmit in progress, so enable the interrupt for USART data
        // register empty.
        UCSR0B |= _BV (UDRIE0);
    }
    else
    {
        tail->next = item;
        tail = item;
    }
}

/********************************************************************/

/**
 *  Remove an item from the head of the transmit queue.
 *
 *  If the queue becomes empty, set both head and tail pointers to null.
 *
 *  Return null if the queue is already empty.
 */
    static struct queue_item *
dequeue (void)
{
    struct queue_item *oldhead = head;

    if (head == NULL)
        return NULL;

    if (oldhead->next == NULL)
    {
        // no more items in the list
        tail = NULL;
    }

    head = head->next;

    return oldhead;
}

/********************************************************************/

/**
 *  Wait for the next character to be received via the USART hardware.
 *  NOTE: this function cannot be called from within an ISR, as it makes use
 *  of sleep mode.
 *
 *  Return value is the received character.
 */
    char
uart_getchar (void)
{
    got_char = 0;

    // Now put the MCU to sleep until we receive a char.
    while (got_char != 1)
    {
        sei ();
        sleep_mode ();
    }

    return received_data;
}

/********************************************************************/

/**
 *  Read data from USART, line oriented.
 *
 *  This function will accept bytes from the USART hardware, storing them in a
 *  buffer specified by the caller until either 1) a newline \n character is
 *  received; or 2) the buffer is filled.
 *
 *  The data in the buffer will be terminated by a null byte, and will include
 *  the newline character (if it is received).
 *
 *  Return value is the number of bytes stored in the buffer.
 */
    size_t
uart_getline (buffer, max_length)
    char *buffer;
    size_t max_length;
{
    size_t bytes_read = 0;

    // keep reading bytes until either a null byte, or the buffer is full.
    while ((*buffer = uart_getchar()) != '\r' && max_length > 1)
    {
        max_length --;
        buffer ++;
        bytes_read ++;
    }

    // place a terminating null byte after the byte just read
    *(buffer + 1) = '\0';

    return bytes_read;
}

/********************************************************************/

/**
 *  Fetch the next available slot in the transmit buffer. If the buffer is
 *  full, this function will return null.
 */
    static struct queue_item *
allocate_item (void)
{
    struct queue_item *next_item = free_list;

    // First, check if the transmit queue is full.
    if (next_item == NULL)
        return NULL;

    // Advance the free list to the next item
    free_list = free_list->next;

    return next_item;
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

    // Stop if we've reached a printf format sequence.
    if (*(data->text) == '%')
    {
        data->text ++;

        // Check that it isn't a '%%' sequence
        if (*(data->text) != '%' || *(data->text) == '\0')
            return 1;

        // if it was a literal % sign, we continue on to the regular logic
        // below to transmit the char over the uart.
    }

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
    if (digit_mask != 0)
    {
        next_digit = data->number / digit_mask;
        data->number %= digit_mask;
        digit_mask /= 10;
    }
    else
    {
        next_digit = data->number;
    }

    // convert the digit to a character, and store it in the USART data
    // register.
    UDR0 = digit_map [next_digit];

    return (digit_mask == 0? 1 : 0);
}

/********************************************************************/

/**
 *  This function is the same concept as the above function to transmit an
 *  integer, but instead of printing in base 10 form, print hexadecimal digits.
 *  It is worth having a separate function for this, because hexadecimal
 *  allows us to make a few simplifications compared to the base 10 code.
 */
    static int
hexadecimal_transmit_handler (data)
    union message_data *data;
{
    uint8_t next_digit;

    // we will use the same digit mask variable as the integer function, but
    // we will use it to select a group of 4 bits (one hex digit).
    if (digit_mask == 0)
    {
        digit_mask = 0xF000;
        shift_bits = 12;
    }

    next_digit = (data->number & digit_mask) >> shift_bits;
    digit_mask >>= 4;
    shift_bits -= 4;

    UDR0 = hexadecimal_digits_map [next_digit];

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
    if (head != NULL)
    {
        current_item = head;

        // Invoke the function pointer to print the next character of the
        // output, and check if the function indicates this item is finished.
        // The transmit_function is responsible for advancing to the next
        // char of the string, or next digit of an int.
        if (current_item->transmit_function (&(current_item->data)) == 1)
        {
            // remove from the head of the queue, and insert to the free list.
            current_item = dequeue ();
            current_item->next = free_list;
            free_list = current_item;
        }
    }
    else
    {
        // nothing to transmit, so disable the UDRE interrupt.
        UCSR0B &= ~ _BV (UDRIE0);
    }
}

/********************************************************************/

/**
 *  USART RX Complete interrupt handler.
 *
 *  This is invoked once the USART hardware has received a byte. The action
 *  performed is to read the data from the USART data register (which clears
 *  the interrupt) and store the value in a global variable.
 */
ISR (USART_RX_vect)
{
    received_data = UDR0;
    got_char = 1;
}

/********************************************************************/

// vim: ts=4 sw=4 et
