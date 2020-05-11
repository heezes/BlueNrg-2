#ifndef _BLE_CONFIG_H_
#define _BLE_CONFIG_H_

#include "bluenrg1_stack.h"
#include "stack_user_cfg.h"

/* This file contains all the information needed to initialize the BlueNRG-1,2 BLE stack.
These constants and variables are used from the BlueNRG-1,2 BLE stack to reserve RAM and FLASH according the application requests. */

/* Device Name */
#define NAME_DEVICE	(BlueNRG)
/* Default number of link */
#define MIN_NUM_LINK	(1)

/* Default number of GAP and GATT services */
#define DEFAULT_NUM_GATT_SERVICES	(2)

/* Default number of GAP and GATT attributes */
#define DEFAULT_NUM_GATT_ATTRIBUTES	(9)

/* Number of services requests from the vim demo */
#define NUM_APP_GATT_SERVICES	(1)

/* Number of attributes requests from the vim demo */
#define NUM_APP_GATT_ATTRIBUTES	(9)

/* OTA characteristics maximum lenght */
#define OTA_MAX_ATT_SIZE	(0)


#define MAX_CHAR_LEN(a,b) ((a) > (b) )? (a) : (b)

/* Set supported max value for attribute size: it is the biggest attribute size enabled by the application. */
#define VIM_MAX_ATT_SIZE	(128)
#define APP_MAX_ATT_SIZE	  MAX_CHAR_LEN(OTA_MAX_ATT_SIZE,  VIM_MAX_ATT_SIZE)

/* Number of links needed for the vim demo: 1 */
#define NUM_LINKS	(MIN_NUM_LINK)

/* Max Number of Attribute Records defined for the service:
Please make sure this value is used for configuring the related aci_gatt_add_service(), Max_Attribute_Records parameter  value */
#define MAX_NUMBER_ATTRIBUTE_SERVICE1	(10)

/* Number of characteristics defined by the user application */
/*#define NUMBER_CHAR_SERVICE1	(3)
*/
/* Number of GATT attributes needed for the vim demo */
#define NUM_GATT_ATTRIBUTES	(DEFAULT_NUM_GATT_ATTRIBUTES + NUM_APP_GATT_ATTRIBUTES)

/* Number of GATT services needed for the vim demo */
#define NUM_GATT_SERVICES	(DEFAULT_NUM_GATT_SERVICES + NUM_APP_GATT_SERVICES)

/* Array size for the attribute value for OTA service */
#if defined (ST_OTA_LOWER_APPLICATION) || defined (ST_OTA_HIGHER_APPLICATION)
#define OTA_ATT_VALUE_ARRAY_SIZE	(119)	/*OTA service is used: 4 characteristics (1 notify property)*/
#else
#define OTA_ATT_VALUE_ARRAY_SIZE	(0)	/*No OTA service is used*/
#endif
/* Array size for the attribute value */
#define ATT_VALUE_ARRAY_SIZE	(490 + OTA_ATT_VALUE_ARRAY_SIZE)

/* Set the size of Flash security database */
#define FLASH_SEC_DB_SIZE	(0X400)
/* Set the size of Flash security database */
#define FLASH_SERVER_DB_SIZE	(0X400)


/* Set supported max value for ATT_MTU enabled by the application. [New parameter added on BLE stack v2.x] */
#define MAX_ATT_MTU	(131)


/* Set supported max value for attribute size: it is the biggest attribute size enabled by the application. */
#define MAX_ATT_SIZE	(APP_MAX_ATT_SIZE)

/*  Set the minumum number of prepare write requests needed for a long write procedure for a characteristic with len > 20bytes: 
 
 It returns 0 for characteristics with len <= 20bytes
 
 NOTE: If prepare write requests are used for a characteristic (reliable write on multiple characteristics), then 
 this value should be set to the number of prepare write needed by the application.
 
  [New parameter added on BLE stack v2.x] 
 */

#define PREPARE_WRITE_LIST_SIZE	(PREP_WRITE_X_ATT(MAX_ATT_SIZE))

/* Set Additional number of memory blocks that will be added to the minimum */
#define OPT_MBLOCKS	(6)	/*6:  for reaching the max throughput: ~220kbps (same as BLE stack 1.x)*/

