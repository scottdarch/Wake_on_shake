/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

Original wake-on-shake concept by Nitzan Gadish, Analog Devices

Copyright 2017 Scott A Dixon

Modified to create lights for my son's pinewood derby car with simulated, working
brake lights.

******************************************************************************/
/*
                           ATTiny 2313A
                             +-----+
                         PA2 |     | VCC
                   RX -> PD0 |     | PB7 -> ADXL SCK
                   TX <- PD1 |     | PB6 -> ADXL MOSI
                   (n/c) PA1 |     | PB5 <- ADXL MISO
                   (n/c) PA0 |     | PB4 -> ADXL !CS
                 INT0 -> PD2 |     | PB3/OC1A -> Brake Light PWM
            ADXL INT1 -> PD3 |     | PB2/OC0A -> Brake Light PWM
    HeadLights Mosfet <- PD4 |     | PB1 (GPIO, not used)
                   (n/c) PD5 |     | PB0 <- button
                         GND |     | PD6 (GPIO, not used)
                             +-----+

 */

// +---------------------------------------------------------------------------+
#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "wake-on-shake.h"
#include "interrupts.h"
#include "spi.h"
#include "ADXL362.h"
#include "xl362.h"
#include "FirmwareRequired.h"

// +---------------------------------------------------------------------------+
// | EXTERN DATA
// +---------------------------------------------------------------------------+

volatile uint8_t serialRxData = 0; // Data passing variable to get data
                                   // from the receive ISR back to main.

// +---------------------------------------------------------------------------+
// | CAR
// +---------------------------------------------------------------------------+
static void taillights_bright(void)
{
    // High duty cycle (brake lights on full)
    OCR0A = 250;
    OCR1AH = 0;
    OCR1AL = 250;
}

static void taillights_dim(void)
{
    // low duty cycle (brake lights dim)
    OCR0A = 10;
    OCR1AH = 0;
    OCR1AL = 10;
}

static void taillights_off(void)
{
    PORTB &= ~((1 << PB3) | (1 << PB2));
}

static void taillights_on(void)
{
    PORTB |= ((1 << PB3) | (1 << PB2));
}

static void headlights_on(void)
{
    PORTD |= (1 << PD4);
}

static void headlights_off(void)
{
    PORTD &= ~(1 << PD4);
}

// +---------------------------------------------------------------------------+
// | STATE MACHINE
// +---------------------------------------------------------------------------+

static Firmware _sc_firmware;

// +---------------------------------------------------------------------------+
// | BUTTON
// +---------------------------------------------------------------------------+
#define BUTTON_DOWN_MASK 0xFFF
#define BUTTON_LONG_PRESS_MASK 0xFFFFFFFF
#define BUTTON_LONG_PRESS_COUNT 32
#define BUTTON_PORT PIND
#define BUTTON_PIN PIND0

static uint16_t _button_state = 0;
static uint32_t _long_click_state = 0;
static uint32_t _long_click_count = 0;
static bool _is_button_down = false;
static bool _is_button_long_pressed = false;

static void _update_button_state(void)
{
    _button_state = (_button_state << 1) | (uint16_t)((BUTTON_PORT & (1 << BUTTON_PIN)) ? 0 : 1);
    if (!_is_button_down && (_button_state & BUTTON_DOWN_MASK) == BUTTON_DOWN_MASK) {
        _is_button_down = true;
    } else if (_is_button_down) {
        if ((_button_state & BUTTON_DOWN_MASK) == 0) {
            // button is now up.
            _is_button_down = false;
            _long_click_count = 0;
            _long_click_state = 0;
            if (!_is_button_long_pressed) {
                firmwareIfaceHMI_raise_button_click(&_sc_firmware);
            } else {
                _is_button_long_pressed = false;
            }
        } else if (!_is_button_long_pressed) {
            _long_click_state = (_long_click_state << 1) | 1;
            if ((_long_click_state & BUTTON_LONG_PRESS_MASK) == BUTTON_LONG_PRESS_MASK) {
                ++_long_click_count;
                _long_click_state = 0;
                if (_long_click_count == BUTTON_LONG_PRESS_COUNT) {
                    _is_button_long_pressed = true;
                    firmwareIfaceHMI_raise_button_long_press(&_sc_firmware);
                }
            }
        }
    }
}

