/******************************************************************************
Created 26 Nov 2012 by Mike Hord at SparkFun Electronics.
Wake-on-Shake hardware and firmware are released under the Creative Commons
Share Alike v3.0 license:
        http://creativecommons.org/licenses/by-sa/3.0/
Feel free to use, distribute, and sell variants of Wake-on-Shake. All we ask
is that you include attribution of 'Based on Wake-on-Shake by SparkFun'.

wake-on-shake.h
This is the main definitions file for the core code in the project. The few
functions in here are all basically convenience aggregations of calls to
functions in other places.
******************************************************************************/

#ifndef _wake_on_shake_h_included
#define _wake_on_shake_h_included

#include <stdbool.h>

extern volatile uint8_t serialRxData;

#endif
