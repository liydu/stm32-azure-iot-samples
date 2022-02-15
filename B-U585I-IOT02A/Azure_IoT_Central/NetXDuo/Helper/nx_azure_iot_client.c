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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
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
  context->azure_iot_model_id_len      = device_model_id_length;

  /* Initialize CA root certificates. */
  ret = nx_secure_x509_certificate_initialize(&context->root_ca_cert,
      (UCHAR*)azure_iot_root_cert,
      (USHORT)azure_iot_root_cert_size,
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
      (UCHAR*)azure_iot_root_cert_2,
      (USHORT)azure_iot_root_cert_size_2,
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
      (UCHAR*)azure_iot_root_cert_3,
      (USHORT)azure_iot_root_cert_size_3,
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
      (ULONG)nx_context,
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
      nx_context->nx_azure_iot_thread_stack,
      sizeof(nx_context->nx_azure_iot_thread_stack),
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

/* USER CODE END 1 */
