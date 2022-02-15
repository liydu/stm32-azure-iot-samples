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
#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"

#include "nx_azure_iot_hub_client.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static NX_AZURE_IOT AzureIoT;

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT RootCaCert;
static NX_SECURE_X509_CERT RootCaCert2;
static NX_SECURE_X509_CERT RootCaCert3;
static ULONG               azure_iot_thread_stack[AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
static UCHAR               azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];

static TX_TIMER             PeriodicTimer;
static TX_EVENT_FLAGS_GROUP Events;

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
  UINT  ret  = NX_SUCCESS;
  UINT  loop = NX_TRUE;
  ULONG app_events;

  /* Initialize CA root certificates. */
  ret = nx_secure_x509_certificate_initialize(&RootCaCert,
      (UCHAR*)_nx_azure_iot_root_cert,
      (USHORT)_nx_azure_iot_root_cert_size,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", ret);
    nx_azure_iot_delete(&AzureIoT);
    Error_Handler();
  }

  ret = nx_secure_x509_certificate_initialize(&RootCaCert2,
      (UCHAR*)_nx_azure_iot_root_cert_2,
      (USHORT)_nx_azure_iot_root_cert_size_2,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("Failed to initialize ROOT CA certificate 2!: error code = 0x%08x\r\n", ret);
    nx_azure_iot_delete(&AzureIoT);
    Error_Handler();
  }

  ret = nx_secure_x509_certificate_initialize(&RootCaCert3,
      (UCHAR*)_nx_azure_iot_root_cert_3,
      (USHORT)_nx_azure_iot_root_cert_size_3,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("Failed to initialize ROOT CA certificate 3!: error code = 0x%08x\r\n", ret);
    nx_azure_iot_delete(&AzureIoT);
    Error_Handler();
  }

  ret = tx_event_flags_create(&Events, (CHAR*)"Azure IoT Events");

  if (ret != NX_SUCCESS)
  {
    printf("Failed to create Azure IoT Events!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  ret = tx_timer_create(&PeriodicTimer,
      (CHAR*)"periodic_timer",
      periodic_timer_entry,
      0,
      NX_IP_PERIODIC_RATE,
      NX_IP_PERIODIC_RATE,
      TX_AUTO_ACTIVATE);

  if (ret != NX_SUCCESS)
  {
    printf("Failed to create Azure IoT Events!: error code = 0x%08x\r\n", ret);
    tx_event_flags_delete(&Events);
    Error_Handler();
  }

  /* Create Azure IoT handler.  */
  ret = nx_azure_iot_create(&AzureIoT,
      (UCHAR*)"Azure IoT",
      ip_ptr,
      pool_ptr,
      dns_ptr,
      azure_iot_thread_stack,
      sizeof(azure_iot_thread_stack),
      AZURE_IOT_THREAD_PRIORITY,
      unix_time_callback);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", ret);
    tx_event_flags_delete(&Events);
    Error_Handler();
  }

  // while (loop)
  //{
  //   /* Pickup event flags.  */
  //   tx_event_flags_get(&Events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_WAIT_FOREVER);

  //  if (app_events & CONNECTED_EVENT)
  //  {
  //    connected_action(&IotHubClient);
  //  }

  //  if (app_events & PERIODIC_EVENT)
  //  {
  //    periodic_action(&IotHubClient);
  //  }

  //  if (app_events & TELEMETRY_SEND_EVENT)
  //  {
  //    telemetry_action(&IotHubClient);
  //  }

  //  /* Connection monitor. */
  //  connection_monitor(ip_ptr, &IotHubClient, connection_status, initialize_iothub);
  //}
}

/* USER CODE END 1 */
