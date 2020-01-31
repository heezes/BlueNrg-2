/*
 * app_vim.c
 *
 *  Created on: Dec 18, 2019
 *      Author: Altamash
 */



#ifdef VIM
#include "app_vim.h"
#include <gatt_db_vim.h>
#include "gp_timer.h"
#include "string.h"
#include "math.h"




/*Private Defines*/
#define  UPDATE_CONN_PARAM 	  0
/*Timer Defines*/
#define  CONN_AUTH_TIMER      0

/*Private TypeDef*/
struct timer g_SoftTim[2];

volatile uint8_t  g_receivedAuthToken[MAX_TOKEN_SIZE] = {0};

volatile BleHandleTypeDef_t myBle = {0};

/* Private function prototypes */
int NotifyKeylessUnlock(void)
{
 return BLE_NotifyAuthentication(1);
}


#if BLE_DEBUG
int _write(int fd, char *str, int len)
{return	(COM_SendBlocking((uint8_t*)str,len,10000)==COM_AWS_SUCCESS)?(len):(-1);}
#endif


void App_Init(void)
{
	  LED_Init();
	  WAKEUP_Init();
	  LED_Off();
	  COM_Init();
	  MODULE_DEBUG("Starting!\r\n");
	  if(Device_Init() != BLE_STATUS_AWS_SUCCESS)
	  {
		  MODULE_DEBUG("App Init Failed at Line: %d\r\n", __LINE__);
		  while(1);
	  }
	  Timer_Set(&g_SoftTim[CONN_AUTH_TIMER], 20000);
	  uint32_t myTempToken = 12345;
	  memcpy((uint8_t*)&g_sentAuthToken, &myTempToken, MAX_TOKEN_SIZE);
	  MODULE_DEBUG("Encrypted Token:%d\n", g_sentAuthToken);
	  myBle.bleState = BLE_DISCONNECTED;
}

