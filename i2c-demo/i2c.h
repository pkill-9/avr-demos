/**
 *  i2c.h
 *
 *  Declares functions used to drive the I2C hardware on the ATmega M328P
 *  microcontroller.
 */

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>

void i2c_init (void);
void i2c_send_to (uint8_t device_address, const char *data, unsigned int length);
uint8_t i2c_read_register (uint8_t device_address, uint8_t device_register);
void i2c_read_bytes (uint8_t device_address, char *buffer, unsigned int length);

#endif // _I2C_H

/** vim: set ts=4 sw=4 et : */