/* Set the number of memory block for packet allocation */
#define MBLOCKS_COUNT           (MBLOCKS_CALC(PREPARE_WRITE_LIST_SIZE, MAX_ATT_MTU, NUM_LINKS) + OPT_MBLOCKS)

/* RAM reserved to manage all the data stack according the number of links,
 * number of services, number of attributes and attribute value length
 */
NO_INIT(uint32_t dyn_alloc_a[TOTAL_BUFFER_SIZE(NUM_LINKS,NUM_GATT_ATTRIBUTES,NUM_GATT_SERVICES,ATT_VALUE_ARRAY_SIZE,MBLOCKS_COUNT,CONTROLLER_DATA_LENGTH_EXTENSION_ENABLED)>>2]);

/* FLASH reserved to store all the security database information and and the server database information */
NO_INIT_SECTION(uint32_t stacklib_flash_data[TOTAL_FLASH_BUFFER_SIZE(FLASH_SEC_DB_SIZE, FLASH_SERVER_DB_SIZE)>>2], ".noinit.stacklib_flash_data");

/* FLASH reserved to store: security root keys, static random address, public address. */
NO_INIT_SECTION(uint8_t stacklib_stored_device_id_data[56], ".noinit.stacklib_stored_device_id_data");

/* Maximum duration of the connection event */
#define MAX_CONN_EVENT_LENGTH	(0XFFFFFFFF)


/* Sleep clock accuracy */
#if (LS_SOURCE == LS_SOURCE_INTERNAL_RO)

/* Sleep clock accuracy in Slave mode */
#define SLAVE_SLEEP_CLOCK_ACCURACY	(500)

/* Sleep clock accuracy in Master mode */
#define MASTER_SLEEP_CLOCK_ACCURACY	(MASTER_SCA_500ppm)

#else

/* Sleep clock accuracy in Slave mode */
#define SLAVE_SLEEP_CLOCK_ACCURACY	(500)

/* Sleep clock accuracy in Master mode */
#define MASTER_SLEEP_CLOCK_ACCURACY	(MASTER_SCA_500ppm)
#endif


/* Low Speed Oscillator source */
#if (LS_SOURCE == LS_SOURCE_INTERNAL_RO)/* Internal RO */

#define LOW_SPEED_SOURCE	(1)	/*Internal RO*/

#else
/* External 32 KHz */
#define LOW_SPEED_SOURCE	(0)	/* External 32 KHz*/

#endif

/* High Speed start up time */
#define HS_STARTUP_TIME	(0x86)	/*328 us*/

/* Low level hardware configuration data for the device */

#define CONFIG_TABLE \
{\
NULL,\
MAX_CONN_EVENT_LENGTH,\
SLAVE_SLEEP_CLOCK_ACCURACY,\
MASTER_SLEEP_CLOCK_ACCURACY,\
LOW_SPEED_SOURCE,\
HS_STARTUP_TIME\
}

/* This structure contains memory and low level hardware configuration data for the device */
const BlueNRG_Stack_Initialization_t BlueNRG_Stack_Init_params = {
(uint8_t*)stacklib_flash_data,
FLASH_SEC_DB_SIZE,
FLASH_SERVER_DB_SIZE,
(uint8_t*)stacklib_stored_device_id_data,
(uint8_t*)dyn_alloc_a,
TOTAL_BUFFER_SIZE(NUM_LINKS,NUM_GATT_ATTRIBUTES,NUM_GATT_SERVICES,ATT_VALUE_ARRAY_SIZE,MBLOCKS_COUNT,CONTROLLER_DATA_LENGTH_EXTENSION_ENABLED),
NUM_GATT_ATTRIBUTES,
NUM_GATT_SERVICES,
ATT_VALUE_ARRAY_SIZE,
NUM_LINKS,
0,
PREPARE_WRITE_LIST_SIZE, /* [New parameter added on BLE stack v2.x] */
MBLOCKS_COUNT,               /* [New parameter added on BLE stack v2.x] */
MAX_ATT_MTU,             /* [New parameter added on BLE stack v2.x] */
CONFIG_TABLE
};

#endif
