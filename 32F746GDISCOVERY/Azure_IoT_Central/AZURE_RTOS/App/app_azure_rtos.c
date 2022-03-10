/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.c
  * @author  MCD Application Team
  * @brief   app_azure_rtos application implementation file
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

/* Includes ------------------------------------------------------------------*/
#include "app_azure_rtos.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_netxduo.h"
#include "app_azure_iot.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_THREAD_STACK_SIZE 4096
#define APP_THREAD_PRIORITY   10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TX_THREAD AppThread;
ULONG     app_thread_stack[APP_THREAD_STACK_SIZE / sizeof(ULONG)];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
/**
 * @brief  App thread entry.
 * @param  first_unused_memory : Pointer to the first unused memory
 * @retval None
 */
static VOID app_thread_entry(ULONG parameter)
{
  UINT status;

  printf("Starting Azure thread\r\n\r\n");

  /* Initialize NetXDuo. */
  if ((status = MX_NetXDuo_Init()))
  {
    printf("ERROR: Failed to initialize NetXDuo (0x%08x)\r\n", status);
    Error_Handler();
  }

  //if ((status = azure_iot_nx_client_entry(&nx_ip, &nx_pool, &nx_dns_client, sntp_time)))
  if ((status = Azure_Iot_Entry(&IpInstance, &AppPool, &DnsClient, sntp_time)))
  {
    printf("ERROR: Failed to run Azure IoT (0x%04x)\r\n", status);
    Error_Handler();
  }
}

/**
* @brief  Define the initial system.
* @param  first_unused_memory : Pointer to the first unused memory
* @retval None
*/
VOID tx_application_define(VOID *first_unused_memory)
{
  /* USER CODE BEGIN  tx_application_define_1*/
  /* Create Azure thread. */
  UINT status = tx_thread_create(&AppThread,
      "App Thread",
      app_thread_entry,
      0,
      app_thread_stack,
      APP_THREAD_STACK_SIZE,
      APP_THREAD_PRIORITY,
      APP_THREAD_PRIORITY,
      TX_NO_TIME_SLICE,
      TX_AUTO_START);

  if (status != TX_SUCCESS)
  {
    printf("ERROR: Azure IoT thread creation failed\r\n");
  }
  /* USER CODE END  tx_application_define_1 */
/*
 * Using dynamic memory allocation requires to apply some changes to the linker file.
 * ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function,
 * using the "first_unused_memory" argument.
 * This require changes in the linker files to expose this memory location.
 * For EWARM add the following section into the .icf file:
     place in RAM_region    { last section FREE_MEM };
 * For MDK-ARM
     - either define the RW_IRAM1 region in the ".sct" file
     - or modify the line below in "tx_low_level_initilize.s to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|

 * For STM32CubeIDE add the following section into the .ld file:
     ._threadx_heap :
       {
          . = ALIGN(8);
          __RAM_segment_used_end__ = .;
          . = . + 64K;
          . = ALIGN(8);
        } >RAM_D1 AT> RAM_D1
    * The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
    * In the example above the ThreadX heap size is set to 64KBytes.
    * The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
    * Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
    * Read more in STM32CubeIDE User Guide, chapter: "Linker script".

 * The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.
 */

  /* USER CODE BEGIN DYNAMIC_MEM_ALLOC */
  (void)first_unused_memory;
  /* USER CODE END DYNAMIC_MEM_ALLOC */
}
