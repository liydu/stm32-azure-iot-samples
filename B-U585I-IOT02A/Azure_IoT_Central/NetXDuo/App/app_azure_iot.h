/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_azure_iot.h
 * @author  MCD Application Team
 * @brief   Azure IoT application header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_AZURE_IOT_H__
#define __APP_AZURE_IOT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include "main.h"

#include "nxd_dns.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define IOT_HUB_HOSTNAME  "liya-iot.azure-devices.net"
#define IOT_HUB_DEVICE_ID "u5"

#define IOT_DEVICE_SAS_KEY "P83AodUwCucRbvtSgPWVKbmSeso4vZ79Fm9M/8a1btc="

/* PnP Configurations*/
#define DEVICE_MODEL_ID      "dtmi:azurertos:devkit:gsgstml4s5;2"

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/* USER CODE BEGIN EFP */
UINT Azure_Iot_Entry(
    NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG* unix_time));
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Define wait option. */
#define WAIT_OPTION (NX_NO_WAIT)

/* Define events. */
#define ALL_EVENTS                        ((ULONG)0xFFFFFFFF)
#define CONNECTED_EVENT                   ((ULONG)0x00000001)
#define DISCONNECT_EVENT                  ((ULONG)0x00000002)
#define PERIODIC_EVENT                    ((ULONG)0x00000004)
#define TELEMETRY_SEND_EVENT              ((ULONG)0x00000008)
#define COMMAND_RECEIVE_EVENT             ((ULONG)0x00000010)
#define PROPERTIES_RECEIVE_EVENT          ((ULONG)0x00000020)
#define WRITABLE_PROPERTIES_RECEIVE_EVENT ((ULONG)0x00000040)
#define REPORTED_PROPERTIES_SEND_EVENT    ((ULONG)0x00000080)

  /* USER CODE END PD */

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_AZURE_IOT_H__ */
