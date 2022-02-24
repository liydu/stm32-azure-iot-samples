/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_netxduo.h
 * @author  MCD Application Team
 * @brief   NetXDuo applicative header file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NX_AZURE_IOT_CLIENT_H__
#define __NX_AZURE_IOT_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include "nx_api.h"

#include "nxd_dns.h"

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"
#include "nx_azure_iot_provisioning_client.h"

#include "nx_azure_iot_ciphersuites.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define NX_AZURE_IOT_STACK_SIZE  (2 * 1024)
#define AZURE_IOT_STACK_SIZE     (3 * 1024)
#define AZURE_IOT_HOST_NAME_SIZE 128
#define AZURE_IOT_DEVICE_ID_SIZE 64

#define AZURE_IOT_AUTH_MODE_UNKNOWN 0
#define AZURE_IOT_AUTH_MODE_SAS     1
#define AZURE_IOT_AUTH_MODE_CERT    2

/* The Azure IoT context used for the client app */
typedef struct AZURE_IOT_CONTEXT_STRUCT AZURE_IOT_CONTEXT;

typedef void (*func_ptr_command_received)(
    AZURE_IOT_CONTEXT*, const UCHAR*, USHORT, const UCHAR*, USHORT, UCHAR*, USHORT, VOID*, USHORT);
typedef void (*func_ptr_writable_property_received)(
    AZURE_IOT_CONTEXT*, const UCHAR*, UINT, UCHAR*, UINT, NX_AZURE_IOT_JSON_READER*, UINT);
typedef void (*func_ptr_property_received)(
    AZURE_IOT_CONTEXT*, const UCHAR*, UINT, UCHAR*, UINT, NX_AZURE_IOT_JSON_READER*, UINT);
typedef void (*func_ptr_properties_complete)(AZURE_IOT_CONTEXT*);
typedef void (*func_ptr_timer)(AZURE_IOT_CONTEXT*);

/* Azure IoT context struct */
struct AZURE_IOT_CONTEXT_STRUCT
{
  NX_SECURE_X509_CERT root_ca_cert;
  NX_SECURE_X509_CERT root_ca_cert_2;
  NX_SECURE_X509_CERT root_ca_cert_3;

  NX_IP* azure_iot_nx_ip;

  ULONG nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE / sizeof(ULONG)];
  ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
  ULONG azure_iot_thread_stack[AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

  UINT azure_iot_auth_mode;

  // PnP device model id
  CHAR* azure_iot_model_id;
  UINT  azure_iot_model_id_length;

  // Auth config
  CHAR*               azure_iot_device_sas_key;
  UINT                azure_iot_device_sas_key_length;
  NX_SECURE_X509_CERT device_certificate;

  // IoT Hub connection config
  CHAR azure_iot_hub_hostname[AZURE_IOT_HOST_NAME_SIZE];
  UINT azure_iot_hub_hostname_length;
  CHAR azure_iot_hub_device_id[AZURE_IOT_DEVICE_ID_SIZE];
  UINT azure_iot_hub_device_id_length;

  TX_THREAD            azure_iot_thread;
  TX_EVENT_FLAGS_GROUP events;
  TX_TIMER             periodic_timer;

  NX_AZURE_IOT nx_azure_iot;

  UINT azure_iot_connection_status;

  // union DPS and Hub as they are used consecutively and will save space
  union CLIENT_UNION {
      NX_AZURE_IOT_HUB_CLIENT          iothub;
      NX_AZURE_IOT_PROVISIONING_CLIENT dps;
  } client;

#define iothub_client client.iothub
#define dps_client    client.dps

  func_ptr_command_received           command_received_cb;
  func_ptr_writable_property_received writable_property_received_cb;
  func_ptr_property_received          property_received_cb;
  func_ptr_properties_complete        properties_complete_cb;
  func_ptr_timer                      timer_cb;
};

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* Periodic telemetry sending. */
UINT nx_azure_iot_client_periodic_interval_set(AZURE_IOT_CONTEXT* context, INT interval);

UINT nx_azure_iot_client_register_timer_callback(
    AZURE_IOT_CONTEXT* context, func_ptr_timer callback, int32_t interval);

UINT nx_azure_iot_client_publish_telemetry(AZURE_IOT_CONTEXT* context,
    CHAR*                                                     component_name_ptr,
    UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr));

/* Set authentication. */
UINT nx_azure_iot_client_sas_set(AZURE_IOT_CONTEXT* context, CHAR* device_sas_key);

UINT nx_azure_iot_client_create(AZURE_IOT_CONTEXT* context,
    NX_IP*                                         nx_ip,
    NX_PACKET_POOL*                                nx_pool,
    NX_DNS*                                        nx_dns,
    UINT (*unix_time_callback)(ULONG* unix_time),
    CHAR* device_model_id,
    UINT  device_model_id_length);

/* Main loop. */
UINT nx_azure_iot_client_hub_run(
    AZURE_IOT_CONTEXT* context, CHAR* iot_hub_hostname, CHAR* iot_hub_device_id, UINT (*network_connect)());
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __NX_AZURE_IOT_CLIENT_H__ */
