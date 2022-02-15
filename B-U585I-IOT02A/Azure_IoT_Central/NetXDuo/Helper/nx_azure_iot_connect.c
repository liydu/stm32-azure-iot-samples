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
#include "nx_azure_iot_hub_client.h"

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
static UINT iothub_init_count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID connection_monitor(
    NX_IP *ip_ptr, 
    NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, 
    UINT connection_status, 
    UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr)
);

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */
static UINT exponential_backoff_with_jitter()
{
double jitter_percent = (MAX_EXPONENTIAL_BACKOFF_JITTER_PERCENT / 100.0) * (rand() / ((double)RAND_MAX));
UINT base_delay = MAX_EXPONENTIAL_BACKOFF_IN_SEC;
uint64_t delay;

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

    return((UINT)(base_delay * (1 + jitter_percent)) * NX_IP_PERIODIC_RATE) ;
}

static VOID exponential_backoff_reset()
{
    exponential_retry_count = 0;
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
VOID connection_monitor(NX_IP *ip_ptr, NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr, UINT connection_status, 
                        UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT* iothub_client_ptr))
{
  UINT loop = NX_TRUE;
  ULONG gateway_address;

    /* Check parameters.  */
  if ((ip_ptr == NX_NULL) || (iothub_client_ptr == NX_NULL) || (iothub_init == NX_NULL))
  {
    Error_Handler();
  }

  /* Check if connected.  */
  if (connection_status == NX_SUCCESS)
  {

    /* Reset the exponential.  */
    exponential_backoff_reset();
  }
  else
  {

    /* Disconnect.  */
    if (connection_status != NX_AZURE_IOT_NOT_INITIALIZED)
    {
      nx_azure_iot_hub_client_disconnect(iothub_client_ptr);
    }

    /* Recover.  */
    while (loop)
    {
      switch (connection_status)
      {

        /* Something bad has happened with client state, we need to re-initialize it.  */
        case NX_DNS_QUERY_FAILED:
        case NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD:
        case NXD_MQTT_ERROR_NOT_AUTHORIZED:
        {

          /* Deinitialize iot hub client. */
          nx_azure_iot_hub_client_deinitialize(iothub_client_ptr);
        }
          /* fallthrough */

        case NX_AZURE_IOT_NOT_INITIALIZED:
        {
          if (iothub_init_count++)
          {
            printf("Re-initializing iothub connection, after backoff\r\n");
            tx_thread_sleep(exponential_backoff_with_jitter());
          }

          /* Initialize iot hub.  */
          if (iothub_init(iothub_client_ptr))
          {
            connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
          }
          else
          {

            /* Wait for network.  */
            while (nx_ip_gateway_address_get(ip_ptr, &gateway_address))
            {
              tx_thread_sleep(NX_IP_PERIODIC_RATE);
            }

            /* Connect to iot hub.  */
            connection_status = nx_azure_iot_hub_client_connect(iothub_client_ptr, NX_FALSE, NX_WAIT_FOREVER);
          }
        }
        break;

        case NX_AZURE_IOT_SAS_TOKEN_EXPIRED:
        {
          printf("SAS token expired\r\n");
        }
        
        /* Fallthrough */
        default:
        {
          printf("Reconnecting iothub, after backoff\r\n");

          tx_thread_sleep(exponential_backoff_with_jitter());

          /* Wait for network.  */
          while (nx_ip_gateway_address_get(ip_ptr, &gateway_address))
          {
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
          }

          connection_status = nx_azure_iot_hub_client_connect(iothub_client_ptr, NX_FALSE, NX_WAIT_FOREVER);
        }
        break;
      }

      /* Check status.  */
      if (connection_status == NX_SUCCESS)
      {
        return;
      }
    }
  }
}
/* USER CODE END 1 */