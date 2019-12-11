/*
 * App.c
 *
 *  Created on: 26-Feb-2019
 *      Author: user
 */

#include "App.h"
#include "BlueNRG1_it.h"
#include "BlueNRG1_conf.h"
#include "ble_const.h"
#include "bluenrg1_stack.h"
#include "osal.h"
#include "gp_timer.h"
#include "rsa_ed.h"
#include "string.h"
#include "math.h"


/* Discoverable */
#define TEMP_OFFSET 8
uint8_t g_advData[] = {0x02,0x01,0x06,0x06,AD_TYPE_MANUFACTURER_SPECIFIC_DATA,0x30,0x00,0x05,0xFF,0xFF,0xFF,0xFF};


/*Private Defines*/
#define  UPDATE_CONN_PARAM 	  0
/*Timer Defines*/
#define  CONN_AUTH_TIMER      0
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

/*Private TypeDef*/
struct timer g_SoftTim[2];

volatile uint8_t g_deviceState = 0;
volatile uint16_t g_connectionHandle = 0;
volatile int g_sentAuthToken = 0;
volatile int g_peerAuthenticated = 0;

/* Private function prototypes */
#if BLE_DEBUG
int _write(int fd, char *str, int len)
{return	(COM_SendBlocking((uint8_t*)str,len,10000)==COM_AWS_SUCCESS)?(len):(-1);}
#endif

/*Service Handles*/
uint16_t g_ServiceHandle[2];

/*Characteristic Handles*/
uint16_t g_DataGroup[8] =  {0};
uint16_t g_DiagnosticGroup[4] = {0};

/*
 * Function Prototypes
 * */
void convertVoltages(uint16_t *inVoltage, uint8_t *outVoltage)
{
	for(int i = 0; i<CELL_COUNT; i++)
	{
		float voltage = (inVoltage[i]/10.0);
		outVoltage[i] = (uint8_t)(voltage - 200U);
	}
}

/**************************************************************************************************************************************/

