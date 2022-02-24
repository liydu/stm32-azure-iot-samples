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
#include "app_netxduo.h"

#include "nx_azure_iot_client.h"

#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"

#include "nx_azure_iot_hub_client.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum TELEMETRY_STATE_ENUM
{
  TELEMETRY_STATE_DEFAULT,
  // TELEMETRY_STATE_MAGNETOMETER,
  // TELEMETRY_STATE_ACCELEROMETER,
  // TELEMETRY_STATE_GYROSCOPE,
  TELEMETRY_STATE_END
} TELEMETRY_STATE;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TELEMETRY_HUMIDITY "humidity"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static AZURE_IOT_CONTEXT nx_azure_iot_client;

static int32_t telemetry_interval = 10;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static UINT append_device_telemetry(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
  float temperature = 22.02;

  //if (BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, &temperature) != BSP_ERROR_NONE)
  //{
  //  printf("ERROR: BSP_ENV_SENSOR_GetValue\r\n");
  //}

  if (nx_azure_iot_json_writer_append_property_with_double_value(
          json_writer, (UCHAR*)TELEMETRY_HUMIDITY, sizeof(TELEMETRY_HUMIDITY) - 1, temperature, 2))
  {
    return NX_NOT_SUCCESSFUL;
  }

  return NX_AZURE_IOT_SUCCESS;
}

static VOID telemetry_callback(AZURE_IOT_CONTEXT* context)
{
  static TELEMETRY_STATE telemetry_state = TELEMETRY_STATE_DEFAULT;

  switch (telemetry_state)
  {
    case TELEMETRY_STATE_DEFAULT:
      nx_azure_iot_client_publish_telemetry(&nx_azure_iot_client, NULL, append_device_telemetry);
    default:
      break;
  }
}

UINT Azure_Iot_Entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time))
{
  UINT ret = NX_SUCCESS;

  ret = nx_azure_iot_client_create(&nx_azure_iot_client,
      ip_ptr,
      pool_ptr,
      dns_ptr,
      unix_time_callback,
      DEVICE_MODEL_ID,
      sizeof(DEVICE_MODEL_ID) - 1);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: nx_azure_iot_client_create failed (0x%08x)\r\n", ret);
    return ret;
  }

  /* Register the callbacks. */
  nx_azure_iot_client_register_timer_callback(&nx_azure_iot_client, telemetry_callback, telemetry_interval);

  /* Set up authentication. */
#ifdef ENABLE_X509

#else
  ret = nx_azure_iot_client_sas_set(&nx_azure_iot_client, IOT_DEVICE_SAS_KEY);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: azure_iot_nx_client_sas_set (0x%08x)\r\n", ret);
    return ret;
  }
#endif

  /* Enter the main loop. */
#ifdef ENABLE_DPS

#else
  nx_azure_iot_client_hub_run(&nx_azure_iot_client, IOT_HUB_HOSTNAME, IOT_HUB_DEVICE_ID, MX_NetXDuo_Connect);
#endif

  return NX_SUCCESS;
}

/* USER CODE END 1 */