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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nx_azure_iot_client.h"

#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"

#include "nx_azure_iot_connect.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* nx_azure_iot_create priority */
#define NX_AZURE_IOT_THREAD_PRIORITY 4

/* Incoming events from the middleware. */
#define HUB_ALL_EVENTS                        0xFF
#define HUB_CONNECT_EVENT                     0x01
#define HUB_DISCONNECT_EVENT                  0x02
#define HUB_COMMAND_RECEIVE_EVENT             0x04
#define HUB_PROPERTIES_RECEIVE_EVENT          0x08
#define HUB_WRITABLE_PROPERTIES_RECEIVE_EVENT 0x10
#define HUB_PROPERTIES_COMPLETE_EVENT         0x20
#define HUB_PERIODIC_TIMER_EVENT              0x40

#define MODULE_ID ""

/* Connection timeouts in threadx ticks. */
#define HUB_CONNECT_TIMEOUT_TICKS  (10 * TX_TIMER_TICKS_PER_SECOND)
#define DPS_REGISTER_TIMEOUT_TICKS (30 * TX_TIMER_TICKS_PER_SECOND)

#define DPS_PAYLOAD_SIZE       (15 + 128)
#define TELEMETRY_BUFFER_SIZE  256
#define PROPERTIES_BUFFER_SIZE 128

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static UCHAR telemetry_buffer[TELEMETRY_BUFFER_SIZE];
static UCHAR properties_buffer[PROPERTIES_BUFFER_SIZE];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
/**
 * @brief  Create actual IoT Hub client.
 * @param context: AZURE_IOT_CONTEXT
 * @retval ret
 */
static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr, UINT status)
{
  // :HACK: This callback doesn't allow us to provide context, pinch it from the command message callback args
  AZURE_IOT_CONTEXT* context = hub_client_ptr->nx_azure_iot_hub_client_command_message.message_callback_args;

  if (status == NX_SUCCESS)
  {
    tx_event_flags_set(&context->events, HUB_CONNECT_EVENT, TX_OR);
  }
  else
  {
    tx_event_flags_set(&context->events, HUB_DISCONNECT_EVENT, TX_OR);
  }

  /* Update the connection status in the connect workflow. */
  connection_status_set(context, status);
}

static VOID periodic_timer_entry(ULONG context)
{
  AZURE_IOT_CONTEXT* nx_context = (AZURE_IOT_CONTEXT*)context;
  tx_event_flags_set(&nx_context->events, HUB_PERIODIC_TIMER_EVENT, TX_OR);
}

