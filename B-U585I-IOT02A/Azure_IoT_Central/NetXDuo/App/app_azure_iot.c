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

#include "pnp_device_info.h"
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
#define TELEMETRY_TEMPERATURE "temperature"
#define TELEMETRY_HUMIDITY    "humidity"
#define PROPERTY_LED_STATE    "led_state"
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

/* Parse PnP Device Information component JSON. */
static UINT append_device_info_properties(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
  if (nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_MANUFACTURER_PROPERTY_NAME,
          sizeof(DEVICE_INFO_MANUFACTURER_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_MANUFACTURER_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_MANUFACTURER_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_MODEL_PROPERTY_NAME,
          sizeof(DEVICE_INFO_MODEL_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_MODEL_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_MODEL_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_SW_VERSION_PROPERTY_NAME,
          sizeof(DEVICE_INFO_SW_VERSION_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_SW_VERSION_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_SW_VERSION_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_OS_NAME_PROPERTY_NAME,
          sizeof(DEVICE_INFO_OS_NAME_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_OS_NAME_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_OS_NAME_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_NAME,
          sizeof(DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_string_value(json_writer,
          (UCHAR*)DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_NAME,
          sizeof(DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_NAME) - 1,
          (UCHAR*)DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_VALUE,
          sizeof(DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_VALUE) - 1) ||
      nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
          (UCHAR*)DEVICE_INFO_TOTAL_STORAGE_PROPERTY_NAME,
          sizeof(DEVICE_INFO_TOTAL_STORAGE_PROPERTY_NAME) - 1,
          DEVICE_INFO_TOTAL_STORAGE_PROPERTY_VALUE,
          2) ||
      nx_azure_iot_json_writer_append_property_with_double_value(json_writer,
          (UCHAR*)DEVICE_INFO_TOTAL_MEMORY_PROPERTY_NAME,
          sizeof(DEVICE_INFO_TOTAL_MEMORY_PROPERTY_NAME) - 1,
          DEVICE_INFO_TOTAL_MEMORY_PROPERTY_VALUE,
          2))
  {
    return NX_NOT_SUCCESSFUL;
  }

  return NX_AZURE_IOT_SUCCESS;
}

static UINT append_device_telemetry(NX_AZURE_IOT_JSON_WRITER* json_writer)
{
  float temperature;

  if (BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, &temperature) != BSP_ERROR_NONE)
  {
    printf("ERROR: BSP_ENV_SENSOR_GetValue\r\n");
  }

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

static VOID properties_complete_callback(AZURE_IOT_CONTEXT* context)
{
  /* Device twin processing is done, send out property updates */
  nx_azure_iot_client_publish_properties(context, DEVICE_INFO_COMPONENT_NAME, append_device_info_properties);
  nx_azure_iot_client_publish_bool_property(context, NULL, PROPERTY_LED_STATE, false);
  //nx_azure_iot_client_publish_int_writable_property(context, NULL, TELEMETRY_INTERVAL_PROPERTY, telemetry_interval);

  printf("\r\nStarting Main loop\r\n");
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
  nx_azure_iot_client_register_properties_complete_callback(&nx_azure_iot_client, properties_complete_callback);

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
  nx_azure_iot_client_dps_run(&nx_azure_iot_client, IOT_DPS_ID_SCOPE, IOT_DPS_REGISTRATION_ID, MX_NetXDuo_Connect);
#else
  nx_azure_iot_client_hub_run(&nx_azure_iot_client, IOT_HUB_HOSTNAME, IOT_HUB_DEVICE_ID, MX_NetXDuo_Connect);
#endif

  return NX_SUCCESS;
}

/* USER CODE END 1 */