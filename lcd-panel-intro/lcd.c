/**
 *  Logic to operate a graphical LCD display
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>

#include "lcd.h"

/********************************************************************/

#define CMD_WRITE_RAM           0x2C

#define DCX_CMD                 0
#define DCX_DATA                1

#define SPI_QUEUE_LENGTH        64


typedef struct spi_queue_item
{
    uint8_t data;
    unsigned int dcx_pin;
    struct spi_queue_item *next;
} spi_queue_item_t;

static spi_queue_item_t spi_queue [SPI_QUEUE_LENGTH];
static spi_queue_item_t *queue_start = NULL;
static spi_queue_item_t *queue_end = NULL;
static volatile spi_queue_item_t *free_list = NULL;


/********************************************************************/

static void spi_enqueue (uint8_t message, unsigned int dcx_pin);
static spi_queue_item_t *spi_dequeue (void);


/********************************************************************/

/**
 *  Initialise the SPI module in the ATmega328P so that we can talk to the
 *  LCD panel. Then initialise the LCD panel.
 *
 *  Note: Looking at the schematic for the DFRobot LCD panel breakout that
 *  I've got, it appears that the controller chip is only connected to be
 *  written to by the MCU, so I don't think I can read the value of any
 *  status registers. At least, not using the conventional SPI bus MOSI and
 *  MISO signals. The LCD controller only receives the MOSI from the MCU;
 *  MISO isn't connected.
 */
    void
lcd_init (void)
{
    //
    // Mark all the slots in the spi_queue as free (add them to the free
    // list).
    //
    for (int i = 0; i < SPI_QUEUE_LENGTH; i ++)
    {
        spi_queue [i].next = (spi_queue_item_t *) free_list;
        free_list = spi_queue + i;
    }

    // Set the DCX pin and CS pin to output mode.
    DDRD |= 0x04 | 0x08 | 0x10;

    // Set the SPI CS pin to HIGH. Once we begin a transfer we will pull it
    // low.
    PORTD |= 0x08 | 0x10;
}

/********************************************************************/

/**
 *  Fill the entire panel with a given colour, erasing any other graphics
 *  previously on the display.
 */
    void
lcd_fill_colour (colour)
    uint16_t colour;
{
    spi_enqueue (CMD_WRITE_RAM, DCX_CMD);

    for (int row = 0; row < SCREEN_ROWS; row ++)
    {
        for (int col = 0; col < SCREEN_COLUMNS; col ++)
        {
            spi_enqueue (colour >> 8, DCX_DATA);
            spi_enqueue (colour & 0x00FF, DCX_DATA);
        }
    }
}

/********************************************************************/

/**
 *  Accept data to be sent over the SPI bus.
 *
 *  If there's no SPI transfer in progress, this function will enable the
 *  SPI and place the message in the data register; bypassing the queue.
 *  Otherwise, this function will place the message on the tail of the
 *  queue.
 *
 *  If the queue is full, this function will wait until a queue slot is
 *  available.
 *
 *  Due to blocking on full queue, this function cannot be called with
 *  interrupts disabled (such as within an ISR).
 */
    static void
spi_enqueue (message, dcx_pin)
    uint8_t message;
    unsigned int dcx_pin;
{
    spi_queue_item_t *queue_slot;

    // Check if the SPI is enabled in the control register
    if ((SPCR & _BV (SPE)) == 0)
    {
        // Set the value on the DCX line. This is connected to port D
        // pin 2.
        PORTD = (dcx_pin == 1)? (PORTD | 0x04) : (PORTD & 0xFB);

        // Pull the CS line LOW
        PORTD &= ~0x08;

        // no transfer in progress.
        SPCR |= (_BV (SPE) | _BV (SPIE) | _BV (MSTR));
        SPDR = message;
        return;
    }

    // busy wait until a queue slot becomes available (in case the queue
    // is full).
    while (free_list == NULL)
        ;

    // get the queue slot from the free list and advance the free list
    queue_slot = (spi_queue_item_t *) free_list;
    free_list = free_list->next;

    // store the data to transfer, and append the message to the queue tail.
    queue_slot->data = message;
    queue_slot->dcx_pin = dcx_pin;

    if (queue_end == NULL)
    {
        queue_start = queue_slot;
        queue_end = queue_slot;
    }
    else
    {
        queue_end->next = queue_slot;
        queue_end = queue_slot;
    }
}

/********************************************************************/

/**
 *  Fetch the item at the start of the SPI message queue.
 *
 *  If the queue is empty this function will return null.
 *
 *  This function will also remove the item from the start of the queue,
 *  and update the queue start and end item pointers. The removed item will
 *  also be added onto the free list.
 */
    static spi_queue_item_t *
spi_dequeue (void)
{
    spi_queue_item_t *item;

    // check if the queue is empty.
    if (queue_start == NULL)
        return NULL;

    // fetch the item and update the pointers.
    item = queue_start;
    queue_start = queue_start->next;

    if (queue_start == NULL)
        queue_end = NULL;

    // update the free slots list
    item->next = (spi_queue_item_t *) free_list;
    free_list = item;

    return item;
}

/********************************************************************/

/**
 *  Handler for the SPI serial transfer complete IRQ.
 *
 *  Action is to dequeue the next message to send over SPI and place it in
 *  the data register. If the queue is empty, the SPI will be disabled.
 */
ISR (SPI_STC_vect)
{
    spi_queue_item_t *next_message = spi_dequeue ();

    if (next_message != NULL)
    {
        PORTD = (next_message->dcx_pin == 1)? (PORTD | 0x04) : (PORTD & 0xFB);
        SPDR = next_message->data;
    }
    else
    {
        // clear the SPI enable bit and the interrupt enable bit in the
        // control register
        SPCR &= ~(_BV (SPE) | _BV (SPIE));

        // Bring the CS line HIGH to indicate to the LCD controller that
        // the SPI bus isn't talking to it now.
        PORTD |= 0x08;
    }
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
