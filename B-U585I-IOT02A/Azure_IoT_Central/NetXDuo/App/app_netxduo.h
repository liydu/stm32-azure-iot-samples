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

#include "nxd_dhcp_client.h"
#include "nxd_dns.h"
#include "nxd_sntp_client.h"
#include "nx_secure_tls_api.h"

#include "nx_driver_emw3080.h"

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_provisioning_client.h"

#include "nxd_mqtt_client.h" 
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
#define DEFAULT_SYSTEM_TIME       1640995200

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define UNIX_TO_NTP_EPOCH_SECOND  0x83AA7E80



#define MOSQUITTO_CERT_FILE         "mosquitto.cert.h"
  
  /* Threads configuration */  
#define PAYLOAD_SIZE                1544
#define NX_PACKET_POOL_SIZE         (( PAYLOAD_SIZE + sizeof(NX_PACKET)) * 10)  
#define DEFAULT_MEMORY_SIZE         1024
#define ARP_MEMORY_SIZE             DEFAULT_MEMORY_SIZE   
#define DEFAULT_MAIN_PRIORITY       10
#define DEFAULT_PRIORITY            5  
#define THREAD_MEMORY_SIZE          4 * DEFAULT_MEMORY_SIZE  

/* MQTT Client configuration */
#define MQTT_CLIENT_STACK_SIZE      1024 * 10
#define CLIENT_ID_STRING            "MQTT_client_ID"  
#define MQTT_THREAD_PRIORTY         2
#define MQTT_KEEP_ALIVE_TIMER       30000                /* Define the MQTT keep alive timer for 5 minutes */
#define CLEAN_SESSION               NX_TRUE
#define STRLEN(p)                   (sizeof(p) - 1)  
                                    
#define TOPIC_NAME                  "Temperature" 
#define NB_MESSAGE                  10                    /*  if NB_MESSAGE = 0, client will publish messages infinitely */
                                    
#define MQTT_BROKER_NAME            "test.mosquitto.org" /* MQTT Server */
                           
#define MQTT_PORT                   NXD_MQTT_TLS_PORT                             
                                    
#define QOS0                        0
#define QOS1                        1 
                                    
#define DEMO_MESSAGE_EVENT          1 
#define DEMO_ALL_EVENTS             3
                                    
#define NULL_ADDRESS                0  
#define USER_DNS_ADDRESS            IP_ADDRESS(1, 1, 1, 1)   /* User should configure it with his DNS address */

#define DEFAULT_TIMEOUT             5 * NX_IP_PERIODIC_RATE
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
