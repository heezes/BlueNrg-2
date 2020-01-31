/*
 * app_vim.h
 *
 *  Created on: Dec 18, 2019
 *      Author: Altamash
 */

#ifndef INC_APP_VIM_H_
#define INC_APP_VIM_H_

#ifdef VIM

#include "ble_service_bms.h"
#include "stdio.h"
#include "MCAL.h"

int _write(int fd, char *str, int len);
void App_Init(void);
void App_Process(void);

#define AUTH_OPCODE					0x01
#define RIDE_RESUME_REQ_OPCODE		0x05
#define LOCK_REQ_OPCODE				0x06
#define VEHICLE_STATE_OPCODE		0x07

#define POSITIVE_ACK				0x01
#define NEGATIVE_ACK				0x02
#define CANNOT_REPLY				0x03
#define LOCK_REQ_DENIED				0x04
#define VEHICLE_LOCKED				0x05
#define VEHICLE_UNLOCKED			0x06
#define VEHICLE_PAUSED				0x07

/*BLE State*/
typedef enum
{
	BLE_ADVERTISING = 0,
	BLE_CONNECT_NOTIFY,
	BLE_AUTHENTICATION_NOTIFY,
	BLE_CONNECTED,
	BLE_LOCK_NOTIFY,
	BLE_RIDE_RESUME_NOTIFY,
	BLE_DISCONNECT_NOTIFY,
	BLE_DISCONNECTED,
	BLE_DATA_REQ
}bleStateHandleTypeDef_t;

typedef struct
{
	uint8_t cmdOpcode;
	uint8_t responseOpcode;
	uint8_t vehicleState;
	bleStateHandleTypeDef_t bleState;
}BleHandleTypeDef_t;


#endif
#endif /* INC_APP_VIM_H_ */
