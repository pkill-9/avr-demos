/**
 *  i2c.c
 *
 *  Functions for interacting with Atmel microcontroller TWI (two wire 
 *  interface) hardware. Atmel TWI is inter-operable with I2C. These 
 *  functions enable the calling code to transfer data to and from other
 *  devices connected to the microcontroller via an I2C bus.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stddef.h>

#include "i2c.h"
#include "uart.h"

/********************************************************************/

#define BUFFER_LENGTH 32

// Each I2C transfer must contain a device address, and the data to send.
// For most devices, the first byte in the data packet specifies the register
// number on the device that is to be written to.
struct i2c_queue_item
{
    uint8_t device_address;
    uint8_t i2c_mode;
    uint8_t *data;
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


#define TWI_FREQ 100000L


/********************************************************************/

static struct i2c_queue_item *allocate_queue_slot (void);
static void master_transmitter_handler (uint8_t status_code);
static void master_receiver_handler (uint8_t status_code);
static void enqueue (struct i2c_queue_item *item);
static void dequeue (void);

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
    //transmit_string ("sending data to I2C address: ");
    //transmit_int (device_address);
    //transmit_string ("; length: ");
    //transmit_int (length);
    //transmit_string ("\r\n");

    buffer_slot->device_address = device_address;
    buffer_slot->data = (uint8_t *) data;
    buffer_slot->length = length;
    buffer_slot->i2c_mode = MASTER_TRANSMITTER_MODE;
    buffer_slot->next = NULL;

    enqueue (buffer_slot);
}

/********************************************************************/

/**
 *  Read the value from a single specified register from a specified device
 *  address on the I2C bus. This function will do a write operation to send
 *  the register address to the device, followed by a read operation to fetch
 *  the value.
 *
 *  This function will block until the data has been fetched from the
 *  register, note that I2C isn't particularly fast so we will sleep for
 *  posibly many CPU cycles.
 */
    uint8_t
i2c_read_register (device_address, device_register)
    uint8_t device_address;
    uint8_t device_register;
{
    uint8_t register_contents;

    // Set the remote device's register pointer to the register that we need
    // to read from. (this call doesn't block)
    i2c_send_to (device_address, &device_register, 1);

    // Reading a single register works the same as reading multiple registers,
    // except the count is 1. This call waits until the data is received.
    i2c_receive_from (device_address, &register_contents, 1);

    return register_contents;
}

/********************************************************************/

/**
 *  Fetch the values from many registers in sequence.
 *
 *  This function will read the specified length of bytess from the specified
 *  device, and the device should automatically advance it's internal
 *  register pointer to the next register after each byte is returned.
 *
 *  This function will put the MCU in a sleep mode until all of the bytes
 *  have been received.
 */
    void
i2c_receive_from (device_address, buffer, length)
    uint8_t device_address;
    uint8_t *buffer;
    unsigned int length;
{
    // get a free slot from the buffer
    struct i2c_queue_item *buffer_slot = allocate_queue_slot ();

    // if the buffer is full, do nothing.
    if (buffer_slot == NULL)
        return;

    // store the message details.
    buffer_slot->device_address = device_address;
    buffer_slot->data = buffer;
    buffer_slot->length = length;
    buffer_slot->i2c_mode = MASTER_RECEIVER_MODE;
    buffer_slot->next = NULL;

    enqueue (buffer_slot);

    // Sleep until all bytes are received.
    while (buffer_slot->i2c_mode != 0)
    {
        sei ();
        sleep_mode ();
    }
}

/********************************************************************/

/**
 *  Append the given queue structure as the new tail of the queue. If the
 *  queue is empty, the item also becomes the queue head.
 *
 *  If the queue is empty, this function will also set the control register
 *  to send the START signal.
 */
    static void
enqueue (item)
    struct i2c_queue_item *item;
{
    if (queue_tail == NULL)
    {
        queue_head = item;
        queue_tail = item;
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA) | _BV (TWINT) | _BV (TWSTA);
    }
    else
    {
        queue_tail->next = item;
        queue_tail = item;
    }
}

/********************************************************************/

/**
 *  Remove the item at the head of the queue, and point the head to the next
 *  item if available.
 *
 *  If the head is the last item in the queue, both the head and tail will
 *  be set to NULL, and this function will also set the control register to
 *  send a STOP signal.
 */
    static void
