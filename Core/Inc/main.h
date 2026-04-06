/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EPD_ENABLE_Pin GPIO_PIN_2
#define EPD_ENABLE_GPIO_Port GPIOA
#define EPD_CS_Pin GPIO_PIN_3
#define EPD_CS_GPIO_Port GPIOA
#define EPD_DC_Pin GPIO_PIN_4
#define EPD_DC_GPIO_Port GPIOA
#define EPD_RST_Pin GPIO_PIN_5
#define EPD_RST_GPIO_Port GPIOA
#define EPD_BUSY_Pin GPIO_PIN_6
#define EPD_BUSY_GPIO_Port GPIOA
#define LED_Y_Pin GPIO_PIN_0
#define LED_Y_GPIO_Port GPIOB
#define nPGOOD_Pin GPIO_PIN_8
#define nPGOOD_GPIO_Port GPIOA
#define BUTTON_Pin GPIO_PIN_12
#define BUTTON_GPIO_Port GPIOA
#define BUTTON_EXTI_IRQn EXTI4_15_IRQn
#define LED_R_Pin GPIO_PIN_4
#define LED_R_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_5
#define LED_G_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define DEBUG_PRINT 1
//#define DEBUG_NO_SENSORS 1
//#define DEBUG_NO_EPD 1

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