static int AddService(void)
{
	  tBleStatus ret;
	  /* UUIDs */
	  Service_UUID_t service_uuid;
	  Char_UUID_t charUUID;
	  /*
	  UUIDs:
	 ->Data Service:
	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75cba

		-SOC+SOH+SOF+CURRENT_TEMPERATURE
		  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75dba
		-BATTERY VOLTAGE
		  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75eba
		-CELL BALANCING STATUS+MOSFET OPEN/CLOSE
		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75fba
		-RANGE
 		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f76aba
 		-BATTERY ID
 		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f76bba
 		-BMS VERSION+SERIES PARALLEL CONFIGURATION
 		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f76cba
 		-FIRMWARE VERSION
 		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f76dba
 		-AUTHENTICATE ACKNOWLEDGE-REQUEST
 		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f76eba
	 ->Diagnostic Service:
 	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f85cba
		-BMS DIAGNOSTIC
 	  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f85cba
 	  	-DESIGN, FULL CHARGE, REMAINING CAPACITY+ CYCLE COUNT+WARRANTY
 	  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f85eba
 	  	-FULL CHARGE, FULL DISCHARGE INSTANCE COUNT+SHORT CIRCUIT, OVER TEMPERATURE, UNDER TEMPERATURE, OVER CURRENT, KEY ON EVENT COUNT
 	  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f85fba
 	  	- CUMULATIVE AH SPENT+MIN CELL VOLTAGE EVER+MAXIMUM CELL VOLTAGE EVER
 	  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f86aba
	  */
      /****************************************************************************************************************************/
	  uint8_t DataService_UUID_128[16]			= {0xba,0x5c,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed};
	  uint8_t DataGroup[][16] =
	  {
			  {0xba,0x5d,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5e,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5f,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6a,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6b,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6c,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6d,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6e,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed}
	  };

      /****************************************************************************************************************************/
      uint8_t DiagnosticService_UUID_128[16] 		= {0xba,0x5c,0xf8,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed};
      uint8_t DiagnosticGroup[][16] =
      {
    		  {0xba,0x5d,0xf8,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5e,0xf8,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5f,0xf8,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x6a,0xf8,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed}
      };

      /****************************************************************************************************************************/
/*
	  The Max_Attribute_Records parameter specifies the maximum number of attribute records that can be added to this service (including the service attribute, include attribute, characteristic attribute,characteristic value attribute and characteristic descriptor attribute).
	  You have to considered that each characteristic uses at least 2 attributes (declaration and value), plus other attributes for each included descriptor.
	  For example, if the characteristic has the notify property then you have to considered +3 attributes for this characteristic.
	  Therefore, if you add a service with two characteristics with Notification, you have to considered 2*3 attributes +1 for the service, in total the Max_Attribute_Records should be equal to 7.
*/

      /***************************************************************************************************************************************************************************************************/
      Osal_MemCpy(&service_uuid.Service_UUID_128, DataService_UUID_128, 16);
	  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 26, &g_ServiceHandle[DATA_SERVICE]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[SOC_CUR_TEMP], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, SOC_CUR_TEMP_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,16, 0, &g_DataGroup[SOC_CUR_TEMP]);
	  if (ret != BLE_STATUS_AWS_SUCCESS){ ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[BATT_VOLT], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, BATT_VOLT_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[BATT_VOLT]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[CELL_BAL_MOSFET_OC][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, CELL_BAL_MOSFET_OC_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[CELL_BAL_MOSFET_OC]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[RANGE][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, RANGE_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[RANGE]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[BATTERY_ID][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, BATTERY_ID_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[BATTERY_ID]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[BMS_VERS_CONFIG][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, BMS_VERS_CONFIG_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[BMS_VERS_CONFIG]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[FW_VERSION][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, FW_VERSION_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DataGroup[FW_VERSION]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[AUTH_ACK_REQ][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, AUTH_ACK_LEN, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 1, &g_DataGroup[AUTH_ACK_REQ]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}
	  /****************************************************************************************************************************************************/

	  Osal_MemCpy(&service_uuid.Service_UUID_128, DiagnosticService_UUID_128, 16);
	  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 13, &g_ServiceHandle[DIAGNOSTIC_SERVICE]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DiagnosticGroup[BMS_DIAGNOSTIC][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DIAGNOSTIC_SERVICE], UUID_TYPE_128, &charUUID, BMS_DIAGNOSTIC_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &g_DiagnosticGroup[BMS_DIAGNOSTIC]);
	  if (ret != BLE_STATUS_AWS_SUCCESS){ ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DiagnosticGroup[CAPACITY_CYCLE_WARR][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DIAGNOSTIC_SERVICE], UUID_TYPE_128, &charUUID, CAPACITY_CYCLE_WARR_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DiagnosticGroup[CAPACITY_CYCLE_WARR]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DiagnosticGroup[INSTANCE_EVENT_COUNT][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DIAGNOSTIC_SERVICE], UUID_TYPE_128, &charUUID, INSTANCE_EVENT_COUNT_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DiagnosticGroup[INSTANCE_EVENT_COUNT]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DiagnosticGroup[AH_MAX_MIN_VOLT][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DIAGNOSTIC_SERVICE], UUID_TYPE_128, &charUUID, AH_MAX_MIN_VOLT_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_DONT_NOTIFY_EVENTS, 16, 0, &g_DiagnosticGroup[AH_MAX_MIN_VOLT]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}
	  /***************************************************************************************************************************************************/
#if BLE_DEBUG
	  iprintf("Service added.\n");
#endif
	  return BLE_STATUS_AWS_SUCCESS;

	fail:
#if BLE_DEBUG
	  iprintf("Error while adding RC service at Line: %d.\n", ret);
#endif
	  return BLE_STATUS_ERROR ;
}

static int Device_Init(void)
{
	int ret = 0;
	uint8_t ble_addr[] = {0x01, 0x00, 0x80, 0xE1, 0x80, 0x02};
	aci_hal_write_config_data(0,sizeof(ble_addr), ble_addr);
	if(ret != BLE_STATUS_AWS_SUCCESS){ret = __LINE__;goto error;}
    /* Set the TX power to 8 dBm */
	ret = aci_hal_set_tx_power_level(1, 0x07);
	if(ret == 0){
#if BLE_DEBUG
		iprintf("Power Setting SuccessFull\n");
#endif
	}
	ret = aci_gatt_init();
	if(ret != BLE_STATUS_AWS_SUCCESS){ret = __LINE__;goto error;}
	uint8_t DeviceName[] = "BMS";
	uint16_t Service_Handle, Dev_Name_Char_Handle, Appearance_Char_Handle;
	ret = aci_gap_init(Peripheral, Disabled, (uint8_t)strlen((char*)DeviceName), &Service_Handle, &Dev_Name_Char_Handle, &Appearance_Char_Handle);
	if(ret != BLE_STATUS_AWS_SUCCESS){ret = __LINE__;goto error;}
	ret = aci_gatt_update_char_value_ext(0,Service_Handle, Dev_Name_Char_Handle,0,(uint16_t)strlen((char*)DeviceName),0, (uint8_t)strlen((char*)DeviceName), DeviceName);
	if(ret != BLE_STATUS_AWS_SUCCESS){ret = __LINE__;goto error;}
	ret = AddService();
	if(ret != BLE_STATUS_AWS_SUCCESS){ret = __LINE__;goto error;}
	return ret;
error:
#if BLE_DEBUG
	iprintf("Device Init Failed at Line: %d",ret);
#endif
	return ret;
}

static void SetConnectable(void)
{
	  uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','M','S','-','6'};
	  hci_le_set_scan_response_data(0,NULL);
	  uint8_t rt = aci_gap_set_discoverable(ADV_IND, 0x0020, 0x0030,STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
	                                 sizeof(local_name), local_name, 0, NULL, 0x00, 0x00);
	  if(rt != BLE_STATUS_AWS_SUCCESS)
	  {
#if BLE_DEBUG
	    iprintf("aci_gap_set_discoverable() failed\r\n");
#endif
	   }
}

void App_Init(void)
{
	  LED_Init();
	  LED_On();
	  COM_Init();
#if BLE_DEBUG
	  iprintf("Starting!\r\n");
#endif
	  RSA_params_t rsaParam;
	  RSA_CryptoInit((RSA_params_t*)&rsaParam, PRIME_NO_1, PRIME_NO_2);
	  g_sentAuthToken = RSA_encryptData(&rsaParam, AUTH_TOKEN);
	  if(Device_Init() != BLE_STATUS_AWS_SUCCESS)
	  {
#if BLE_DEBUG
		  iprintf("App Init Failed at Line: %d\r\n", __LINE__);
#endif
		  while(1);
	  }
	  Timer_Set(&g_SoftTim[CONN_AUTH_TIMER], 20000);
#if BLE_DEBUG
	  iprintf("Encrypted Token:%d\n", g_sentAuthToken);
#endif
}

void App_Process(void)
{
	switch(g_deviceState)
	{
		case BLE_CONNECT_NOTIFY:
		{
			BLE_NotifyConnection(1);
			Timer_Restart(&g_SoftTim[CONN_AUTH_TIMER]);
			uint8_t temp[6] = {0};
			memcpy(temp, (int*)&g_sentAuthToken, 4);
			aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[AUTH_ACK_REQ], 0x01, AUTH_ACK_LEN, 0, sizeof(int), temp);
			aci_l2cap_connection_parameter_update_req(g_connectionHandle, 0x0006, 0x0006, 0, 150);
			__disable_irq();
			g_deviceState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		case BLE_CONNECTED:
		{
			BmsPackState_t 		BleBuff	= {0};
			BmsPackInfo_t  		PackInfo = {0};
			DiagnosticInfo_t	DiagInfo = {0};
		    int ret = BLE_DataProcess(&BleBuff, &PackInfo, &DiagInfo);
			if(ret == PACKSTATE_MSG_ID){
				LED_Toggle();
		#if BLE_DEBUG
				iprintf("SOC: %d SOH: %d Current: %d\r\n",BleBuff.soc, BleBuff.soh, (int)BleBuff.current_mA);
		#endif
				uint8_t tempData[32] = {0};
				memcpy(&tempData, &BleBuff.soc, 3);
				memcpy(&tempData[3], &BleBuff.current_mA, 4);
				memcpy(&tempData[7], &BleBuff.temperature_deciC, (TEMP_SENSOR_LEN));
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[SOC_CUR_TEMP], 0x01, SOC_CUR_TEMP_LEN, 0, SOC_CUR_TEMP_LEN, tempData);
				convertVoltages(&BleBuff.battVolt_mV[0], tempData);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[BATT_VOLT], 0x01, BATT_VOLT_LEN, 0, BATT_VOLT_LEN, tempData);
				memcpy(&tempData, &BleBuff.cell_balancing_status, 2);
				memcpy(&tempData[2], &BleBuff.mosfet_open_close, 1);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[CELL_BAL_MOSFET_OC], 0x01, CELL_BAL_MOSFET_OC_LEN, 0, CELL_BAL_MOSFET_OC_LEN, tempData);
				memcpy(&tempData, &BleBuff.range_dM, 2);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[RANGE], 0x01, RANGE_LEN, 0, RANGE_LEN, tempData);
			}
			else if(ret == BMS_PACK_INFO_MSG_ID)
			{
		#if BLE_DEBUG
				iprintf("FW Version: %s %dS %dP\n", PackInfo.fw_version, PackInfo.series_parallel_config.series, PackInfo.series_parallel_config.parallel);
		#endif
				uint8_t tempData[20] = {0};
				memcpy(&tempData, &PackInfo.battery_id, 20);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[BATTERY_ID], 0x01, BATTERY_ID_LEN, 0, BATTERY_ID_LEN, tempData);
				memcpy(&tempData, &PackInfo.bms_version, 3);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[BMS_VERS_CONFIG], 0x01, BMS_VERS_CONFIG_LEN, 0, BMS_VERS_CONFIG_LEN, tempData);
				memcpy(&tempData, &PackInfo.fw_version, 20);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[FW_VERSION], 0x01, FW_VERSION_LEN, 0, FW_VERSION_LEN, tempData);
			}
			else if (ret == DIAGNOSTIC_MSG_ID)
			{
				uint8_t tempData[20] = {0};
				memcpy(&tempData, &DiagInfo.BmsDiagostic, 11);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DIAGNOSTIC_SERVICE], g_DiagnosticGroup[BMS_DIAGNOSTIC], 0x01, BMS_DIAGNOSTIC_LEN, 0, BMS_DIAGNOSTIC_LEN, tempData);
				memcpy(&tempData, &DiagInfo.PackDiagnostic.DesignCapacity, 9);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DIAGNOSTIC_SERVICE], g_DiagnosticGroup[CAPACITY_CYCLE_WARR], 0x01, CAPACITY_CYCLE_WARR_LEN, 0, CAPACITY_CYCLE_WARR_LEN, tempData);
				memcpy(&tempData, &DiagInfo.PackDiagnostic.FullChargeInstancesCount, 14);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DIAGNOSTIC_SERVICE], g_DiagnosticGroup[INSTANCE_EVENT_COUNT], 0x01, INSTANCE_EVENT_COUNT_LEN, 0, INSTANCE_EVENT_COUNT_LEN, tempData);
				memcpy(&tempData, &DiagInfo.PackDiagnostic.CumulativeAhSpent, 8);
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DIAGNOSTIC_SERVICE], g_DiagnosticGroup[AH_MAX_MIN_VOLT], 0x01, AH_MAX_MIN_VOLT_LEN, 0, AH_MAX_MIN_VOLT_LEN, tempData);
			}
			break;
		}
		case BLE_DISCONNECT_NOTIFY:
		{
			BLE_NotifyConnection(0);
			__disable_irq();
			g_deviceState = BLE_DISCONNECTED;
			__enable_irq();
			break;
		}
		case BLE_DISCONNECTED:
		{
			SetConnectable();
#if BLE_DEBUG
		iprintf("Setting Connectable\r\n");
#endif
			__disable_irq();
			g_deviceState = BLE_ADVERTISING;
			__enable_irq();
			break;
		}
		case BLE_DIAGNOSTIC_REQ:
		{
			BLE_DiagnosticRequest();
			__disable_irq();
			g_deviceState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		default:
			break;
	}
	//Checks if the Auth timeout has occurred
	if((Timer_Expired(&g_SoftTim[CONN_AUTH_TIMER])) && (g_deviceState == BLE_CONNECTED) && (!g_peerAuthenticated))
	{
		hci_disconnect(g_connectionHandle, 0x05);
    }
}