dequeue (void)
{
    // de-allocate the item at the head of the queue, by setting the i2c_mode
    // field to 0.
    queue_head->i2c_mode = 0;
    queue_head = queue_head->next;

    // if there's another item to transmit, send REPEAT START. If
    // there's no other item, send STOP.
    if (queue_head == NULL)
    {
        // queue is empty, so mark tail as null too and send the STOP signal
        queue_tail = NULL;
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA) | _BV (TWINT) | _BV (TWSTO);
    }
    else
    {
        // send REPEAT START signal.
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWEA) | _BV (TWINT) | _BV (TWSTA) | _BV (TWSTO);
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
master_transmitter_handler (status_code)
    uint8_t status_code;
{
    //transmit_string ("[master transmitter mode] status code: ");
    //transmit_int (status_code);
    //transmit_string ("\r\n");
    switch (status_code)
    {
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
            dequeue ();
            break;
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
        //if (tx_slots_free () > 2)
        //{
        //    transmit_int (*(queue_head->data));
        //    transmit_string ("\r\n");
        //}
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
        transmit_string ("I2C error: ");
        transmit_int (status_code);
        transmit_string ("\r\n");
        break;
    }
}

/********************************************************************/

/**
 *  Handle I2C events in master receiver mode.
 */
    void
master_receiver_handler (status_code)
    uint8_t status_code;
{
    uint8_t ack;

    switch (status_code)
    {
    case 0x50:
        // data byte has been received, ACK has been returned. We need to
        // fetch the data from TWDR.
        *(queue_head->data) = TWDR;
        
        // move the pointer to the next data slot, and reduce the length to
        // read.
        queue_head->data ++;
        queue_head->length --;

        //
        // fall through to decide whether to send an ACK or NACK, depending
        // on whether the next byte is the last we want to receive.
        //

    case 0x40:
        // slave address + read has been transmitted, and ACK received. Next
        // action is to set the TWEA bit to send either ACK or NACK after we
        // receive the data byte; ACK if we want to keep receiving more data.
        ack = (queue_head->length > 1)? _BV (TWEA) : 0x00;
        TWCR = _BV (TWINT) | _BV (TWEN) | _BV (TWIE) | ack;
        break;

    case 0x58:
        // data byte has been received, NACK returned. This is the last data
        // byte we want to receive (hopefully). Fetch the data from TWDR and
        // advance the queue to the next item.
        *(queue_head->data) = TWDR;
        dequeue ();
        break;

    case 0x38:
        // Arbitration lost. This shouldn't happen since the MCU should be
        // the only master on the I2C bus.

    case 0x48:
        // NACK received after slave address + read transmitted. This most
        // likely indicates connectivity problems (broken wire etc) or
        // something else that means the slave isn't available.

    default:
        // This should never be reached, as the above cases cover all of the
        // status codes applicable for master receiver mode.
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
    uint8_t status_code = TWSR & 0xF8;

    // check that the queue head is available (if not, ignore the interrupt)
    if (queue_head == NULL)
    {
        TWCR |= _BV (TWINT);
        return;
    }

    // check the status code. If it's 0x08 or 0x10, indicating START or
    // REPEAT START completed, next step is to send the slave address plus
    // read/write bit depending on the operation. Handled here to avoid 
    // duplicating the code.
    if (status_code == 0x08 || status_code == 0x10)
    {
        TWDR = (queue_head->device_address << 1) | 
            ((queue_head->i2c_mode == MASTER_RECEIVER_MODE)? 0x01 : 0x00);
        TWCR = _BV (TWEN) | _BV (TWIE) | _BV (TWINT) | _BV (TWEA);
        return;
    }

    // check the I2C mode of the queue head, and dispatch to the corresponding
    // function
    switch (queue_head->i2c_mode)
    {
    case MASTER_TRANSMITTER_MODE:
        //transmit_string ("[master transmitter]\r\n");
        master_transmitter_handler (status_code);
        break;

    case MASTER_RECEIVER_MODE:
        //transmit_string ("[master receiver]\r\n");
        master_receiver_handler (status_code);
        break;

    default:
        TWCR |= _BV (TWINT);
    }
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
