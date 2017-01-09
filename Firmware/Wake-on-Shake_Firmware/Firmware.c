
#include <stdlib.h>
#include <string.h>
#include "sc_types.h"
#include "Firmware.h"
#include "FirmwareRequired.h"
/*! \file Implementation of the state machine 'firmware'
*/

/* prototypes of all internal functions */
static sc_boolean firmware_check_main_region_idle_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_idle_lr0_lr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_tr1_tr1(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_lr1_lr1(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_headlights_on_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_headlights_off_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_taillights_brakes_on_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_running_taillights_brakes_off_tr0_tr0(const Firmware* handle);
static sc_boolean firmware_check_main_region_error_tr0_tr0(const Firmware* handle);
static void firmware_effect_main_region_idle_tr0(Firmware* handle);
static void firmware_effect_main_region_idle_lr0_lr0(Firmware* handle);
static void firmware_effect_main_region_running_tr0(Firmware* handle);
static void firmware_effect_main_region_running_tr1(Firmware* handle);
static void firmware_effect_main_region_running_lr1_lr1(Firmware* handle);
static void firmware_effect_main_region_running_headlights_on_tr0(Firmware* handle);
static void firmware_effect_main_region_running_headlights_off_tr0(Firmware* handle);
static void firmware_effect_main_region_running_taillights_brakes_on_tr0(Firmware* handle);
static void firmware_effect_main_region_running_taillights_brakes_off_tr0(Firmware* handle);
static void firmware_effect_main_region_error_tr0(Firmware* handle);
static void firmware_enact_main_region_running(Firmware* handle);
static void firmware_enact_main_region_running_headlights_on(Firmware* handle);
static void firmware_enact_main_region_running_headlights_off(Firmware* handle);
static void firmware_enact_main_region_running_taillights_brakes_on(Firmware* handle);
static void firmware_enact_main_region_running_taillights_brakes_off(Firmware* handle);
static void firmware_enact_main_region_error(Firmware* handle);
static void firmware_exact_main_region_running(Firmware* handle);
static void firmware_enseq_main_region_idle_default(Firmware* handle);
static void firmware_enseq_main_region_running_default(Firmware* handle);
static void firmware_enseq_main_region_running_headlights_on_default(Firmware* handle);
static void firmware_enseq_main_region_running_headlights_off_default(Firmware* handle);
static void firmware_enseq_main_region_running_taillights_brakes_on_default(Firmware* handle);
static void firmware_enseq_main_region_running_taillights_brakes_off_default(Firmware* handle);
static void firmware_enseq_main_region_error_default(Firmware* handle);
static void firmware_enseq_main_region_default(Firmware* handle);
static void firmware_enseq_main_region_running_headlights_default(Firmware* handle);
static void firmware_shenseq_main_region_running_headlights(Firmware* handle);
static void firmware_enseq_main_region_running_taillights_default(Firmware* handle);
static void firmware_exseq_main_region_idle(Firmware* handle);
static void firmware_exseq_main_region_running(Firmware* handle);
static void firmware_exseq_main_region_running_headlights_on(Firmware* handle);
static void firmware_exseq_main_region_running_headlights_off(Firmware* handle);
static void firmware_exseq_main_region_running_taillights_brakes_on(Firmware* handle);
static void firmware_exseq_main_region_running_taillights_brakes_off(Firmware* handle);
static void firmware_exseq_main_region_error(Firmware* handle);
static void firmware_exseq_main_region(Firmware* handle);
static void firmware_exseq_main_region_running_headlights(Firmware* handle);
static void firmware_exseq_main_region_running_taillights(Firmware* handle);
static void firmware_react_main_region_idle(Firmware* handle);
static void firmware_react_main_region_running_headlights_on(Firmware* handle);
static void firmware_react_main_region_running_headlights_off(Firmware* handle);
static void firmware_react_main_region_running_taillights_brakes_on(Firmware* handle);
static void firmware_react_main_region_running_taillights_brakes_off(Firmware* handle);
static void firmware_react_main_region_error(Firmware* handle);
static void firmware_react_main_region__entry_Default(Firmware* handle);
static void firmware_react_main_region_running_headlights__entry_Default(Firmware* handle);
static void firmware_react_main_region_running_taillights__entry_Default(Firmware* handle);
static void firmware_clearInEvents(Firmware* handle);
static void firmware_clearOutEvents(Firmware* handle);


void firmware_init(Firmware* handle)
{
	sc_integer i;

	for (i = 0; i < FIRMWARE_MAX_ORTHOGONAL_STATES; ++i)
	{
		handle->stateConfVector[i] = Firmware_last_state;
	}
	
	for (i = 0; i < FIRMWARE_MAX_HISTORY_STATES; ++i)
	{
		handle->historyVector[i] = Firmware_last_state;
	}
	
	handle->stateConfVectorPosition = 0;

	firmware_clearInEvents(handle);
	firmware_clearOutEvents(handle);

	/* Default init sequence for statechart firmware */
	handle->ifaceCar.brakes_on = bool_false;

}

void firmware_enter(Firmware* handle)
{
	/* Default enter sequence for statechart firmware */
	firmware_enseq_main_region_default(handle);
}

void firmware_exit(Firmware* handle)
{
	/* Default exit sequence for statechart firmware */
	firmware_exseq_main_region(handle);
}

sc_boolean firmware_isActive(const Firmware* handle)
{
	sc_boolean result;
	if (handle->stateConfVector[0] != Firmware_last_state || handle->stateConfVector[1] != Firmware_last_state)
	{
		result =  bool_true;
	}
	else
	{
		result = bool_false;
	}
	return result;
}

/* 
 * Always returns 'false' since this state machine can never become final.
 */
sc_boolean firmware_isFinal(const Firmware* handle)
{
   return bool_false;
}

static void firmware_clearInEvents(Firmware* handle)
{
	handle->ifaceMCU.error_raised = bool_false;
	handle->ifaceHMI.button_click_raised = bool_false;
	handle->ifaceHMI.button_long_press_raised = bool_false;
}

static void firmware_clearOutEvents(Firmware* handle)
{
}

void firmware_runCycle(Firmware* handle)
{
	
	firmware_clearOutEvents(handle);
	
	for (handle->stateConfVectorPosition = 0;
		handle->stateConfVectorPosition < FIRMWARE_MAX_ORTHOGONAL_STATES;
		handle->stateConfVectorPosition++)
		{
			
		switch (handle->stateConfVector[handle->stateConfVectorPosition])
		{
		case Firmware_main_region_idle :
		{
			firmware_react_main_region_idle(handle);
			break;
		}
		case Firmware_main_region_running_headlights_on :
		{
			firmware_react_main_region_running_headlights_on(handle);
			break;
		}
		case Firmware_main_region_running_headlights_off :
		{
			firmware_react_main_region_running_headlights_off(handle);
			break;
		}
		case Firmware_main_region_running_taillights_brakes_on :
		{
			firmware_react_main_region_running_taillights_brakes_on(handle);
			break;
		}
		case Firmware_main_region_running_taillights_brakes_off :
		{
			firmware_react_main_region_running_taillights_brakes_off(handle);
			break;
		}
		case Firmware_main_region_error :
		{
			firmware_react_main_region_error(handle);
			break;
		}
		default:
			break;
		}
	}
	
	firmware_clearInEvents(handle);
}


sc_boolean firmware_isStateActive(const Firmware* handle, FirmwareStates state)
{
	sc_boolean result = bool_false;
	switch (state)
	{
		case Firmware_main_region_idle :
			result = (sc_boolean) (handle->stateConfVector[0] == Firmware_main_region_idle
			);
			break;
		case Firmware_main_region_running :
			result = (sc_boolean) (handle->stateConfVector[0] >= Firmware_main_region_running
				&& handle->stateConfVector[0] <= Firmware_main_region_running_taillights_brakes_off);
			break;
		case Firmware_main_region_running_headlights_on :
			result = (sc_boolean) (handle->stateConfVector[0] == Firmware_main_region_running_headlights_on
			);
			break;
		case Firmware_main_region_running_headlights_off :
			result = (sc_boolean) (handle->stateConfVector[0] == Firmware_main_region_running_headlights_off
			);
			break;
		case Firmware_main_region_running_taillights_brakes_on :
			result = (sc_boolean) (handle->stateConfVector[1] == Firmware_main_region_running_taillights_brakes_on
			);
			break;
		case Firmware_main_region_running_taillights_brakes_off :
			result = (sc_boolean) (handle->stateConfVector[1] == Firmware_main_region_running_taillights_brakes_off
			);
			break;
		case Firmware_main_region_error :
			result = (sc_boolean) (handle->stateConfVector[0] == Firmware_main_region_error
			);
			break;
		default:
			result = bool_false;
			break;
	}
	return result;
}



sc_boolean firmwareIfaceCar_get_brakes_on(const Firmware* handle)
{
	return handle->ifaceCar.brakes_on;
}
void firmwareIfaceCar_set_brakes_on(Firmware* handle, sc_boolean value)
{
	handle->ifaceCar.brakes_on = value;
}
void firmwareIfaceMCU_raise_error(Firmware* handle)
{
	handle->ifaceMCU.error_raised = bool_true;
}






void firmwareIfaceHMI_raise_button_click(Firmware* handle)
{
	handle->ifaceHMI.button_click_raised = bool_true;
}
void firmwareIfaceHMI_raise_button_long_press(Firmware* handle)
{
	handle->ifaceHMI.button_long_press_raised = bool_true;
}



/* implementations of all internal functions */

static sc_boolean firmware_check_main_region_idle_tr0_tr0(const Firmware* handle)
{
	return handle->ifaceHMI.button_click_raised;
}

static sc_boolean firmware_check_main_region_idle_lr0_lr0(const Firmware* handle)
{
	return bool_true;
}

static sc_boolean firmware_check_main_region_running_tr0_tr0(const Firmware* handle)
{
	return handle->ifaceHMI.button_long_press_raised;
}

static sc_boolean firmware_check_main_region_running_tr1_tr1(const Firmware* handle)
{
	return handle->ifaceMCU.error_raised;
}

static sc_boolean firmware_check_main_region_running_lr1_lr1(const Firmware* handle)
{
	return bool_true;
}

static sc_boolean firmware_check_main_region_running_headlights_on_tr0_tr0(const Firmware* handle)
{
	return handle->ifaceHMI.button_click_raised;
}

static sc_boolean firmware_check_main_region_running_headlights_off_tr0_tr0(const Firmware* handle)
{
	return handle->ifaceHMI.button_click_raised;
}

static sc_boolean firmware_check_main_region_running_taillights_brakes_on_tr0_tr0(const Firmware* handle)
{
	return (!handle->ifaceCar.brakes_on) ? bool_true : bool_false;
}

static sc_boolean firmware_check_main_region_running_taillights_brakes_off_tr0_tr0(const Firmware* handle)
{
	return handle->ifaceCar.brakes_on;
}

static sc_boolean firmware_check_main_region_error_tr0_tr0(const Firmware* handle)
{
	return bool_true;
}

static void firmware_effect_main_region_idle_tr0(Firmware* handle)
{
	firmware_exseq_main_region_idle(handle);
	firmware_enseq_main_region_running_default(handle);
}

static void firmware_effect_main_region_idle_lr0_lr0(Firmware* handle)
{
	firmwareIfaceMCU_wait_for_interrupt(handle);
}

static void firmware_effect_main_region_running_tr0(Firmware* handle)
{
	firmware_exseq_main_region_running(handle);
	firmware_enseq_main_region_idle_default(handle);
}

static void firmware_effect_main_region_running_tr1(Firmware* handle)
{
	firmware_exseq_main_region_running(handle);
	firmware_enseq_main_region_error_default(handle);
}

static void firmware_effect_main_region_running_lr1_lr1(Firmware* handle)
{
	firmwareIfaceMCU_wait_for_interrupt(handle);
}

static void firmware_effect_main_region_running_headlights_on_tr0(Firmware* handle)
{
	firmware_exseq_main_region_running_headlights_on(handle);
	firmware_enseq_main_region_running_headlights_off_default(handle);
}

static void firmware_effect_main_region_running_headlights_off_tr0(Firmware* handle)
{
	firmware_exseq_main_region_running_headlights_off(handle);
	firmware_enseq_main_region_running_headlights_on_default(handle);
}

static void firmware_effect_main_region_running_taillights_brakes_on_tr0(Firmware* handle)
{
	firmware_exseq_main_region_running_taillights_brakes_on(handle);
	firmware_enseq_main_region_running_taillights_brakes_off_default(handle);
}

static void firmware_effect_main_region_running_taillights_brakes_off_tr0(Firmware* handle)
{
	firmware_exseq_main_region_running_taillights_brakes_off(handle);
	firmware_enseq_main_region_running_taillights_brakes_on_default(handle);
}

static void firmware_effect_main_region_error_tr0(Firmware* handle)
{
	firmware_exseq_main_region_error(handle);
	firmware_enseq_main_region_idle_default(handle);
}

/* Entry action for state 'running'. */
static void firmware_enact_main_region_running(Firmware* handle)
{
	/* Entry action for state 'running'. */
	firmwareIfaceCar_start_car(handle);
}

/* Entry action for state 'on'. */
static void firmware_enact_main_region_running_headlights_on(Firmware* handle)
{
	/* Entry action for state 'on'. */
	firmwareIfaceHeadLights_on(handle);
}

/* Entry action for state 'off'. */
static void firmware_enact_main_region_running_headlights_off(Firmware* handle)
{
	/* Entry action for state 'off'. */
	firmwareIfaceHeadLights_off(handle);
}

/* Entry action for state 'brakes_on'. */
static void firmware_enact_main_region_running_taillights_brakes_on(Firmware* handle)
{
	/* Entry action for state 'brakes_on'. */
	firmwareIfaceTailLights_bright(handle);
}

/* Entry action for state 'brakes_off'. */
static void firmware_enact_main_region_running_taillights_brakes_off(Firmware* handle)
{
	/* Entry action for state 'brakes_off'. */
	firmwareIfaceTailLights_dim(handle);
}

/* Entry action for state 'error'. */
static void firmware_enact_main_region_error(Firmware* handle)
{
	/* Entry action for state 'error'. */
	firmwareIfaceMCU_handle_error(handle);
}

/* Exit action for state 'running'. */
static void firmware_exact_main_region_running(Firmware* handle)
{
	/* Exit action for state 'running'. */
	firmwareIfaceCar_stop_car(handle);
}

/* 'default' enter sequence for state idle */
static void firmware_enseq_main_region_idle_default(Firmware* handle)
{
	/* 'default' enter sequence for state idle */
	handle->stateConfVector[0] = Firmware_main_region_idle;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for state running */
static void firmware_enseq_main_region_running_default(Firmware* handle)
{
	/* 'default' enter sequence for state running */
	firmware_enact_main_region_running(handle);
	firmware_enseq_main_region_running_headlights_default(handle);
	firmware_enseq_main_region_running_taillights_default(handle);
}

/* 'default' enter sequence for state on */
static void firmware_enseq_main_region_running_headlights_on_default(Firmware* handle)
{
	/* 'default' enter sequence for state on */
	firmware_enact_main_region_running_headlights_on(handle);
	handle->stateConfVector[0] = Firmware_main_region_running_headlights_on;
	handle->stateConfVectorPosition = 0;
	handle->historyVector[0] = handle->stateConfVector[0];
}

/* 'default' enter sequence for state off */
static void firmware_enseq_main_region_running_headlights_off_default(Firmware* handle)
{
	/* 'default' enter sequence for state off */
	firmware_enact_main_region_running_headlights_off(handle);
	handle->stateConfVector[0] = Firmware_main_region_running_headlights_off;
	handle->stateConfVectorPosition = 0;
	handle->historyVector[0] = handle->stateConfVector[0];
}

/* 'default' enter sequence for state brakes_on */
static void firmware_enseq_main_region_running_taillights_brakes_on_default(Firmware* handle)
{
	/* 'default' enter sequence for state brakes_on */
	firmware_enact_main_region_running_taillights_brakes_on(handle);
	handle->stateConfVector[1] = Firmware_main_region_running_taillights_brakes_on;
	handle->stateConfVectorPosition = 1;
}

/* 'default' enter sequence for state brakes_off */
static void firmware_enseq_main_region_running_taillights_brakes_off_default(Firmware* handle)
{
	/* 'default' enter sequence for state brakes_off */
	firmware_enact_main_region_running_taillights_brakes_off(handle);
	handle->stateConfVector[1] = Firmware_main_region_running_taillights_brakes_off;
	handle->stateConfVectorPosition = 1;
}

/* 'default' enter sequence for state error */
static void firmware_enseq_main_region_error_default(Firmware* handle)
{
	/* 'default' enter sequence for state error */
	firmware_enact_main_region_error(handle);
	handle->stateConfVector[0] = Firmware_main_region_error;
	handle->stateConfVectorPosition = 0;
}

/* 'default' enter sequence for region main region */
static void firmware_enseq_main_region_default(Firmware* handle)
{
	/* 'default' enter sequence for region main region */
	firmware_react_main_region__entry_Default(handle);
}

/* 'default' enter sequence for region headlights */
static void firmware_enseq_main_region_running_headlights_default(Firmware* handle)
{
	/* 'default' enter sequence for region headlights */
	firmware_react_main_region_running_headlights__entry_Default(handle);
}

/* shallow enterSequence with history in child headlights */
static void firmware_shenseq_main_region_running_headlights(Firmware* handle)
{
	/* shallow enterSequence with history in child headlights */
	/* Handle shallow history entry of headlights */
	switch(handle->historyVector[ 0 ])
	{
		case Firmware_main_region_running_headlights_on :
		{
			firmware_enseq_main_region_running_headlights_on_default(handle);
			break;
		}
		case Firmware_main_region_running_headlights_off :
		{
			firmware_enseq_main_region_running_headlights_off_default(handle);
			break;
		}
		default: break;
	}
}

/* 'default' enter sequence for region taillights */
static void firmware_enseq_main_region_running_taillights_default(Firmware* handle)
{
	/* 'default' enter sequence for region taillights */
	firmware_react_main_region_running_taillights__entry_Default(handle);
}

/* Default exit sequence for state idle */
static void firmware_exseq_main_region_idle(Firmware* handle)
{
	/* Default exit sequence for state idle */
	handle->stateConfVector[0] = Firmware_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state running */
static void firmware_exseq_main_region_running(Firmware* handle)
{
	/* Default exit sequence for state running */
	firmware_exseq_main_region_running_headlights(handle);
	firmware_exseq_main_region_running_taillights(handle);
	firmware_exact_main_region_running(handle);
}

/* Default exit sequence for state on */
static void firmware_exseq_main_region_running_headlights_on(Firmware* handle)
{
	/* Default exit sequence for state on */
	handle->stateConfVector[0] = Firmware_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state off */
static void firmware_exseq_main_region_running_headlights_off(Firmware* handle)
{
	/* Default exit sequence for state off */
	handle->stateConfVector[0] = Firmware_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for state brakes_on */
static void firmware_exseq_main_region_running_taillights_brakes_on(Firmware* handle)
{
	/* Default exit sequence for state brakes_on */
	handle->stateConfVector[1] = Firmware_last_state;
	handle->stateConfVectorPosition = 1;
}

/* Default exit sequence for state brakes_off */
static void firmware_exseq_main_region_running_taillights_brakes_off(Firmware* handle)
{
	/* Default exit sequence for state brakes_off */
	handle->stateConfVector[1] = Firmware_last_state;
	handle->stateConfVectorPosition = 1;
}

/* Default exit sequence for state error */
static void firmware_exseq_main_region_error(Firmware* handle)
{
	/* Default exit sequence for state error */
	handle->stateConfVector[0] = Firmware_last_state;
	handle->stateConfVectorPosition = 0;
}

/* Default exit sequence for region main region */
static void firmware_exseq_main_region(Firmware* handle)
{
	/* Default exit sequence for region main region */
	/* Handle exit of all possible states (of firmware.main_region) at position 0... */
	switch(handle->stateConfVector[ 0 ])
	{
		case Firmware_main_region_idle :
		{
			firmware_exseq_main_region_idle(handle);
			break;
		}
		case Firmware_main_region_running_headlights_on :
		{
			firmware_exseq_main_region_running_headlights_on(handle);
			break;
		}
		case Firmware_main_region_running_headlights_off :
		{
			firmware_exseq_main_region_running_headlights_off(handle);
			break;
		}
		case Firmware_main_region_error :
		{
			firmware_exseq_main_region_error(handle);
			break;
		}
		default: break;
	}
	/* Handle exit of all possible states (of firmware.main_region) at position 1... */
	switch(handle->stateConfVector[ 1 ])
	{
		case Firmware_main_region_running_taillights_brakes_on :
		{
			firmware_exseq_main_region_running_taillights_brakes_on(handle);
			firmware_exact_main_region_running(handle);
			break;
		}
		case Firmware_main_region_running_taillights_brakes_off :
		{
			firmware_exseq_main_region_running_taillights_brakes_off(handle);
			firmware_exact_main_region_running(handle);
			break;
		}
		default: break;
	}
}

/* Default exit sequence for region headlights */
static void firmware_exseq_main_region_running_headlights(Firmware* handle)
{
	/* Default exit sequence for region headlights */
	/* Handle exit of all possible states (of firmware.main_region.running.headlights) at position 0... */
	switch(handle->stateConfVector[ 0 ])
	{
		case Firmware_main_region_running_headlights_on :
		{
			firmware_exseq_main_region_running_headlights_on(handle);
			break;
		}
		case Firmware_main_region_running_headlights_off :
		{
			firmware_exseq_main_region_running_headlights_off(handle);
			break;
		}
		default: break;
	}
}

/* Default exit sequence for region taillights */
static void firmware_exseq_main_region_running_taillights(Firmware* handle)
{
	/* Default exit sequence for region taillights */
	/* Handle exit of all possible states (of firmware.main_region.running.taillights) at position 1... */
	switch(handle->stateConfVector[ 1 ])
	{
		case Firmware_main_region_running_taillights_brakes_on :
		{
			firmware_exseq_main_region_running_taillights_brakes_on(handle);
			break;
		}
		case Firmware_main_region_running_taillights_brakes_off :
		{
			firmware_exseq_main_region_running_taillights_brakes_off(handle);
			break;
		}
		default: break;
	}
}

/* The reactions of state idle. */
static void firmware_react_main_region_idle(Firmware* handle)
{
	/* The reactions of state idle. */
	if (firmware_check_main_region_idle_tr0_tr0(handle) == bool_true)
	{ 
		firmware_effect_main_region_idle_tr0(handle);
	}  else
	{
		firmware_effect_main_region_idle_lr0_lr0(handle);
	}
}

/* The reactions of state on. */
static void firmware_react_main_region_running_headlights_on(Firmware* handle)
{
	/* The reactions of state on. */
	if (firmware_check_main_region_running_tr0_tr0(handle) == bool_true)
	{ 
		firmware_effect_main_region_running_tr0(handle);
	}  else
	{
		if (firmware_check_main_region_running_tr1_tr1(handle) == bool_true)
		{ 
			firmware_effect_main_region_running_tr1(handle);
		}  else
		{
			firmware_effect_main_region_running_lr1_lr1(handle);
			if (firmware_check_main_region_running_headlights_on_tr0_tr0(handle) == bool_true)
			{ 
				firmware_effect_main_region_running_headlights_on_tr0(handle);
			} 
		}
	}
}

/* The reactions of state off. */
static void firmware_react_main_region_running_headlights_off(Firmware* handle)
{
	/* The reactions of state off. */
	if (firmware_check_main_region_running_tr0_tr0(handle) == bool_true)
	{ 
		firmware_effect_main_region_running_tr0(handle);
	}  else
	{
		if (firmware_check_main_region_running_tr1_tr1(handle) == bool_true)
		{ 
			firmware_effect_main_region_running_tr1(handle);
		}  else
		{
			firmware_effect_main_region_running_lr1_lr1(handle);
			if (firmware_check_main_region_running_headlights_off_tr0_tr0(handle) == bool_true)
			{ 
				firmware_effect_main_region_running_headlights_off_tr0(handle);
			} 
		}
	}
}

/* The reactions of state brakes_on. */
static void firmware_react_main_region_running_taillights_brakes_on(Firmware* handle)
{
	/* The reactions of state brakes_on. */
	if (firmware_check_main_region_running_taillights_brakes_on_tr0_tr0(handle) == bool_true)
	{ 
		firmware_effect_main_region_running_taillights_brakes_on_tr0(handle);
	} 
}

/* The reactions of state brakes_off. */
static void firmware_react_main_region_running_taillights_brakes_off(Firmware* handle)
{
	/* The reactions of state brakes_off. */
	if (firmware_check_main_region_running_taillights_brakes_off_tr0_tr0(handle) == bool_true)
	{ 
		firmware_effect_main_region_running_taillights_brakes_off_tr0(handle);
	} 
}

/* The reactions of state error. */
static void firmware_react_main_region_error(Firmware* handle)
{
	/* The reactions of state error. */
	firmware_effect_main_region_error_tr0(handle);
}

/* Default react sequence for initial entry  */
static void firmware_react_main_region__entry_Default(Firmware* handle)
{
	/* Default react sequence for initial entry  */
	firmware_enseq_main_region_idle_default(handle);
}

/* Default react sequence for shallow history entry  */
static void firmware_react_main_region_running_headlights__entry_Default(Firmware* handle)
{
	/* Default react sequence for shallow history entry  */
	/* Enter the region with shallow history */
	if (handle->historyVector[0] != Firmware_last_state)
	{
		firmware_shenseq_main_region_running_headlights(handle);
	} else
	{
		firmware_enseq_main_region_running_headlights_on_default(handle);
	} 
}

/* Default react sequence for initial entry  */
static void firmware_react_main_region_running_taillights__entry_Default(Firmware* handle)
{
	/* Default react sequence for initial entry  */
	firmware_enseq_main_region_running_taillights_brakes_off_default(handle);
}


