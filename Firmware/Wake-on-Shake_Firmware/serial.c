/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

serial.cpp
Function implementation for serial write handling.
******************************************************************************/

#include <avr/io.h>
#include <stdio.h>
#include "serial.h"

void setup_usi()
{
    // Now, set up the USI peripheral for communication with the ADXL362 part.
    //   The ADXL362 uses SPI Mode 0- CPHA = CPOL = 0.

    // UISCR- USWM1:0 are mode select pins; 01 is three-wire mode.
    //   USICS1:0 are clock source select pins; 10 is software control.
    //   USICLK puts the 4-bit data counter under software control.
    //   Strobing USITC toggles the 4-bit clock signal.
    USICR = (0 << USIWM1) | (1 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (1 << USICLK);
    // USISR- Writing '1' to USIOIF will clear the 4-bit counter overflow
    //   flag and ready it for the next transfer. Implicit here is a write
    //   of zeroes to bits 3:0 of this register, which also clears the 4-bit
    //   counter.
    USISR = (1 << USIOIF);

    // Set up the USART peripheral for writing in and out. This isn't used
    //   during normal operation- only during user configuration. We'll
    //   set the mode to 9600 baud, 8-N-1. Pretty standard, really.

    // For 9600 baud, at 1.000MHz (which is our clock speed, since we're
    //   using the internal oscillator clocked down), UBRR should be set to
    //   12, and the U2X bit of UCSRA should be set to '1'.
    UBRRH = 0;
    UBRRL = 12;
    UCSRA = (1 << U2X);
    // UCSRB- RXEN and TXEN enable the transmit and receive circuitry.
    //   UCSZ2 is a frame size bit; when set to 0 (as here), the size is
    //   determined by UCSZ1:0 in UCSRC. RXCIE is the receive interrupt
    //   enable bit; we want that interrupt for handling incoming settings
    //   changes while the part is awake.
    UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
    // UCSRC- Setting UCSZ1:0 to '1' gives us an 8-bit frame size. There
    //   are provisions in this register for synchronous mode, parity,
    //   and stop-bit count, but we'll ignore them.
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

// Print a single character out to the serial port. Blocks until write has
//   completed- is that a mistake?
void serialWriteChar(char data)
{
    UDR = data;
    while ((UCSRA & (1 << TXC)) == 0) {
    }                    // Wait for the transmit to finish.
    UCSRA |= (1 << TXC); // Clear the "transmit complete" flag.
}

// serialWrite() takes a pointer to a string and iterates over that string
//   until it finds the C end-of-string character ('\0'). It's a blocking
//   operation and does not return until the print operation is completed.
void serialWrite(char *data)
{
    do {
        serialWriteChar((char)*data); // Print the first character.
        data++;                       // Increment the pointer.
    } while (*data != '\0');          // Check for the end of the string.
    serialNewline();
}

// Convert a 16-bit unsigned value into ASCII characters and dump it out
//   to the serial port.
void serialWriteInt(unsigned int data)
{
    uint8_t tenth = 0;
    uint8_t thou = 0;
    uint8_t huns = 0;
    uint8_t tens = 0;
    uint8_t ones = 0;
    // This is an awkward shifting/dividing method of isolating the individual
    //   digits of the number. I'm sure there's a better way, but done is.
    tenth = (data / 10000);
    data -= tenth * 10000;
    thou = (uint8_t)(data / 1000);
    data -= (uint16_t)thou * 1000;
    huns = (uint8_t)(data / 100);
    data -= (uint16_t)huns * 100;
    tens = (uint8_t)(data / 10);
    ones = (uint8_t)(data % 10);

    tenth += 48;
    thou += 48;
    huns += 48;
    tens += 48;
    ones += 48;
    // Write the individual digits out, followed by a line feed and CR.
    serialWriteChar(tenth);
    serialWriteChar(thou);
    serialWriteChar(huns);
    serialWriteChar(tens);
    serialWriteChar(ones);
    serialNewline();
}

void serialNewline(void)
{
    serialWriteChar((char)'\n');
    serialWriteChar((char)'\r');
}
