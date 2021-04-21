/**
 *  uart.h
 *
 *  Functions to transmit and receive data via built in UART hardware
 */

#ifndef _UART_H
#define _UART_H

#include <string.h>

void uart_init (int baud_rate);
void transmit_byte (char byte);
char receive_byte (void);
size_t transmit_string (const char *message);

#endif // _UART_H

// vim: ts=4 sw=4 et
