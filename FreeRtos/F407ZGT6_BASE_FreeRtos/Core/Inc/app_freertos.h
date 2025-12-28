/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif



extern SemaphoreHandle_t  BinaryRxSem_Handle;
extern QueueHandle_t      Uart_Rx_Request_Queue;

extern void APP_Create(void);


#ifdef __cplusplus
}
#endif

#endif /* __APP_FREERTOS_H */