void App_Process(void)
{
	BLE_DataProcess();
	switch(myBle.bleState)
	{
		case BLE_ADVERTISING:
			break;
		case BLE_CONNECT_NOTIFY:
		{
			WAKEUP_Pulse();
			Timer_Restart(&g_SoftTim[CONN_AUTH_TIMER]);
			__disable_irq();
			myBle.bleState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		case BLE_AUTHENTICATION_NOTIFY:
		{
			BLE_NotifyConnection(1);
			HAL_Delay(100); //To make sure UART Tx is done and VIM is ready to receive data
			NotifyKeylessUnlock();
			__disable_irq();
			myBle.bleState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		case BLE_CONNECTED:
		{
			if(g_peerAuthenticated){
				LED_On();
			}
			if(myBle.responseOpcode != 0x00)
			{
				//Update the Response
				aci_gatt_update_char_value_ext(g_connectionHandle , g_ServiceHandle[DATA_SERVICE], g_DataGroup[RESPONSE], 0x01, RESPONSE_LEN, 0, RESPONSE_LEN, (uint8_t *)&myBle.responseOpcode);
				__disable_irq();
				myBle.responseOpcode = 0;
				__enable_irq();
			}
			break;
		}
		case BLE_LOCK_NOTIFY:
		{
			BLE_NotifyLockRequest();
			__disable_irq();
			myBle.bleState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		case BLE_RIDE_RESUME_NOTIFY:
		{
			BLE_NotifyRideResume();
			myBle.responseOpcode = POSITIVE_ACK;
			__disable_irq();
			myBle.bleState = BLE_CONNECTED;
			__enable_irq();
			break;
		}
		case BLE_DISCONNECT_NOTIFY:
		{
			BLE_NotifyConnection(0);
			LED_Off();
			__disable_irq();
			myBle.bleState = BLE_DISCONNECTED;
			__enable_irq();
			break;
		}
		case BLE_DISCONNECTED:
		{
			SetConnectable();
			MODULE_DEBUG("Setting Connectable\r\n");
			__disable_irq();
			myBle.bleState = BLE_ADVERTISING;
			__enable_irq();
			break;
		}
		default:
			break;
	}
	//Checks if the Auth timeout has occurred
	if((Timer_Expired(&g_SoftTim[CONN_AUTH_TIMER])) && (myBle.bleState == BLE_CONNECTED) && (!g_peerAuthenticated))
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
	MODULE_DEBUG("Conn Interval:%u Conn Latency:%u Supervision Timeout:%u \n",Conn_Interval,Conn_Latency,Supervision_Timeout);
	g_connectionHandle = Connection_Handle;
	myBle.bleState = BLE_CONNECT_NOTIFY;
}
/*
 * BLE Disconnect Event
 * */
void hci_disconnection_complete_event(uint8_t Status, uint16_t Connection_Handle, uint8_t Reason)
{
	if(Reason == 19)
		MODULE_DEBUG("Disconnected by user\n");
	else
		MODULE_DEBUG("Disconnection Reason:%u\n", Reason);
	myBle.bleState = BLE_DISCONNECT_NOTIFY;
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
	MODULE_DEBUG("Attribute Changed: %d\n", Attr_Handle);
	if(Attr_Handle == (g_DataGroup[DATA]+1))
	{
		if(myBle.cmdOpcode == AUTH_OPCODE)
		{
			memcpy((uint8_t*)&g_receivedAuthToken, Attr_Data, 4);
			MODULE_DEBUG("%d %d %d\n", g_receivedAuthToken, Attr_Handle, g_DataGroup[DATA]);
			if(memcmp((uint8_t*)&g_receivedAuthToken, (uint8_t*)&g_sentAuthToken, MAX_TOKEN_SIZE) == 0)
			{
				g_peerAuthenticated = 1;
				MODULE_DEBUG("Peer Authenticated\n");
				if(myBle.cmdOpcode == AUTH_OPCODE)
				{
					myBle.responseOpcode = POSITIVE_ACK;
					myBle.bleState = BLE_AUTHENTICATION_NOTIFY;
				}
			}
			else
			{
				if(myBle.cmdOpcode == AUTH_OPCODE)
				{
					myBle.responseOpcode = NEGATIVE_ACK;
				}
			}
		}
	}
	else if(Attr_Handle == (g_DataGroup[COMMAND]+1))
	{
		myBle.cmdOpcode = Attr_Data[0];
		switch(myBle.cmdOpcode)
		{
			case RIDE_RESUME_REQ_OPCODE:
			{
				myBle.bleState = BLE_RIDE_RESUME_NOTIFY;
				break;
			}
			case LOCK_REQ_OPCODE:
			{
				myBle.bleState = BLE_LOCK_NOTIFY;
				break;
			}
			case VEHICLE_STATE_OPCODE:
			{
				myBle.responseOpcode = myBle.vehicleState;
				break;
			}
			default:
				break;
		}
	}
}

/*******************************************************************************************************/
void BLE_AuthTokenReceivedCallback(uint8_t *token, uint8_t len)
{
	if(token != NULL)
	{
		memcpy((uint8_t*)&g_sentAuthToken, &token, len);
	}
}

void BLE_LockReqCallback(uint8_t state)
{
	if(myBle.cmdOpcode == LOCK_REQ_OPCODE)
		if(state == LOCK_REQUEST_DENIED)
			myBle.responseOpcode = LOCK_REQ_DENIED;
		else
		{
			myBle.responseOpcode = POSITIVE_ACK;
			myBle.vehicleState = VEHICLE_LOCKED;
		}
	else
		MODULE_DEBUG("Lock Req Denied Sync Error\n");
}

void BLE_DataReceivedCallback(uint8_t* data, int len)
{

}

void BLE_UnlockCallback(uint8_t data)
{
	if(data == 1)
	{
		myBle.vehicleState = VEHICLE_UNLOCKED;
	}
}

void BLE_RidePausedCallback(void)
{
	myBle.vehicleState = VEHICLE_PAUSED;
}
/*******************************************************************************************************/
#endif
