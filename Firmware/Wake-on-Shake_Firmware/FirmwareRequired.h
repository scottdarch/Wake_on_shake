
#ifndef FIRMWAREREQUIRED_H_
#define FIRMWAREREQUIRED_H_

#include "sc_types.h"
#include "Firmware.h"

#ifdef __cplusplus
extern "C"
{
#endif 

/*! \file This header defines prototypes for all functions that are required by the state machine implementation.

This state machine makes use of operations declared in the state machines interface or internal scopes. Thus the function prototypes:
	- firmwareIfaceCar_start_car
	- firmwareIfaceCar_stop_car
	- firmwareIfaceMCU_wait_for_interrupt
	- firmwareIfaceHeadLights_on
	- firmwareIfaceHeadLights_off
	- firmwareIfaceTailLights_bright
	- firmwareIfaceTailLights_dim
	- firmwareIfaceTailLights_off
are defined.

These functions will be called during a 'run to completion step' (runCycle) of the statechart. 
There are some constraints that have to be considered for the implementation of these functions:
	- never call the statechart API functions from within these functions.
	- make sure that the execution time is as short as possible.
 
*/
extern void firmwareIfaceCar_start_car(const Firmware* handle);
extern void firmwareIfaceCar_stop_car(const Firmware* handle);

extern void firmwareIfaceMCU_wait_for_interrupt(const Firmware* handle);

extern void firmwareIfaceHeadLights_on(const Firmware* handle);
extern void firmwareIfaceHeadLights_off(const Firmware* handle);

extern void firmwareIfaceTailLights_bright(const Firmware* handle);
extern void firmwareIfaceTailLights_dim(const Firmware* handle);
extern void firmwareIfaceTailLights_off(const Firmware* handle);





#ifdef __cplusplus
}
#endif 

#endif /* FIRMWAREREQUIRED_H_ */
