
#ifndef FIRMWARE_H_
#define FIRMWARE_H_

#include "sc_types.h"
		
#ifdef __cplusplus
extern "C" { 
#endif 

/*! \file Header of the state machine 'firmware'.
*/

/*! Enumeration of all states */ 
typedef enum
{
	Firmware_main_region_idle,
	Firmware_main_region_running,
	Firmware_main_region_running_headlights_on,
	Firmware_main_region_running_headlights_off,
	Firmware_main_region_running_taillights_brakes_on,
	Firmware_main_region_running_taillights_brakes_off,
	Firmware_main_region_error,
	Firmware_last_state
} FirmwareStates;

/*! Type definition of the data structure for the FirmwareIfaceCar interface scope. */
typedef struct
{
	sc_boolean brakes_on;
} FirmwareIfaceCar;

/*! Type definition of the data structure for the FirmwareIfaceMCU interface scope. */
typedef struct
{
	sc_boolean error_raised;
} FirmwareIfaceMCU;

/*! Type definition of the data structure for the FirmwareIfaceHMI interface scope. */
typedef struct
{
	sc_boolean button_click_raised;
	sc_boolean button_long_press_raised;
} FirmwareIfaceHMI;


/*! Define dimension of the state configuration vector for orthogonal states. */
#define FIRMWARE_MAX_ORTHOGONAL_STATES 2
	/*! Define dimension of the state configuration vector for history states. */
#define FIRMWARE_MAX_HISTORY_STATES 1

/*! 
 * Type definition of the data structure for the Firmware state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
	FirmwareStates stateConfVector[FIRMWARE_MAX_ORTHOGONAL_STATES];
	FirmwareStates historyVector[FIRMWARE_MAX_HISTORY_STATES];
	sc_ushort stateConfVectorPosition; 
	
	FirmwareIfaceCar ifaceCar;
	FirmwareIfaceMCU ifaceMCU;
	FirmwareIfaceHMI ifaceHMI;
} Firmware;

/*! Initializes the Firmware state machine data structures. Must be called before first usage.*/
extern void firmware_init(Firmware* handle);

/*! Activates the state machine */
extern void firmware_enter(Firmware* handle);

/*! Deactivates the state machine */
extern void firmware_exit(Firmware* handle);

/*! Performs a 'run to completion' step. */
extern void firmware_runCycle(Firmware* handle);


/*! Gets the value of the variable 'brakes_on' that is defined in the interface scope 'Car'. */ 
extern sc_boolean firmwareIfaceCar_get_brakes_on(const Firmware* handle);
/*! Sets the value of the variable 'brakes_on' that is defined in the interface scope 'Car'. */ 
extern void firmwareIfaceCar_set_brakes_on(Firmware* handle, sc_boolean value);
/*! Raises the in event 'error' that is defined in the interface scope 'MCU'. */ 
extern void firmwareIfaceMCU_raise_error(Firmware* handle);

/*! Raises the in event 'button_click' that is defined in the interface scope 'HMI'. */ 
extern void firmwareIfaceHMI_raise_button_click(Firmware* handle);

/*! Raises the in event 'button_long_press' that is defined in the interface scope 'HMI'. */ 
extern void firmwareIfaceHMI_raise_button_long_press(Firmware* handle);


/*!
 * Checks whether the state machine is active (until 2.4.1 this method was used for states).
 * A state machine is active if it was entered. It is inactive if it has not been entered at all or if it has been exited.
 */
extern sc_boolean firmware_isActive(const Firmware* handle);

/*!
 * Checks if all active states are final. 
 * If there are no active states then the state machine is considered being inactive. In this case this method returns false.
 */
extern sc_boolean firmware_isFinal(const Firmware* handle);

/*! Checks if the specified state is active (until 2.4.1 the used method for states was called isActive()). */
extern sc_boolean firmware_isStateActive(const Firmware* handle, FirmwareStates state);

#ifdef __cplusplus
}
#endif 

#endif /* FIRMWARE_H_ */
