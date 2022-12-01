/**
 *  Logic to operate a graphical LCD display
 */

#include <avr/interrupt.h>
#include <stddef.h>

#include "lcd.h"

/********************************************************************/

#define SPI_QUEUE_LENGTH        64


typedef struct spi_queue_item
{
    uint8_t data;
    struct spi_queue_item *next;
} spi_queue_item_t;

static spi_queue_item_t spi_queue [SPI_QUEUE_LENGTH];
static spi_queue_item_t *queue_start = NULL;
static spi_queue_item_t *queue_end = NULL;


/********************************************************************/

static void spi_enqueue (uint8_t message);
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
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