static UINT iot_hub_initialize(AZURE_IOT_CONTEXT* context)
{
  UINT ret;

  /* Initialize IoT Hub client. */
  if ((ret = nx_azure_iot_hub_client_initialize(&context->iothub_client,
           &context->nx_azure_iot,
           (UCHAR*)context->azure_iot_hub_hostname,
           context->azure_iot_hub_hostname_length,
           (UCHAR*)context->azure_iot_hub_device_id,
           context->azure_iot_hub_device_id_length,
           (UCHAR*)MODULE_ID,
           sizeof(MODULE_ID) - 1,
           _nx_azure_iot_tls_supported_crypto,
           _nx_azure_iot_tls_supported_crypto_size,
           _nx_azure_iot_tls_ciphersuite_map,
           _nx_azure_iot_tls_ciphersuite_map_size,
           (UCHAR*)context->nx_azure_iot_tls_metadata_buffer,
           sizeof(context->nx_azure_iot_tls_metadata_buffer),
           &context->root_ca_cert)))
  {
    printf("Error: on nx_azure_iot_hub_client_initialize (0x%08x)\r\n", ret);
    return ret;
  }

  /* Set credentials. */
  if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_SAS)
  {
    /* Symmetric (SAS) Key. */
    if ((ret = nx_azure_iot_hub_client_symmetric_key_set(&context->iothub_client,
             (UCHAR*)context->azure_iot_device_sas_key,
             context->azure_iot_device_sas_key_length)))
    {
      printf("Error: failed on nx_azure_iot_hub_client_symmetric_key_set (0x%08x)\r\n", ret);
    }
  }
  else if (context->azure_iot_auth_mode == AZURE_IOT_AUTH_MODE_CERT)
  {
    /* X509 Certificate. */
    if ((ret = nx_azure_iot_hub_client_device_cert_set(&context->iothub_client, &context->device_certificate)))
    {
      printf("Error: failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", ret);
    }
  }

  if (ret != NX_AZURE_IOT_SUCCESS)
  {
    printf("Failed to set auth credentials\r\n");
  }

  // Add more CA certificates
  else if ((ret = nx_azure_iot_hub_client_trusted_cert_add(&context->iothub_client, &context->root_ca_cert_2)))
  {
    printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", ret);
  }
  else if ((ret = nx_azure_iot_hub_client_trusted_cert_add(&context->iothub_client, &context->root_ca_cert_3)))
  {
    printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", ret);
  }

  /* Set Model id. */
  else if ((ret = nx_azure_iot_hub_client_model_id_set(&context->iothub_client,
                (UCHAR*)context->azure_iot_model_id,
                context->azure_iot_model_id_length)))
  {
    printf("Error: nx_azure_iot_hub_client_model_id_set (0x%08x)\r\n", ret);
  }

  /* Set connection status callback. */
  else if ((ret = nx_azure_iot_hub_client_connection_status_callback_set(
                &context->iothub_client, connection_status_callback)))
  {
    printf("Error: failed on connection_status_callback (0x%08x)\r\n", ret);
  }

  /* Enable commands. */
  //else if ((ret = nx_azure_iot_hub_client_command_enable(&context->iothub_client)))
  //{
  //  printf("Error: command receive enable failed (0x%08x)\r\n", ret);
  //}

  /* Enable properties. */
  //else if ((ret = nx_azure_iot_hub_client_properties_enable(&context->iothub_client)))
  //{
  //  printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", ret);
  //}

  /* Set properties callback. */
  //else if ((ret = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
  //              NX_AZURE_IOT_HUB_PROPERTIES,
  //              message_receive_callback_properties,
  //              (VOID*)context)))
  //{
  //  printf("Error: device twin callback set (0x%08x)\r\n", ret);
  //}

  /* Set command callback. */
  //else if ((ret = nx_azure_iot_hub_client_receive_callback_set(
  //              &context->iothub_client, NX_AZURE_IOT_HUB_COMMAND, message_receive_command, (VOID*)context)))
  //{
  //  printf("Error: device method callback set (0x%08x)\r\n", ret);
  //}

  /* Set the writable property callback. */
  //else if ((ret = nx_azure_iot_hub_client_receive_callback_set(&context->iothub_client,
  //              NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
  //              message_receive_callback_writable_property,
  //              (VOID*)context)))
  //{
  //  printf("Error: device twin desired property callback set (0x%08x)\r\n", ret);
  //}

  if (ret != NX_AZURE_IOT_SUCCESS)
  {
    nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
  }

  return ret;
}

static VOID process_connect(AZURE_IOT_CONTEXT* context)
{
  UINT ret;

  // Request the client properties
  if ((ret = nx_azure_iot_hub_client_properties_request(&context->iothub_client, NX_WAIT_FOREVER)))
  {
    printf("ERROR: failed to request properties (0x%08x)\r\n", ret);
  }

  // Start the periodic timer
  if ((ret = tx_timer_activate(&context->periodic_timer)))
  {
    printf("ERROR: tx_timer_activate (0x%08x)\r\n", ret);
  }
}

static VOID process_disconnect(AZURE_IOT_CONTEXT* context)
{
  UINT ret;

  printf("Disconnected from IoT Hub\r\n");

  // Stop the periodic timer
  if ((ret = tx_timer_deactivate(&context->periodic_timer)))
  {
    printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", ret);
  }
}

