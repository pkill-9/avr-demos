/**
 *  touch.c
 *
 *  Functions for controlling and interacting with a CAP1188 touch sensor
 *  module. Allows the caller to configure a function to be invoked when a
 *  touch event is detected on a specified channel.
 */

#include "touch.h"
#include "i2c.h"

/********************************************************************/

#define I2C_CHANNEL     0x29    // default channel.

/********************************************************************/

/**
 *  Prepare the CAP1188 chip for detecting touch events.
 *
 *  Callback functions must be added separately by calling install_handler
 */
    void
init (void)
{
}

/********************************************************************/

/**
 *  Configure a function to be invoked by touch events on the specified channel.
 *  When the function is invoked, the channel number where the touch event
 *  was detected will be passed as an argument.
 */
    void
install_handler (handler, channel)
    void (*handler) (uint8_t);
    uint8_t channel;
{
}

/********************************************************************/

/** vim: set ts=4 sw=4 et : */
