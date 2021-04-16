#include "uart.h"

#define BUFFER_LENGTH 32

struct buffer {
    char data [BUFFER_LENGTH];
    int head_pos;
    int tail_pos;
};

struct buffer transmit_queue;
struct buffer receive_queue;

/********************************************************************/

void init_uart (int baud_rate) {
}

/********************************************************************/

void transmit_byte (char byte) {
}

/********************************************************************/

char receive_byte (void) {
}

// vim: ts=4 sw=4 et
