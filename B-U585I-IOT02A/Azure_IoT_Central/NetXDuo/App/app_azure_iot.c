/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_iot.c
  * @author  MCD Application Team
  * @brief   Azure IoT applicative file
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

static NX_AZURE_IOT AzureIoT;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
ULONG azure_iot_thread_stack[AZURE_IOT_STACK_SIZE / sizeof(ULONG)];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

void azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

void azure_iot_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time))
{
  UINT ret;

  /* Create Azure IoT handler.  */
  if ((ret = nx_azure_iot_create(&AzureIoT, (UCHAR *)"Azure IoT", ip_ptr, pool_ptr, dns_ptr,
                            azure_iot_thread_stack, sizeof(azure_iot_thread_stack),
                            AZURE_IOT_THREAD_PRIORITY, unix_time_callback)));
  {
    printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", ret);
    Error_Handler();
  }

  
}

/* USER CODE END 1 */