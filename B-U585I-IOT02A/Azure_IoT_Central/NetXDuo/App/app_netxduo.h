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
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"

/* USER CODE BEGIN Includes */
#include "main.h"

#include "nxd_sntp_client.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"
#include "nx_secure_tls_api.h"

#include "nx_driver_emw3080.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define SNTP_SYNC_MAX        30
#define SNTP_UPDATE_MAX      10
#define SNTP_UPDATE_INTERVAL (NX_IP_PERIODIC_RATE / 2)

 /* Default time. GMT: Friday, Jan 1, 2022 12:00:00 AM. Epoch timestamp: 1640995200.  */
#define DEFAULT_SYSTEM_TIME 1640995200

 /* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define UNIX_TO_NTP_EPOCH_SECOND 0x83AA7E80

/* Threads configuration */
#define PAYLOAD_SIZE              1544
#define NX_PACKET_POOL_SIZE       ((PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)
#define DEFAULT_IP_STACK_SIZE     1024
#define ARP_MEMORY_SIZE           DEFAULT_IP_STACK_SIZE
#define DEFAULT_IP_STACK_PRIORITY 10
#define DEFAULT_PRIORITY          5
#define THREAD_MEMORY_SIZE        4 * DEFAULT_IP_STACK_SIZE

#define NULL_ADDRESS     0
#define USER_DNS_ADDRESS IP_ADDRESS(1, 1, 1, 1) /* User should configure it with his DNS address */

#define DEFAULT_TIMEOUT 5 * NX_IP_PERIODIC_RATE
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define PRINT_IP_ADDRESS(addr)           do { \
                                              printf("STM32 %s: %lu.%lu.%lu.%lu \n", #addr, \
                                                (addr >> 24) & 0xff,                        \
                                                  (addr >> 16) & 0xff,                      \
                                                    (addr >> 8) & 0xff,                     \
                                                      (addr & 0xff));                       \
                                            } while(0)      
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
