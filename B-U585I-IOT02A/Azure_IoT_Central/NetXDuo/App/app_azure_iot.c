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
static volatile UINT        connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
static volatile ULONG       periodic_counter  = 0;

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

void azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

extern VOID connection_monitor(
    NX_IP *ip_ptr, 
    NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
    UINT connection_status,
    UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr)
);

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr, UINT ret)
{
  NX_PARAMETER_NOT_USED(iothub_client_ptr);
  if (ret)
  {
    printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", ret);
    tx_event_flags_set(&Events, DISCONNECT_EVENT, TX_OR);
  }
  else
  {
    printf("Connected to IoTHub.\r\n");
    tx_event_flags_set(&Events, CONNECTED_EVENT, TX_OR);
  }

  connection_status = ret;
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr, VOID* context)
{

  NX_PARAMETER_NOT_USED(iothub_client_ptr);
  NX_PARAMETER_NOT_USED(context);
  tx_event_flags_set(&Events, PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_writable_properties(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr, VOID* context)
{

  NX_PARAMETER_NOT_USED(iothub_client_ptr);
  NX_PARAMETER_NOT_USED(context);
  tx_event_flags_set(&Events, WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

/* Initialize IoT Hub Client for connection_monitor */
static UINT initialize_iothub(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
  UINT ret = NX_SUCCESS;

  UCHAR *iothub_hostname = (UCHAR *)HOST_NAME;
  UCHAR *iothub_device_id = (UCHAR *)DEVICE_ID;
  UINT iothub_hostname_length = sizeof(HOST_NAME) - 1;
  UINT iothub_device_id_length = sizeof(DEVICE_ID) - 1;

  printf("IoT Hub Host Name: %.*s.\r\n", iothub_hostname_length, iothub_hostname);
  printf("Device ID: %.*s.\r\n", iothub_device_id_length, iothub_device_id);

  /* Initialize IoTHub client. */
  ret = nx_azure_iot_hub_client_initialize(iothub_client_ptr,
      &AzureIoT,
      iothub_hostname,
      iothub_hostname_length,
      iothub_device_id,
      iothub_device_id_length,
      (const UCHAR*)MODULE_ID,
      sizeof(MODULE_ID) - 1,
      _nx_azure_iot_tls_supported_crypto,
      _nx_azure_iot_tls_supported_crypto_size,
      _nx_azure_iot_tls_ciphersuite_map,
      _nx_azure_iot_tls_ciphersuite_map_size,
      azure_iot_tls_metadata_buffer,
      sizeof(azure_iot_tls_metadata_buffer),
      &RootCaCert);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  /* Set the model id. */
  ret = nx_azure_iot_hub_client_model_id_set(iothub_client_ptr, (const UCHAR*)DEVICE_MODEL_ID, sizeof(DEVICE_MODEL_ID) - 1);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_model_id_set!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  /* Add more CA certificates. */
  ret = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &RootCaCert2);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  ret = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &RootCaCert3);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  /* Set symmetric key. */
  ret = nx_azure_iot_hub_client_symmetric_key_set(
      iothub_client_ptr, (UCHAR*)DEVICE_SYMMETRIC_KEY, sizeof(DEVICE_SYMMETRIC_KEY) - 1);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
    Error_Handler();
  }

  /* Enable command and properties features. */
  ret = nx_azure_iot_hub_client_command_enable(iothub_client_ptr);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_command_enable!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  ret = nx_azure_iot_hub_client_properties_enable(iothub_client_ptr);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  /* Set connection ret callback. */
  ret = nx_azure_iot_hub_client_connection_status_callback_set(iothub_client_ptr, connection_status_callback);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on connection_ret_callback!\r\n");
    Error_Handler();
  }

  ret = nx_azure_iot_hub_client_receive_callback_set(
      iothub_client_ptr, NX_AZURE_IOT_HUB_PROPERTIES, message_receive_callback_properties, NX_NULL);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on setting device properties callback!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  ret = nx_azure_iot_hub_client_receive_callback_set(
      iothub_client_ptr, NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES, message_receive_callback_writable_properties, NX_NULL);

  if (ret != NX_SUCCESS)
  {
    printf("Failed on setting device writable properties callback!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  if (ret)
  {
    nx_azure_iot_hub_client_deinitialize(iothub_client_ptr);
  }

  return (ret);
}

static VOID periodic_timer_entry(ULONG context)
{
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&Events, PERIODIC_EVENT, TX_OR);
}

void azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time))
{
  UINT  ret = NX_SUCCESS;
  UINT  loop = NX_TRUE;
  ULONG app_events;

  /* Create Azure IoT handler.  */
  ret = nx_azure_iot_create(&AzureIoT, (UCHAR *)"Azure IoT", ip_ptr, pool_ptr, dns_ptr,
                            azure_iot_thread_stack, sizeof(azure_iot_thread_stack),
                            AZURE_IOT_THREAD_PRIORITY, unix_time_callback);
  if (ret != NX_SUCCESS)
  {
    printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

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
    printf("Failed to initialize ROOT CA certificate 1!: error code = 0x%08x\r\n", ret);
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

  tx_timer_create(&PeriodicTimer,
      (CHAR*)"periodic_timer",
      periodic_timer_entry,
      0,
      NX_IP_PERIODIC_RATE,
      NX_IP_PERIODIC_RATE,
      TX_AUTO_ACTIVATE);

  tx_event_flags_create(&Events, (CHAR *)"Azure IoT Events");

  while (loop)
  {
    /* Pickup event flags.  */
    tx_event_flags_get(&Events, ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_WAIT_FOREVER);

    if (app_events & CONNECTED_EVENT)
    {

    }

    /* Connection monitor. */
    connection_monitor(ip_ptr, &IotHubClient, connection_status, initialize_iothub);
  }
  
}

/* USER CODE END 1 */