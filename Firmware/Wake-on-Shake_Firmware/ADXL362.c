/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

ADXL362.cpp
Function code for the ADXL362. If you're looking for register aliases,
check out xl362.h, which was provided by Analog Devices.
******************************************************************************/

#include <avr/io.h>
#include "ADXL362.h"
#include "spi.h"
#include "xl362.h"
#include "wake-on-shake.h"

// ADXLConfig() sets all the necessary registers on the ADXL362 up to support
//   the wake-on-shake type application.
void ADXLConfig(void)
{
    // Activity threshold level (0x20)-
    //   Defaults to 150mg; user can change this.
    ADXLWriteByte((uint8_t)XL362_THRESH_ACTH, 0);
    ADXLWriteByte((uint8_t)XL362_THRESH_ACTL, 150);
    // Inactivity threshold level (0x23)-
    //   Written to 50 to give a 50mg sleep detection level
    ADXLWriteByte((uint8_t)XL362_THRESH_INACTH, 0);
    ADXLWriteByte((uint8_t)XL362_THRESH_INACTL, 50);
    // Inactivity timer (0x25)-
    //   Written to 15; wait 15 samples (~2.5 seconds) before going back
    //   to sleep.
    ADXLWriteByte((uint8_t)XL362_TIME_INACTH, 0);
    ADXLWriteByte((uint8_t)XL362_TIME_INACTL, 5);
    ADXLWriteByte((uint8_t)XL362_ACT_INACT_CTL, (uint8_t)0xFF);
    // INT1 function map register (0x2A)-
    //   Needs to be set to "Active Low" (7 = 1)
    //   Needs to be set to activity mode (4 = 1)
    //   Other bits must be zero.
    ADXLWriteByte((uint8_t)XL362_INTMAP1, (uint8_t)0b10010000);

    // Autosleep with ultra-low-noise.
    ADXLWriteByte((uint8_t)XL362_POWER_CTL, (uint8_t)0x26);
}

// Simple functions to assert chip select and copy data in and out of the
//   ADXL362.
uint8_t ADXLReadByte(uint8_t addr)
{
    PORTB &= !(1 << PB4);
    spiXfer((uint8_t)XL362_REG_READ);
    spiXfer(addr);
    addr = spiXfer(addr);
    PORTB |= (1 << PB4);
    return addr;
}

void ADXLWriteByte(uint8_t addr, uint8_t data)
{
    PORTB &= !(1 << PB4);
    spiXfer((uint8_t)XL362_REG_WRITE);
    spiXfer(addr);
    spiXfer(data);
    PORTB |= (1 << PB4);
}

bool ADXLIsAwake(void)
{
    const uint8_t status = ADXLReadByte(XL362_STATUS);
    return ((0x40 & status) != 0);
}

uint8_t ADXLReadXMSB(void)
{
    return ADXLReadByte(XL362_XDATA8);
}
