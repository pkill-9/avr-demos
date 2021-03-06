/**
 *  i2c.c
 *
 *  Functions for interacting with Atmel microcontroller TWI (two wire 
 *  interface) hardware. Atmel TWI is inter-operable with I2C. These 
 *  functions enable the calling code to transsfer data to and from other
 *  devices connected to the microcontroller via an I2C bus.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>

#include "i2c.h"

/********************************************************************/

#define BUFFER_LENGTH 32

// Each I2C transfer must contain a device address, and the data to send.
// For most devices, the first byte in the data packet specifies the register
// number on the device that is to be written to.
struct i2c_queue_item
{
    uint8_t device_address;
    uint8_t i2c_mode;
    const uint8_t *data;
    uint8_t length;
    struct i2c_queue_item *next;
};

// these constants are used to determine which mode to put the I2C hardware in
// for the current transmission.
#define MASTER_TRANSMITTER_MODE 0x02
#define MASTER_RECEIVER_MODE 0x04


static struct i2c_queue_item i2c_buffer [BUFFER_LENGTH];

static struct i2c_queue_item *queue_head;
static struct i2c_queue_item *queue_tail;

//
// constants for certain register bitmasks
//
// TWI control register
#define I2C_INT_FLAG    0x80
#define I2C_ENABLE      0x04
#define I2C_ENABLE_IRQ  0x01
#define I2C_ENABLE_ACK  0x40
#define I2C_START       0x20
#define I2C_STOP        0x10

#define TWI_FREQ 100000L


/********************************************************************/

static struct i2c_queue_item *allocate_queue_slot (void);
static void master_transmitter_handler (void);

/********************************************************************/

/**
 *  Prepare the ATmega328P TWI hardware for data transfers.
 */
    void
i2c_init (void)
{
    queue_head = NULL;
    queue_tail = NULL;

    // step through all the slots in the buffer and mark them as free by
    // setting the i2c_mode to zero.
    for (int i = 0; i < BUFFER_LENGTH; i ++)
        i2c_buffer [i].i2c_mode = 0x00;

    // enable internal pull-up resistors on SDA & SCL lines.
    PORTC = 0x30;

    // Set the bit rate register to the correct value for the desired I2C
    // bus frequency. This formula can be found in the Atmel datasheet.
    TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
    TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA);
}

/********************************************************************/

/**
 *  Send data to a specified destination over the I2C bus.
 *
 *  Note that sending is asynchronous; this function will place the data in
 *  a queue of pending I2C transactions which will be executed when the I2C
 *  bus has finished any other read/writes.
 *
 *  This function does not copy the data to another buffer, so the caller
 *  must take care not to modify the data in the buffer before sending is
 *  complete. This includes after this function returns; since the I2C bus
 *  speed is an order of magnitude or two slower than the CPU clock speed.
 */
    void
i2c_send_to (device_address, data, length)
    uint8_t device_address;     // 8 bit I2C device address
    const uint8_t *data;        // data to send (one or more bytes)
    unsigned int length;        // number of bytes to send
{
    // get a free slot from the buffer
    struct i2c_queue_item *buffer_slot = allocate_queue_slot ();

    // if the buffer is full, do nothing.
    if (buffer_slot == NULL)
        return;

    // store the message details.
    buffer_slot->device_address = device_address;
    buffer_slot->data = data;
    buffer_slot->length = length;
    buffer_slot->i2c_mode = MASTER_TRANSMITTER_MODE;
    buffer_slot->next = NULL;

    // If the queue is empty, the new item is the new head, and we also need
    // to instruct the hardware to send a START signal. If there are other
    // items in the queue, append the new item at the tail.
    if (queue_head == NULL)
    {
        queue_head = buffer_slot;
        queue_tail = buffer_slot;
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA) | _BV (TWINT) | _BV (TWSTA);
    }
    else
    {
        queue_tail->next = buffer_slot;
        queue_tail = buffer_slot;
    }
}

/********************************************************************/

/**
 *  Find an available slot in the I2C message buffer.
 *
 *  Used slots are identified based on the i2c_mode field being set to either
 *  master transmitter or master receiver. If the mode is set to zero, the
 *  slot is available. If the buffer is full, this function will return NULL
 */
    struct i2c_queue_item *
allocate_queue_slot (void)
{
    struct i2c_queue_item *found_slot = NULL;

    // iterate through the array and find a slot with the i2c_mode set to
    // zero.
    for (int i = 0; i < BUFFER_LENGTH; i ++)
    {
        if (i2c_buffer [i].i2c_mode == 0x00)
        {
            found_slot = &(i2c_buffer [i]);
            break;
        }
    }

    return found_slot;
}

/********************************************************************/

/**
 *  Handle events when the hardware is in master transmitter mode.
 *
 *  This function will check the value in the status register (TWSR) and
 *  take the appropriate action by loading values into the data register
 *  and/or control register
 */
    static void
master_transmitter_handler (void)
{
    uint8_t status_code = TWSR & 0xF8;

    switch (status_code)
    {
    case 0x08:
    case 0x10:
        // START or REPEAT START has been sent; load slave address + write
        // bit (LSB = 0) into TWDR.
        TWDR = queue_head->device_address << 1;
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWINT) | _BV (TWEA);
        break;

    case 0x28:
    case 0x30:
        // data has been transmitted and either ACK (0x28) or NOT ACK (0x30)
        // has been received. Move on to the next byte to be transmitted (if
        // available).
        queue_head->data ++;
        queue_head->length --;

        // if the data length is zero, move the queue head along the list.
        if (queue_head->length == 0)
        {
            queue_head->i2c_mode = 0;
            queue_head = queue_head->next;

            // if there's another item to transmit, send REPEAT START. If
            // there's no other item, send STOP.
            if (queue_head != NULL)
            {
                TWCR = I2C_INT_FLAG | I2C_START | I2C_ENABLE | I2C_ENABLE_IRQ;
                break;
            }
            else
            {
                // queue is empty, so mark tail as null too.
                queue_tail = NULL;
                TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA) | _BV (TWINT) | _BV (TWSTO);
                break;
            }
        }

        // If we reach this point, there is valid data to transmit. Fall
        // through to send the next byte.

    case 0x18:
    case 0x20:
        // slave address + write has been transmitted and ACK received. load
        // data byte into TWDR.
        // TODO: 0x20 indicates that NOT ACK was received, should this be
        // considered an error?
        TWDR = *(queue_head->data);
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWINT) | _BV (TWEA);
        break;

    case 0x38:
        // Arbitration lost. This can only happen if there is another device
        // trying to become master on the I2C bus, and is not applicable to
        // this code.
        //
        // fall through to default.

    default:
        // status code not defined in datasheet. This code should never be
        // reached.
        break;
    }
}

/********************************************************************/

/**
 *  Interrupt handler for TWI / I2C hardware. This is invoked after hardware
 *  events, as set out in the datasheet (eg sent start signal, sent data).
 */
ISR (TWI_vect)
{
    // check that the queue head is available (if not, ignore the interrupt)
    if (queue_head == NULL)
    {
        TWCR |= I2C_INT_FLAG;
        return;
    }

    // check the I2C mode of the queue head, and dispatch to the corresponding
    // function
    switch (queue_head->i2c_mode)
    {
    case MASTER_TRANSMITTER_MODE:
        master_transmitter_handler ();
        break;

    default:
        TWCR |= I2C_INT_FLAG;
    }
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
