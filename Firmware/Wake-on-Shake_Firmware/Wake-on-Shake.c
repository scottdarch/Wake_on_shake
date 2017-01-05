/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

Original wake-on-shake concept by Nitzan Gadish, Analog Devices

Modified/simplified to create brake lights for my son's pinewood derby car.

******************************************************************************/
// avrdude programming string
// avrdude -p t2313 -B 10 -P usb -c avrispmkii -U flash:w:Wake-on-Shake.hex -U hfuse:w:0xdf:m -U
// lfuse:w:0x64:m

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "serial.h"
#include "wake-on-shake.h"
#include "interrupts.h"
#include "spi.h"
#include "ADXL362.h"
#include "xl362.h"

// +---------------------------------------------------------------------------+
// | EXTERN DATA
// +---------------------------------------------------------------------------+

volatile uint8_t serialRxData = 0; // Data passing variable to get data
                                   //   from the receive ISR back to main.

// +---------------------------------------------------------------------------+
// | CONTROL FUNCTIONS
// +---------------------------------------------------------------------------+

static void apply_brakes(void)
{
    // High duty cycle (brake lights on full)
    OCR0A = 250;
    OCR1AL = 250;
    serialWrite("brakes on");
}

static void release_brakes(void)
{
    // low duty cycle (brake lights dim)
    OCR0A = 10;
    OCR1AL = 10;
    serialWrite("brakes off");
}

static void turn_on_headlights(void)
{
    PORTD |= (1 << PD4);
}

// static void turn_off_headlights(void)
//{
//    PORTD &= !(1 << PD4);
//}

// +---------------------------------------------------------------------------+
// | INIT
// +---------------------------------------------------------------------------+
static void setup_gpio(void)
{
    // Port A- both pins are unused, so we'll make them both outputs and drive them
    //   low to save power.
    DDRA = (1 << PA1) | (1 << PA0);
    // Port B- PB0-3 are unused; make them outputs and tie them low. PB4 is !CS
    //   for the ADXL362, so it should be an output. PB5 is MISO from the ADXL362, so
    //   leave it an input. PB6 is MOSI to ADXL362, and PB7 is SCK, so they should be
    //   outputs.
    DDRB = (1 << PB7) | (1 << PB6) | (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0);
    // No port C pins on this chip
    // Port D- PD0 is the serial receive input. PD1 is serial transmit output. PD2 is
    //   is an external interrupt used to wake the processor on serial activity. PD3
    //   is the interrupt from the ADXL362, used to wake the device on detected
    //   motion. PD4 is the MOSFET turn-on signal, so should be an output. PD5 and
    //   PD6 are unused; make them outputs and tie them low. PD7 doesn't exist.
    DDRD = (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD1);

    // Now let's configure some initial IO states

    // Port A- both set low to reduce current consumption
    PORTA = (0 << PA1) | (0 << PA0);
    // Port B- PB0-3 should be low for power consumption; PB4 is !CS, so bring it
    //   high to keep the ADXL362 non-selected. Others are don't care.
    PORTB = (1 << PB4) | (0 << PB3) | (0 << PB2) | (0 << PB1) | (0 << PB0);
    // Port C doesn't exist
    // Port D- PD5 and PD6 should be tied low; others are (for now) don't care.
    //   Also, PD2/PD0 should be made high to enable pullup resistor for when no
    //   serial connection is present.
    PORTD = (1 << PD6) | (1 << PD5) | (1 << PD2) | (1 << PD0);
}

static void setup_timers(void)
{
    // +--[TIMER 0]------------------------------------------------------------+
    // Timer0 is setup to provide an 8% duty cycle at 490Hz when enabled. This
    // is used as the "brake off" current sent to the brake lights.

    // Using fast PWM
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS01);

    TCCR1A = (1 << COM1A1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1AH = 0;
}

void _init_MCU(void) __attribute__((naked)) __attribute__((section(".init3")));

void _init_MCU(void)
{
    setup_gpio();
    setup_interrupts();
    setup_usi();
    setup_timers();
    set_sleep_mode(SLEEP_MODE_IDLE);
}

void _init_peripherals(void) __attribute__((naked)) __attribute__((section(".init8")));

void _init_peripherals(void)
{
    ADXLConfig();
    serialWrite("periph. init");
}

// +---------------------------------------------------------------------------+
// | MAIN
// +---------------------------------------------------------------------------+
int main(void)
{
    static bool was_awake = true;

    serialWrite("ready");
    apply_brakes();
    turn_on_headlights();

    sei();

    while (1) {
        if (!ADXLIsAwake()) {
            was_awake = false;
            serialWrite("zzzzzz"); // Let the user know sleep mode is coming.

            enable_external_interrupts();

            apply_brakes();
            sleep_mode(); // Go to sleep until woken by an interrupt.
            release_brakes();
        } else if (!was_awake) {
            was_awake = true;
            serialWrite("Wha? Oh. I'm up.");
        } else {
            for (uint8_t i = 0; i < 255; ++i) {
                _NOP();
            }
        }
    }
}
