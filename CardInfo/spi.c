/**
 *  Basic functions to send & receive data over the SPI bus.
 */

#include <avr/io.h>

#include "spi.h"

/********************************************************************/

/**
 *  Send and/or receive a single byte over SPI.
 *
 *  Note that since the bus is bi-directional (MOSI and MISO lines), sending
 *  and receiving are done simultaneously. If you don't want to receive any
 *  data, just ignore the value that this function returns. If you don't have
 *  anything to send, and just want to receive data from the slave, set the
 *  byte to be sent to 0x00 or 0xFF (or some similar value that the slave will
 *  ignore).
 *
 *  This function does not handle the chip select signal; the caller must pull
 *  the correct line low to indicate which slave you want to talk to. When this
 *  function returns, the SPI transfer is complete (hardware isn't still
 *  working in the background), so it is safe to release the chip select signal
 *  immediately on return.
 */
    uint8_t
spi_transaction_byte (data)
    uint8_t data;
{
    uint8_t result;

    // loading a value into the SPDR register triggers the hardware to send
    // that value, and simultaneously receive a byte from the slave.
    SPCR |= (_BV (SPE) | _BV (MSTR));
    SPDR = data;

    // busy waiting until the SPI transfer is complete. SPI is a lot faster
    // than I2C or UART, which seems to cause timing issues with interrupt
    // handling.
    while ((SPSR & _BV (SPIF)) == 0)
        ;

    result = SPDR;
    SPCR &= ~_BV (SPE);

    return result;
}

/********************************************************************/

/**
 *  Send & receive a 16 bit value over the SPI bus.
 */
    uint16_t
spi_transaction_16 (data)
    uint16_t data;
{
    uint16_t result = 0x0000;

    // start with the left most byte (MSB)
    result |= spi_transaction_byte (data >> 8) << 8;
    result |= spi_transaction_byte (data & 0xFF);

    return result;
}

/********************************************************************/

/**
 *  Send & receive 32 bit values over the SPI bus
 */
    uint32_t
spi_transaction_32 (data)
    uint32_t data;
{
    uint32_t result = 0x00000000;

    result |= spi_transaction_byte (data >> 24) << 24;
    result |= spi_transaction_byte (data >> 16) << 16;
    result |= spi_transaction_byte (data >> 8) << 8;
    result |= spi_transaction_byte (data);

    return result;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
