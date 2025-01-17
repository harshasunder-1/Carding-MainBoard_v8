/*
 * SettingsState.c
 *
 *  Created on: Mar 10, 2023
 *      Author: harsha
 */

#include "Struct.h"
#include "BT_Machine.h"
#include "MachineErrors.h"
#include "CommonConstants.h"
#include "stm32g4xx_hal.h"
#include "StateMachine.h"

extern UART_HandleTypeDef huart1;
extern userSettings u;
extern userSettings uBT;

void SettingsState(void){

	uint8_t ee_success = 0;

	if (BT.information == REQ_SETTINGS_FROM_APP){
		  uint8_t packetSize = BT_MC_generateSettingsMsg(&u);
		  while(S.BT_transmission_over != 1){};
		  HAL_UART_Transmit_IT(&huart1,(uint8_t*)BufferTransmit,packetSize);
		  S.BT_transmission_over = 0;
	}

	if (BT.information == SETTINGS_FROM_APP){
	  if (S.BT_dataOK){
		  if (BT.subState == SAVE_CARDING_SETTINGS){ //remove this later.
			  //only write into eeprom when the prev state is IDLE, not during RUN
			  if (S.prev_state == RUN_STATE){
				  BT_MC_Update_Settings(&u,&uBT);
				  HAL_UART_Transmit_IT(&huart1,(uint8_t*)SAVINGSUCCESS,6);
				  S.BT_transmission_over = 1;
				  S.settingsModified = 1;
			  }
			  else if (S.prev_state == IDLE_STATE){
				  ee_success = BT_MC_Save_Settings(&uBT);
				  if (ee_success){
					  BT_MC_Update_Settings(&u,&uBT);
					  // send success msg to App
					  HAL_UART_Transmit_IT(&huart1,(uint8_t*)SAVINGSUCCESS,6);
					  S.BT_transmission_over = 1;
					  S.settingsModified = 1;
				  }else{
					  MBE.AppSettings_eepromWriteFailure += 1;
					  HAL_UART_Transmit_IT(&huart1,(uint8_t*)SAVINGFAILURE,6);
					  S.BT_transmission_over = 1;
				  }
			  }else {}
		  S.BT_dataOK = 0;
		  }// closes save Carding Settings
	  }else{ // data not OK
		  S.BT_transmission_over = 0;
		  MBE.AppSettings_parsingFailure += 1;
		  HAL_UART_Transmit_IT(&huart1,(uint8_t*)SAVINGFAILURE,6);
	  }
	}
	  /* go back to the idle or run state. the new values get integrated into the
	   * machine parameters struct in the idle state when you first enter it
	   */
	  ChangeState(&S,S.prev_state);
	  if (S.current_state == RUN_STATE){ // if you go back to run state, then dont send the rampUp msg to the cylinders.
		  S.oneTime = 0;
	  }
}
