/**
 *  uart.h
 *
 *  Functions to transmit and receive data via built in UART hardware
 */

#ifndef _UART_H
#define _UART_H

#include <string.h>

void uart_init (unsigned long baud_rate);
size_t transmit_string (const char *message);
size_t transmit_int (int value);

#endif // _UART_H

// vim: ts=4 sw=4 et
