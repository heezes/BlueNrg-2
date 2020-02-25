/*
 * App.c
 *
 *  Created on: 26-Feb-2019
 *      Author: user
 */
#ifdef BMS


#include <app_bms.h>
#include <gatt_db_bms.h>
#include "gp_timer.h"
#include "rsa_ed.h"
#include "string.h"
#include "math.h"



/*Private Defines*/
#define  UPDATE_CONN_PARAM 	  0
/*Timer Defines*/
#define  CONN_AUTH_TIMER      0

/*Private TypeDef*/
struct timer g_SoftTim[2];

/* Private function prototypes */
#if BLE_DEBUG
int _write(int fd, char *str, int len)
{return	(COM_SendBlocking((uint8_t*)str,len,10000)==COM_AWS_SUCCESS)?(len):(-1);}
#endif


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
	  g_deviceState = BLE_DISCONNECTED;
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

#endif