/*Callbacks*/
void hci_le_connection_complete_event(uint8_t Status,
                                      uint16_t Connection_Handle,
                                      uint8_t Role,
                                      uint8_t Peer_Address_Type,
                                      uint8_t Peer_Address[6],
                                      uint16_t Conn_Interval,
                                      uint16_t Conn_Latency,
                                      uint16_t Supervision_Timeout,
                                      uint8_t Master_Clock_Accuracy)
{
#if BLE_DEBUG
	iprintf("Conn Interval:%u Conn Latency:%u Supervision Timeout:%u \n",Conn_Interval,Conn_Latency,Supervision_Timeout);
#endif
	g_connectionHandle = Connection_Handle;
	g_deviceState = BLE_CONNECT_NOTIFY;
}
/*
 * BLE Disconnect Event
 * */
void hci_disconnection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Reason)
{
#if BLE_DEBUG
	if(Reason == 19)
		iprintf("Disconnected by user\n");
	else
		iprintf("Disconnection Reason:%u\n", Reason);
#endif
	g_deviceState = BLE_DISCONNECT_NOTIFY;
	g_peerAuthenticated = 0;
}
/*
 * This callback is triggered when any data is written to a characteristic by central.
 * */
void aci_gatt_attribute_modified_event(uint16_t Connection_Handle,
                                       uint16_t Attr_Handle,
                                       uint16_t Offset,
                                       uint16_t Attr_Data_Length,
                                       uint8_t Attr_Data[])
{
#if BLE_DEBUG
	iprintf("Attribute Changed: %d\n", Attr_Handle);
#endif
	if(Attr_Handle == (g_DataGroup[AUTH_ACK_REQ]+1))
	{
		unsigned int receivedAuthToken = 0;
		receivedAuthToken = ((Attr_Data[3]<<24)|(Attr_Data[2]<<16)|(Attr_Data[1]<<8)|(Attr_Data[0]<<0));
#if BLE_DEBUG
		iprintf("%d %d %d\n", receivedAuthToken, Attr_Handle, g_DataGroup[AUTH_ACK_REQ]);
#endif
		if(receivedAuthToken == (AUTH_TOKEN+100))
		{
			g_peerAuthenticated = 1;
#if BLE_DEBUG
      iprintf("Peer Authenticated\n");
#endif
		}
		if(Attr_Data[4] == 2)
		{
			g_deviceState = BLE_DIAGNOSTIC_REQ;
#if BLE_DEBUG
      iprintf("Diagnostic Requested\n");
#endif
		}
	}
}

/*
 * Callback triggered when a Diagnostic ACK is received from BMS
 * Context : while loop.
 * */
void BLE_DiagnosticMsgCallback(uint8_t state)
{
	if (g_deviceState == 1)
	{
	#if BLE_DEBUG
			iprintf("Diagnostics Request: %s\n", (state == 1)? "Accepted":"Denied");
	#endif
			aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[AUTH_ACK_REQ], 0x01, AUTH_ACK_LEN, 4, 1, &state);
	}
}
