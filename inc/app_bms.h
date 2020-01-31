/*
 * App.h
 *
 *  Created on: 26-Feb-2019
 *      Author: user
 */

#ifndef APP_H_
#define APP_H_

#ifdef BMS

#include "ble_service_bms.h"
#include "stdio.h"
#include "MCAL.h"

int _write(int fd, char *str, int len);
void App_Init(void);
void App_Process(void);

/*Private Define*/
#define AUTH_TOKEN	1235
#define PRIME_NO_1	11
#define PRIME_NO_2	7919

/*BLE State*/
#define BLE_ADVERTISING             0
#define BLE_CONNECT_NOTIFY			1
#define BLE_CONNECTED				2
#define BLE_DISCONNECT_NOTIFY		3
#define BLE_DISCONNECTED			4
#define BLE_DIAGNOSTIC_REQ			5

#endif
#endif /* APP_H_ */
