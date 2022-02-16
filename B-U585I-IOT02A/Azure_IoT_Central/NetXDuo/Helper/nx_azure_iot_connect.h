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
#ifndef __NX_AZURE_IOT_CONNECT_H__
#define __NX_AZURE_IOT_CONNECT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nx_api.h"

#include "nx_azure_iot_client.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
VOID connection_status_set(AZURE_IOT_CONTEXT* context, UINT connection_status);

VOID connection_monitor(AZURE_IOT_CONTEXT* context, UINT (*iothub_init)(AZURE_IOT_CONTEXT* context));
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __NX_AZURE_IOT_CONNECT_H__ */
