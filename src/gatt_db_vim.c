/*
 * gatt_db_vim.c
 *
 *  Created on: Dec 18, 2019
 *      Author: Altamash
 */

#include <gatt_db_vim.h>

#ifdef VIM
uint8_t g_advData[] = {0x02,0x01,0x06,0x06,AD_TYPE_MANUFACTURER_SPECIFIC_DATA,0x30,0x00,0x05,0xFF,0xFF,0xFF,0xFF};

/*Service Handles*/
uint16_t g_ServiceHandle[1] = {0};

/*Characteristic Handles*/
uint16_t g_DataGroup[3] =  {0};

volatile uint8_t g_deviceState = 0;
volatile uint16_t g_connectionHandle = 0;
volatile uint8_t  g_sentAuthToken[MAX_TOKEN_SIZE] = {0};
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

		-Command Characteristic
		  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75dba
		-Response Characteristic
		  	  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75eba
		-DATA Characteristic
		  (-)ed0ef62e-9b0d-11e4-89d3-123b93f75fba
	  */
      /****************************************************************************************************************************/
	  uint8_t DataService_UUID_128[16]			= {0xba,0x5c,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed};
	  uint8_t DataGroup[][16] =
	  {
			  {0xba,0x5d,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5e,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
			  {0xba,0x5f,0xf7,0x93,0x3b,0x12,0xd3,0x89,0xe4,0x11,0x0d,0x9b,0x2e,0xf6,0x0e,0xed},
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
	  ret = aci_gatt_add_service(UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 10, &g_ServiceHandle[DATA_SERVICE]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[COMMAND], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, COMMAND_LEN, CHAR_PROP_WRITE|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,16, 0, &g_DataGroup[COMMAND]);
	  if (ret != BLE_STATUS_AWS_SUCCESS){ ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[RESPONSE], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, RESPONSE_LEN, CHAR_PROP_READ|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &g_DataGroup[RESPONSE]);
	  if (ret != BLE_STATUS_AWS_SUCCESS) {ret = __LINE__;goto fail;}

	  Osal_MemCpy(&charUUID.Char_UUID_128, &DataGroup[DATA][0], 16);
	  ret =  aci_gatt_add_char(g_ServiceHandle[DATA_SERVICE], UUID_TYPE_128, &charUUID, DATA_LEN, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_NOTIFY,
			  	  	  	  	  ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &g_DataGroup[DATA]);
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
	uint8_t DeviceName[] = "VIM";
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
	  uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'V','I','M','-','1'};
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

