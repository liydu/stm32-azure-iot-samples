/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_azure_iot.c
 * @author  Microsoft
 * @brief   Azure IoT application file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Microsoft.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_azure_iot.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nx_azure_iot_client.h"

#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"

#include "nx_azure_iot_hub_client.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static AZURE_IOT_CONTEXT nx_azure_iot_client;


/* Starting from not initialized */
static volatile UINT  connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
static volatile ULONG periodic_counter  = 0;

/* Telemetry */
static const CHAR telemetry_name[] = "Message ID: ";
static UCHAR      scratch_buffer[256];
static UINT       telemetry_id = 0;

/* Generally, IoTHub Client and DPS Client do not run at the same time, user can use union as below to
   share the memory between IoTHub Client and DPS Client.

   NOTE: If user can not make sure sharing memory is safe, IoTHub Client and DPS Client must be defined seperately.  */
typedef union AZURE_IOT_CLIENT_UNION {
  NX_AZURE_IOT_HUB_CLIENT iothub_client;
#ifdef ENABLE_DPS
  NX_AZURE_IOT_PROVISIONING_CLIENT prov_client;
#endif /* ENABLE_DPS */
} AZURE_IOT_CLIENT;

static AZURE_IOT_CLIENT client;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IotHubClient client.iothub_client
#ifdef ENABLE_DPS
#define ProvisionClient client.prov_client
#endif /* ENABLE_DPS */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

void azure_iot_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static VOID periodic_timer_entry(ULONG context)
{
  NX_PARAMETER_NOT_USED(context);
  tx_event_flags_set(&Events, PERIODIC_EVENT, TX_OR);
}

void azure_iot_entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
  UINT ret = NX_SUCCESS;

  ret = nx_azure_iot_client_create()
}

/* USER CODE END 1 */