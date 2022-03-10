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
#ifndef __PNP_DEVICE_INFO_H__
#define __PNP_DEVICE_INFO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

#define DEVICE_INFO_COMPONENT_NAME "deviceInformation"

// Device Info property names
#define DEVICE_INFO_MANUFACTURER_PROPERTY_NAME           "manufacturer"
#define DEVICE_INFO_MODEL_PROPERTY_NAME                  "model"
#define DEVICE_INFO_SW_VERSION_PROPERTY_NAME             "swVersion"
#define DEVICE_INFO_OS_NAME_PROPERTY_NAME                "osName"
#define DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_NAME "processorArchitecture"
#define DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_NAME "processorManufacturer"
#define DEVICE_INFO_TOTAL_STORAGE_PROPERTY_NAME          "totalStorage"
#define DEVICE_INFO_TOTAL_MEMORY_PROPERTY_NAME           "totalMemory"

// Device Info property values
#define DEVICE_INFO_MANUFACTURER_PROPERTY_VALUE           "STMicroelectronics"
#define DEVICE_INFO_MODEL_PROPERTY_VALUE                  "B-U585I-IOT02A"
#define DEVICE_INFO_SW_VERSION_PROPERTY_VALUE             "1.0.0"
#define DEVICE_INFO_OS_NAME_PROPERTY_VALUE                "Azure RTOS"
#define DEVICE_INFO_PROCESSOR_ARCHITECTURE_PROPERTY_VALUE "ARM Cortex M33"
#define DEVICE_INFO_PROCESSOR_MANUFACTURER_PROPERTY_VALUE "STMicroelectronics"
#define DEVICE_INFO_TOTAL_STORAGE_PROPERTY_VALUE          2048
#define DEVICE_INFO_TOTAL_MEMORY_PROPERTY_VALUE           768

  /* USER CODE END EC */

  /* Exported macro ------------------------------------------------------------*/
  /* USER CODE BEGIN EM */

  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
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
#endif /* __PNP_DEVICE_INFO_H__ */
