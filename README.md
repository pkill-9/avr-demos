# avr-demos
Selected Arduino examples written in C

## Overview
I learned about Arduino as an introduction to microcontrollers, and for sure it has many advantages in
being able to build prototype circuits quickly, without having to trawl through AVR datasheets.
However, I've always preferred to use Vim and makefiles for C or C++ projects.

Fortunately, I found an example makefile which I was able to adapt to my purposes with only minor
changes [original version here](https://github.com/ComputerNerd/ov7670-no-ram-arduino-uno/blob/master/Makefile)

After much reading of the AVR datasheets, and header files in avr-gcc's include directory, I managed to get
an LED to blink! So I decided to share what I'd done, in the hope that it's a useful learning resource for
others.

## Requirements
* to compile the source code to the hex image, you will need avr-gcc and friends. I believe the packages are gcc-avr, and avr-libc
* to transfer the hex image to the chip, you will need avrdude installed, and also hardware: an **ISP programmer**. I used a USBtinyISP programmer, so the makefiles will work with that by default. If you have a different programmer you will need to edit the makefile so that it invokes avrdude with the correct programmer flag.
* Tested on an ATmega328P microcontroller. If you want to run the demos on a different MCU, again you will need to edit the makefile.
