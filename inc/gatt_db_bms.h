

#ifndef _GATT_DB_H_
#define _GATT_DB_H_
#ifdef BMS
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h"
#include "bluenrg1_stack.h"
#include "osal.h"
#include "stddef.h"
#include "ble_service_bms.h"

/*PUBLIC DEFINES*/
/* Discoverable */
#define TEMP_OFFSET 8
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
#define  DIAGNOSTIC_SERVICE	  1
/*Characteristic Index Defines*/
/*DATA*/
#define  SOC_CUR_TEMP		  0 /*Contains in order SOC, SOH, SOF, Current, Temperature*/
#define  BATT_VOLT			  1 /*Contains Battery Voltage of 1 Byte size*/
#define  CELL_BAL_MOSFET_OC   2 /*Contains Cell balancing status and Mosfet open close status*/
#define  RANGE				  3 /*Contains Range*/
#define  BATTERY_ID			  4 /*Contains Battery ID*/
#define  BMS_VERS_CONFIG	  5 /*Contains BMS version and Series and Parallel Configuration*/
#define  FW_VERSION			  6 /*Contains firmware version*/
#define  AUTH_ACK_REQ	      7 /*CONTAINS AUTHENTICATION and ACKNOWLEDGGE DATA*/

/*DIAGNOSTIC*/
#define  BMS_DIAGNOSTIC		  0 /*Contains the complete BmsDiagnostic_t structure*/
#define  CAPACITY_CYCLE_WARR  1 /*Contains Design, Full Charge, Remaining Capacity, Cycle Count & Warranty*/
#define  INSTANCE_EVENT_COUNT 2 /*Contains FullChargeInstancesCount, FullDishargeInstancesCount,
								  ShortCircuitEventsCount, OverTemperatureEventsCount, UnderTemperatureEventsCount,
								  OverCurrentEventsCount, KeyOnEventsCount*/
#define  AH_MAX_MIN_VOLT	  3 /*Contains CumulativeAhSpent, MinimumCellVoltageEver, MaximumCellVoltageEver*/

/*DATA CHARACTERISTIC LEN*/
#define TEMP_SENSOR_LEN 			2*THERM_COUNT
#define SOC_CUR_TEMP_LEN			(7+(TEMP_SENSOR_LEN))
#define BATT_VOLT_LEN				CELL_COUNT
#define CELL_BAL_MOSFET_OC_LEN		3
#define RANGE_LEN					2
#define BATTERY_ID_LEN				20 /*20 Bytes max characteristic len*/
#define BMS_VERS_CONFIG_LEN			3
#define FW_VERSION_LEN				20 /*20 Bytes max characteristic len*/
#define AUTH_ACK_LEN				5

/*DIAGNOSTIC CHARACTERISTIC LEN*/
#define BMS_DIAGNOSTIC_LEN			11
#define CAPACITY_CYCLE_WARR_LEN		9
#define INSTANCE_EVENT_COUNT_LEN	14
#define AH_MAX_MIN_VOLT_LEN			8

/***********************************************************************************************************************/

/*Service Handles*/
extern uint16_t g_ServiceHandle[2];

/*Characteristic Handles*/
extern uint16_t g_DataGroup[8];
extern uint16_t g_DiagnosticGroup[4];

extern volatile uint8_t g_deviceState;
extern volatile uint16_t g_connectionHandle;
extern volatile int g_sentAuthToken;
extern volatile int g_peerAuthenticated;


int Device_Init(void);
void SetConnectable(void);
#endif
#endif /* _GATT_DB_H_ */
