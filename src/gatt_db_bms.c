/*
 * gatt.c
 *
 *  Created on: 12-Dec-2019
 *      Author: Altamash
 */

/**************************************************************************************************************************************/

#include <gatt_db_bms.h>
#ifdef BMS
uint8_t g_advData[] = {0x02,0x01,0x06,0x06,AD_TYPE_MANUFACTURER_SPECIFIC_DATA,0x30,0x00,0x05,0xFF,0xFF,0xFF,0xFF};

/*Service Handles*/
uint16_t g_ServiceHandle[2] = {0};

/*Characteristic Handles*/
uint16_t g_DataGroup[8] =  {0};
uint16_t g_DiagnosticGroup[4] = {0};

volatile uint8_t g_deviceState = 0;
volatile uint16_t g_connectionHandle = 0;
volatile int g_sentAuthToken = 0;
volatile int g_peerAuthenticated = 0;


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
	  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 25, &g_ServiceHandle[DATA_SERVICE]);
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


int Device_Init(void)
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

void SetConnectable(void)
{
	  uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','M','S','-','7'};
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
#endif