UINT nx_azure_iot_client_publish_telemetry(
  AZURE_IOT_CONTEXT* context, CHAR* component_name_ptr, 
  UINT (*append_properties)(NX_AZURE_IOT_JSON_WRITER* json_writer_ptr))
{
  UINT ret;
  UINT telemetry_length;
  NX_PACKET* packet_ptr;
  NX_AZURE_IOT_JSON_WRITER json_writer;

  if ((ret = nx_azure_iot_hub_client_telemetry_message_create(
           &context->iothub_client, &packet_ptr, NX_WAIT_FOREVER)))
  {
    printf("Error: nx_azure_iot_hub_client_telemetry_message_create failed (0x%08x)\r\n", ret);
  }

  if ((ret = nx_azure_iot_json_writer_with_buffer_init(&json_writer, telemetry_buffer, sizeof(telemetry_buffer))))
  {
    printf("Error: Failed to initialize json writer (0x%08x)\r\n", ret);
    nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
    return ret;
  }

  if ((ret = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
      (component_name_ptr != NX_NULL &&
          (ret = nx_azure_iot_hub_client_reported_properties_component_begin(
               &context->iothub_client, &json_writer, (UCHAR*)component_name_ptr, strlen(component_name_ptr)))) ||
      (ret = append_properties(&json_writer)) ||
      (component_name_ptr != NX_NULL && (ret = nx_azure_iot_hub_client_reported_properties_component_end(
                                             &context->iothub_client, &json_writer))) ||
      (ret = nx_azure_iot_json_writer_append_end_object(&json_writer)))
  {
    printf("Error: Failed to build telemetry (0x%08x)\r\n", ret);
    nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
    return ret;
  }

  telemetry_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
  if ((ret = nx_azure_iot_hub_client_telemetry_send(
           &context->iothub_client, packet_ptr, telemetry_buffer, telemetry_length, NX_WAIT_FOREVER)))
  {
    printf("Error: Telemetry message send failed (0x%08x)\r\n", ret);
    nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
    return ret;
  }

  printf("Telemetry message sent: %.*s.\r\n", telemetry_length, telemetry_buffer);

  return ret;
}

UINT nx_azure_iot_client_periodic_interval_set(AZURE_IOT_CONTEXT* context, INT interval)
{
  UINT ret;
  UINT active;
  UINT ticks = interval * TX_TIMER_TICKS_PER_SECOND;

  if ((ret = tx_timer_info_get(&context->periodic_timer, NULL, &active, NULL, NULL, NULL)))
  {
    printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", ret);
    return ret;
  }

  if (active == TX_TRUE && (ret = tx_timer_deactivate(&context->periodic_timer)))
  {
    printf("ERROR: tx_timer_deactivate (0x%08x)\r\n", ret);
  }

  else if ((ret = tx_timer_change(&context->periodic_timer, ticks, ticks)))
  {
    printf("ERROR: tx_timer_change (0x%08x)\r\n", ret);
  }

  else if (active == TX_TRUE && (ret = tx_timer_activate(&context->periodic_timer)))
  {
    printf("ERROR: tx_timer_activate (0x%08x)\r\n", ret);
  }

  return ret;
}

UINT nx_azure_iot_client_register_timer_callback(
    AZURE_IOT_CONTEXT* context, func_ptr_timer callback, int32_t interval)
{
  if (context == NULL || context->timer_cb != NULL)
  {
    return NX_PTR_ERROR;
  }

  nx_azure_iot_client_periodic_interval_set(context, interval);

  context->timer_cb = callback;

  return NX_SUCCESS;
}

UINT nx_azure_iot_client_sas_set(AZURE_IOT_CONTEXT* context, CHAR* device_sas_key)
{
  if (device_sas_key[0] == 0)
  {
    printf("Error: azure_iot_nx_client_sas_set device_sas_key is null\r\n");
    return NX_PTR_ERROR;
  }

  context->azure_iot_auth_mode             = AZURE_IOT_AUTH_MODE_SAS;
  context->azure_iot_device_sas_key        = device_sas_key;
  context->azure_iot_device_sas_key_length = strlen(device_sas_key);

  return NX_SUCCESS;
}

UINT nx_azure_iot_client_create(AZURE_IOT_CONTEXT* context,
  NX_IP*                                         nx_ip,
  NX_PACKET_POOL*                                nx_pool,
  NX_DNS*                                        nx_dns,
  UINT (*unix_time_callback)(ULONG* unix_time),
  CHAR* device_model_id,
  UINT  device_model_id_length)
{
  UINT ret = NX_SUCCESS;

  if (device_model_id_length == 0)
  {
    printf("ERROR: azure_iot_nx_client_create_new empty model_id\r\n");
    return NX_PTR_ERROR;
  }

  /* Initialise the context. */
  memset(context, 0, sizeof(AZURE_IOT_CONTEXT));

  /* Stash parameters. */
  context->azure_iot_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
  context->azure_iot_nx_ip             = nx_ip;
  context->azure_iot_model_id          = device_model_id;
  context->azure_iot_model_id_length   = device_model_id_length;

  /* Initialize CA root certificates. */
  ret = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
      (UCHAR*)_nx_azure_iot_root_cert,
      (USHORT)_nx_azure_iot_root_cert_size,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", ret);
    Error_Handler();
  }

  ret = nx_secure_x509_certificate_initialize(&context->root_ca_cert_2,
      (UCHAR*)_nx_azure_iot_root_cert_2,
      (USHORT)_nx_azure_iot_root_cert_size_2,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", ret);
    Error_Handler();
  }

  ret = nx_secure_x509_certificate_initialize(&context->root_ca_cert_3,
      (UCHAR*)_nx_azure_iot_root_cert_3,
      (USHORT)_nx_azure_iot_root_cert_size_3,
      NX_NULL,
      0,
      NULL,
      0,
      NX_SECURE_X509_KEY_TYPE_NONE);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: nx_secure_x509_certificate_initialize (0x%08x)\r\n", ret);
    Error_Handler();
  }

  ret = tx_event_flags_create(&context->events, "nx_azure_iot_client");

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: tx_event_flags_creates (0x%08x)\r\n", ret);
    Error_Handler();
  }

  ret = tx_timer_create(&context->periodic_timer,
      "periodic_timer",
      periodic_timer_entry,
      (ULONG)context,
      60 * NX_IP_PERIODIC_RATE,
      60 * NX_IP_PERIODIC_RATE,
      TX_NO_ACTIVATE);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: tx_timer_create (0x%08x)\r\n", ret);
    tx_event_flags_delete(&context->events);
    Error_Handler();
  }

  /* Create Azure IoT handler. */
  ret = nx_azure_iot_create(&context->nx_azure_iot,
      (UCHAR*)"Azure IoT",
      nx_ip,
      nx_pool,
      nx_dns,
      context->nx_azure_iot_thread_stack,
      sizeof(context->nx_azure_iot_thread_stack),
      NX_AZURE_IOT_THREAD_PRIORITY,
      unix_time_callback);

  if (ret != NX_SUCCESS)
  {
    printf("ERROR: failed on nx_azure_iot_create (0x%08x)\r\n", ret);
    tx_event_flags_delete(&context->events);
    tx_timer_delete(&context->periodic_timer);
    Error_Handler();
  }

  return ret;
 }

 static UINT client_run(
     AZURE_IOT_CONTEXT* context, UINT (*iot_initialize)(AZURE_IOT_CONTEXT*))
 {
   ULONG app_events;

   while (true)
   {
     app_events = 0;
     tx_event_flags_get(&context->events, HUB_ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_IP_PERIODIC_RATE);

     if (app_events & HUB_DISCONNECT_EVENT)
     {
       process_disconnect(context);
     }

     if (app_events & HUB_CONNECT_EVENT)
     {
       process_connect(context);
     }

     //if (app_events & HUB_PERIODIC_TIMER_EVENT)
     //{
     //  process_timer_event(context);
     //}

     //if (app_events & HUB_PROPERTIES_COMPLETE_EVENT)
     //{
     //  process_properties_complete(context);
     //}

     //if (app_events & HUB_COMMAND_RECEIVE_EVENT)
     //{
     //  process_command(context);
     //}

     //if (app_events & HUB_PROPERTIES_RECEIVE_EVENT)
     //{
     //  process_properties(context);
     //}

     //if (app_events & HUB_WRITABLE_PROPERTIES_RECEIVE_EVENT)
     //{
     //  process_writable_properties(context);
     //}

     /* Mainain monitor and reconnect state */
     //connection_monitor(context, iot_initialize, network_connect);
     connection_monitor(context, iot_initialize);
   }

   return NX_SUCCESS;
 }

 /**
  * @brief  IoT Hub client run.
  * @param context: AZURE_IOT_CONTEXT, 
  * @retval int
  */
 UINT nx_azure_iot_client_hub_run(
     AZURE_IOT_CONTEXT* context, CHAR* iot_hub_hostname, CHAR* iot_hub_device_id)
 {
   if (iot_hub_hostname == 0 || iot_hub_device_id == 0)
   {
     printf("ERROR: azure_iot_nx_client_hub_run hub config is null\r\n");
     return NX_PTR_ERROR;
   }

   if (strlen(iot_hub_hostname) > AZURE_IOT_HOST_NAME_SIZE || strlen(iot_hub_device_id) > AZURE_IOT_DEVICE_ID_SIZE)
   {
     printf("ERROR: azure_iot_nx_client_hub_run hub config exceeds buffer size\r\n");
     return NX_SIZE_ERROR;
   }

   // take a copy of the hub config
   memcpy(context->azure_iot_hub_hostname, iot_hub_hostname, AZURE_IOT_HOST_NAME_SIZE);
   memcpy(context->azure_iot_hub_device_id, iot_hub_device_id, AZURE_IOT_DEVICE_ID_SIZE);
   context->azure_iot_hub_hostname_length  = strlen(iot_hub_hostname);
   context->azure_iot_hub_device_id_length = strlen(iot_hub_device_id);

   return client_run(context, iot_hub_initialize);
 }

/* USER CODE END 1 */
