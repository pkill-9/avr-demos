#include <Wire.h>

#define MCP23008_ADDRESS 0x20
#define IODIR_REGISTER    0x00
#define GPIO_REGISTER     0x09
#define GPINTEN           0x02
#define INTCOM            0x04
#define GPPULLUP          0x06
#define INTCAPTURE        0x08

static volatile int pin_changed;


void setup() {
  Wire.begin ();
  Serial.begin (9600);

  // send config commands to mcp23008
  Wire.beginTransmission (MCP23008_ADDRESS);
  Wire.write (IODIR_REGISTER);
  Wire.write (0xFE);
  Wire.endTransmission ();
/*
  Wire.beginTransmission (MCP23008_ADDRESS);
  Wire.write (GPINTEN);
  Wire.write (0x02);
  Wire.endTransmission ();
*/
  Wire.beginTransmission (MCP23008_ADDRESS);
  Wire.write (GPPULLUP);
  Wire.write (0x02);
  Wire.endTransmission ();

  pin_changed = 0;
}

void loop() {
  byte pin_states;
  
  //if (pin_changed != 0) {
    // update state of GPIO register on MCP23008 chip. First
    // fetch the GPIO register contents.
    Wire.beginTransmission (MCP23008_ADDRESS);
    Wire.write (GPIO_REGISTER);
    Wire.endTransmission ();

    Wire.requestFrom (MCP23008_ADDRESS, 1);
    pin_states = Wire.read ();
    Serial.println (pin_states);

    // check the pin states, has the button been pressed?
    if ((pin_states & 0x02) != 0) {
      pin_states = 0x01;
    } else {
      pin_states = 0x00;
    }

    Wire.beginTransmission (MCP23008_ADDRESS);
    Wire.write (GPIO_REGISTER);
    Wire.write (pin_states);
    Wire.endTransmission ();
  //}
  delay (100);
}
