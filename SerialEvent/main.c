#include "uart.h"

/********************************************************************/

#define BUFFER_SIZE     128
char buffer [BUFFER_SIZE];

/********************************************************************/

/**
 *  ARDUINO SERIALEVENT DEMO, IN C
 *
 *  Reads lines of text from the UART, and echos them back.
 *
 *  This version uses more interrupt driven logic compared to the Arduino
 *  example, and hopefully is easier to understand, and uses less power as a
 *  result.
 */
    int
main (void)
{
    uart_init (9600);

    while (1)
    {
        // read a string from the UART and echo it back.
        uart_getline (buffer, BUFFER_SIZE);
        transmit_string (buffer);
    }

    return 0;
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