// +---------------------------------------------------------------------------+
// | INIT
// +---------------------------------------------------------------------------+
static void setup_gpio(void)
{
    // Port A- both pins are unused, so we'll make them both outputs and drive them
    //   low to save power.
    DDRA = (1 << PA1) | (1 << PA0);

    // Port B- PB1-3 are either unused or outputs; make them outputs and tie them
    //   low.
    //   PB0 is input for for the multi-function button.
    //   PB4 is !CS for the ADXL362, so it should be an output.
    //   PB5 is MISO from the ADXL362, so leave it as input.
    //   PB6 is MOSI to ADXL362 set as output
    //   PB7 is SCK to ADXL362 set as output
    DDRB = (1 << PB7) | (1 << PB6) | (1 << PB4) | (1 << PB3) | (1 << PB2) | (1 << PB1);

    // Port D- PD0 is the serial receive input.
    //         PD1 is serial transmit output.
    //         PD2 is an external interrupt used to wake the processor on serial
    //             activity.
    //         PD3 is the interrupt from the ADXL362, used to wake the device on
    //             detected motion.
    //         PD4 is the MOSFET turn-on signal, so should be an output. PD5 and
    //         PD6 is unused.
    DDRD = (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD1);

    // Port A- both set low to reduce current consumption
    PORTA = (0 << PA1) | (0 << PA0);

    // Port B- PB1-3 should be low for power consumption; PB4 is !CS, so bring it
    //   high to keep the ADXL362 non-selected. Others are don't care.
    PORTB = (1 << PB4) | (0 << PB3) | (0 << PB2) | (0 << PB1) | (1 << PB0);

    // Port D- PD5 and PD6 should be tied low; others are (for now) don't care.
    //   Also, PD2/PD0 should be made high to enable pullup resistor for when no
    //   serial connection is present.
    PORTD = (1 << PD6) | (1 << PD5) | (1 << PD2) | (1 << PD0);
}

static void start_timers(void)
{
    // +--[TIMER 0]------------------------------------------------------------+
    // Timer0 is setup to provide an 8% duty cycle at 490Hz when enabled. This
    // is used as the "brake off" current sent to the brake lights.

    // Using fast PWM
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS01);

    TCCR1A = (1 << COM1A1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS11);

    _NOP();
}

static void stop_timers(void)
{
    TCCR0A = 0;
    TCCR0B = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    _NOP();
}

void _init_MCU(void) __attribute__((naked)) __attribute__((section(".init3")));

void _init_MCU(void)
{
    setup_gpio();
    setup_interrupts();
    // setup_usi();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void _init_peripherals(void) __attribute__((naked)) __attribute__((section(".init8")));

void _init_peripherals(void)
{
    ADXLConfig();

    firmware_init(&_sc_firmware);
}

// +---------------------------------------------------------------------------+
// | MAIN
// +---------------------------------------------------------------------------+
int main(void)
{
    firmware_enter(&_sc_firmware);

    // serialWrite("ready");

    sei();

    while (1) {
        _update_button_state();
        const uint8_t status = ADXLGetStatus();
        if (status & XL362_STATUS_ERR) {
            firmwareIfaceMCU_raise_error(&_sc_firmware);
        } else if (status & XL362_INT_AWAKE) {
            firmwareIfaceCar_set_brakes_on(&_sc_firmware, false);
        } else {
            firmwareIfaceCar_set_brakes_on(&_sc_firmware, true);
        }
        firmware_runCycle(&_sc_firmware);
    }

    firmware_exit(&_sc_firmware);
}

// +---------------------------------------------------------------------------+
// | FirmwareRequired
// +---------------------------------------------------------------------------+
void firmwareIfaceMCU_wait_for_interrupt(const Firmware *handle)
{
    if (_button_state == 0) {
        enable_external_interrupts();
        sleep_mode();
    }
}

void firmwareIfaceMCU_handle_error(const Firmware *handle)
{
    ADXLConfig();
    stop_timers();
    headlights_on();
    taillights_on();
    for (uint32_t i = 0; i < 0xFFFFFFFF; ++i) {
        _NOP();
    }
}

void firmwareIfaceHeadLights_on(const Firmware *handle)
{
    headlights_on();
}

void firmwareIfaceHeadLights_off(const Firmware *handle)
{
    headlights_off();
}

void firmwareIfaceTailLights_bright(const Firmware *handle)
{
    taillights_bright();
}

void firmwareIfaceTailLights_dim(const Firmware *handle)
{
    taillights_dim();
}

void firmwareIfaceTailLights_off(const Firmware *handle)
{
    taillights_off();
}

void firmwareIfaceCar_start_car(const Firmware *handle)
{
    ADXLConfig();
    ADXLEnable();
    taillights_off();
    start_timers();
}

void firmwareIfaceCar_stop_car(const Firmware *handle)
{
    ADXLDisable();
    stop_timers();
    taillights_off();
    headlights_off();
}
