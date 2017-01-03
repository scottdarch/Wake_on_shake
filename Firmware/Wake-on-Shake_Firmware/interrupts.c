/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

interrupts.cpp
Interrupt service routine code. ISR length is minimized as much as possible;
the application in question doesn't really have hard real-time deadlines, so
we're free to be kind of lax with our response times.
******************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupts.h"
#include "wake-on-shake.h"
#include "serial.h"

// INT0 ISR- This is one way the processor can wake from sleep. INT0 is tied
//   externally to the RX pin, so traffic on the serial receive line will
//   wake up the part when it is asleep. Note that the receive interrupt
//   can't wake the processor from sleep- don't try!
ISR(INT0_vect)
{
    disable_external_interrupts();
}

// INT1 ISR- this is the primary way the processor wakes from sleep. INT1 is
//   tied to the interrupt output pin on the ADXL362, which goes low when
//   motion is detected.
ISR(INT1_vect)
{
    disable_external_interrupts();
}

// USART_RX ISR- gets called when the processor is awake and a complete
//   byte (including stop bit) has been received by the USART. This
//   interrupt CANNOT be used to wake the processor, so don't try it.
ISR(USART0_RX_vect)
{
    TCNT1 = 0;          // Reset the wakefulness timer, so the processor
                        //   doesn't go to sleep while the user is
                        //   interacting with it.
    serialRxData = UDR; // Pass the data back to the main loop for parsing.
}

void enable_external_interrupts(void)
{
    GIMSK = (1 << INT0) | (1 << INT1); // Enable external interrupts to wake the
                                       //   processor up; INT0 is incoming serial
                                       //   data, INT1 is accelerometer interrupt
}

void disable_external_interrupts(void)
{
    GIMSK = (0 << INT0) | (0 << INT1); // Disable INT pins while we're awake.
                                       //  This is important b/c the INT pins
                                       //  cause an interrupt on LOW rather
                                       //  than on an edge, so the interrupt
                                       //  will continue to fire as long as
                                       //  the pin is low unless it is disabled.
}

void setup_interrupts()
{
    // We'll need two interrupts: INT0 and INT1.
    //   INT0 will wiggle when a serial connection occurs. INT1 will wiggle when
    //   the ADXL362 detects motion and wakes up.

    // MCUCR- Bits 0:3 control the interrupt event we want to be sensitive to.
    //   We'll set it so a low level is what each pin is looking for.
    MCUCR = (0 << ISC11) | (0 << ISC10) | (0 << ISC01) | (0 << ISC00);

    disable_external_interrupts();
}
