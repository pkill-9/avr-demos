/**
 *  uart.h
 *
 *  Functions to transmit and receive data via built in UART hardware
 */

#ifndef _UART_H
#define _UART_H

#include <string.h>
#include <stdint.h>

#define DECIMAL     10
#define HEX         0x10

void uart_init (unsigned long baud_rate);
size_t transmit_string (const char *message);
size_t transmit_int (int value, int base);
int uart_printf (const char *format, ...);

char uart_getchar (void);
size_t uart_getline (char *buffer, size_t max_length);

#endif // _UART_H

// vim: ts=4 sw=4 et
