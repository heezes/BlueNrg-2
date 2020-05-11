/*
 * gatt_db_vim.h
 *
 *  Created on: Dec 18, 2019
 *      Author: Altamash
 */

#ifndef INC_GATT_DB_VIM_H_
#define INC_GATT_DB_VIM_H_



#ifdef VIM
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h"
#include "bluenrg1_stack.h"
#include "osal.h"
#include "stddef.h"
#include "ble_service_vim.h"

/*PUBLIC DEFINES*/
/* Discoverable */
extern uint8_t g_advData[12];

/***********************************************************************************************************************/

/*Roles*/
#define Peripheral		0x01
#define Broadcaster		0x02
#define Central			0x04
#define Observer		0x08

/*Privacy*/
#define Disabled			0x00
#define Host_Enabled		0x01
#define Controller_Enabled 	0x02

/*Service Index defines*/
#define  DATA_SERVICE		  0
/*Characteristic Index Defines*/
/*DATA*/
#define  COMMAND		0 /*Characteristic idx for commands*/
#define  RESPONSE		1 /*Characteristic idx for response*/
#define  DATA   		2 /*Characteristic idx for data*/


/*DATA CHARACTERISTIC LEN*/
#define COMMAND_LEN 		128
#define RESPONSE_LEN		128
#define DATA_LEN			128

#define MAX_TOKEN_SIZE		4
/***********************************************************************************************************************/

/*Service Handles*/
extern uint16_t g_ServiceHandle[1];

/*Characteristic Handles*/
extern uint16_t g_DataGroup[3];

extern volatile uint8_t g_deviceState;
extern volatile uint16_t g_connectionHandle;
extern volatile uint8_t g_sentAuthToken[MAX_TOKEN_SIZE];
extern volatile int g_peerAuthenticated;


int Device_Init(uint8_t* deviceID);
void SetConnectable(uint8_t* deviceId);



#endif

#endif /* INC_GATT_DB_VIM_H_ */
