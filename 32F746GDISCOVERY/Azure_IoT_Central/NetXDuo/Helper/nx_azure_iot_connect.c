/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    nx_azure_iot_connect.c
  * @author  Microsoft
  * @brief   Azure IoT connection monitoring file
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
#include "nx_azure_iot_client.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_EXPONENTIAL_BACKOFF_IN_SEC         (10 * 60)
#define INITIAL_EXPONENTIAL_BACKOFF_IN_SEC     (3)
#define MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT (60)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static UINT exponential_retry_count;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static UINT exponential_backoff_with_jitter()
{
  double   jitter_percent = (MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0) * (rand() / ((double)RAND_MAX));
  UINT     base_delay     = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
  uint64_t delay;
  UINT     backoff_seconds;

  // If the retry is 0, then we don't need to delay the first time
  if (exponential_retry_count++ == 0)
  {
    return NX_TRUE;
  }

  if (exponential_retry_count < (sizeof(UINT) * 8))
  {
    delay = (uint64_t)((1 << exponential_retry_count) * INITIAL_EXPONENTIAL_BACKOFF_IN_SEC);
    if (delay <= (UINT)(-1))
    {
      base_delay = (UINT)delay;
    }
  }

  if (base_delay > MAX_EXPONENTIAL_BACKOFF_IN_SEC)
  {
    base_delay = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
  }
  else
  {
    exponential_retry_count++;
  }

  backoff_seconds = (UINT)(base_delay * (1 + jitter_percent));

  printf("\r\nIoT connection backoff for %d seconds\r\n", backoff_seconds);
  tx_thread_sleep(backoff_seconds * NX_IP_PERIODIC_RATE);
}

static void exponential_backoff_reset()
{
    exponential_retry_count = 0;
}

static void iothub_connect(AZURE_IOT_CONTEXT* context)
{
  UINT status;

  // Connect to IoT hub
  printf("\r\nInitializing Azure IoT Hub client\r\n");
  printf("\tHub hostname: %.*s\r\n", context->azure_iot_hub_hostname_length, context->azure_iot_hub_hostname);
  printf("\tDevice id: %.*s\r\n", context->azure_iot_hub_device_id_length, context->azure_iot_hub_device_id);
  printf("\tModel id: %.*s\r\n", context->azure_iot_model_id_length, context->azure_iot_model_id);

  if ((status = nx_azure_iot_hub_client_connect(&context->iothub_client, NX_FALSE, NX_WAIT_FOREVER)))
  {
    printf("ERROR: nx_azure_iot_hub_client_connect (0x%08x)\r\n", status);
  }

  // stash the connection status to be used by the monitor loop
  context->azure_iot_connection_status = status;
}

VOID connection_status_set(AZURE_IOT_CONTEXT* context, UINT connection_status)
{
  context->azure_iot_connection_status = connection_status;

  if (context->azure_iot_connection_status == NX_SUCCESS)
  {
    printf("SUCCESS: Connected to IoT Hub\r\n\r\n");
  }
}

/**
 *
 * Connection state
 *
 *
 * +--------------+             +--------------+                      +--------------+
 * |              |    INIT     |              |       NETWORK        |              |
 * |              |   SUCCESS   |              |        GOOD          |              |
 * |    INIT      |             |   NETWORK    |                      |   CONNECT    |
 * |              +------------->    CHECK      +--------------------->              +---------+
 * |              |             |              |                      |              |         |
 * |              |             |              |                      |              |         |
 * +------^-------+             +------^-------+                      +------+-------+         |
 *        |                            |                                     |                 |
 *        |                            |                        CONNECT FAIL |                 |
 *        |                            |          +--------------------------+                 |
 *        |                            |          |                                  CONNECTED |
 *        |                  RECONNECT |          |                                            |
 *        |                            |          |                                            |
 *        |                            |   +------v-------+              +--------------+      |
 *        | REINITIALIZE               |   |              |              |              |      |
 *        |                            +---+              |  DISCONNECT  |              |      |
 *        |                                | DISCONNECTED |              |   CONNECTED  |      |
 *        |                                |              <--------------+              <------+
 *        +--------------------------------+              |              |              |
 *                                         |              |              |              |
 *                                         +--------------+              +---^------+---+
 *                                                                           |      |(TELEMETRY |
 *                                                                           |      | C2D |
 *                                                                           |      | COMMAND | METHOD |
 *                                                                           +------+ PROPERTIES | DEVICE TWIN)
 *
 *
 *
 */
VOID connection_monitor(
    AZURE_IOT_CONTEXT* context, UINT (*iothub_init)(AZURE_IOT_CONTEXT* context), UINT (*network_connect)())
{
  UINT loop = NX_TRUE;

  /* Check parameters. */
  if ((context == NX_NULL) || (iothub_init == NX_NULL))
  {
    return;
  }

  /* Check if connected. */
  if (context->azure_iot_connection_status == NX_SUCCESS)
  {
    /* Reset the exponential. */
    exponential_backoff_reset();
    return;
  }

  /* Disconnect. */
  if (context->azure_iot_connection_status != NX_AZURE_IOT_NOT_INITIALIZED)
  {
    nx_azure_iot_hub_client_disconnect(&context->iothub_client);
  }

  /* Recover. */
  while (loop)
  {
    switch (context->azure_iot_connection_status)
    {
      /* Something bad has happened with client state, we need to re-initialize it. */
      case NX_DNS_QUERY_FAILED:
      case NXD_MQTT_COMMUNICATION_FAILURE:
      case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
      case NXD_MQTT_ERROR_NOT_AUTHORIZED:
      {
        /* Deinitialize iot hub client. */
        nx_azure_iot_hub_client_deinitialize(&context->iothub_client);
      }

      /* Fallthrough. */
      case NX_AZURE_IOT_NOT_INITIALIZED:
      {
        /* Set the state to not initialized. */
        context->azure_iot_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;

        /* Connect the network. */
        if (network_connect() != NX_SUCCESS)
        {
          /* Failed, break out to try again next time. */
          break;
        }

        /* Initiliaze connection to IoT Hub. */
        exponential_backoff_with_jitter();
        if (iothub_init(context) == NX_SUCCESS)
        {
          iothub_connect(context);
        }
      }
      break;

      case NX_AZURE_IOT_SAS_TOKEN_EXPIRED:
      {
        printf("SAS token has expired\r\n");
      }

      /* Fallthrough. */
      default:
      {
        /* Connect IoT Hub. */
        exponential_backoff_with_jitter();
        iothub_connect(context);
      }
      break;
    }

    /* Check status, return on success. */
    if (context->azure_iot_connection_status == NX_SUCCESS)
    {
      return;
    }
  }
}
/* USER CODE END 1 */