/**
 *  spi.h
 *
 *  Functions for interacting with the SPI hardware.
 */

#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>


uint8_t spi_transaction_byte (uint8_t mosi);
uint16_t spi_transaction_16 (uint16_t mosi);
uint32_t spi_transaction_32 (uint32_t mosi);

#endif // _SPI_H

/** vim: set ts=4 sw=4 et : */
