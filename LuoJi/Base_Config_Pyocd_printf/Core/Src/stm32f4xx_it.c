/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart6;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  __asm volatile
  (
    "TST lr, #4\n"              // 判断当前使用的是MSP还是PSP
    "ITE EQ\n"                  // 条件执行指令
    "MRSEQ r0, MSP\n"           // 如果是MSP，将MSP值放入r0
    "MRSNE r0, PSP\n"           // 如果是PSP，将PSP值放入r0
    "B HardFault_Handler_C\n"   // 跳转到C函数处理
    :
    :
    : "r0"
  );
}


void HardFault_Handler_C(uint32_t *hardfault_args)
{
    uint32_t stacked_r0 = hardfault_args[0];
    uint32_t stacked_r1 = hardfault_args[1];
    uint32_t stacked_r2 = hardfault_args[2];
    uint32_t stacked_r3 = hardfault_args[3];
    uint32_t stacked_r12 = hardfault_args[4];
    uint32_t stacked_lr = hardfault_args[5];
    uint32_t stacked_pc = hardfault_args[6];
    uint32_t stacked_psr = hardfault_args[7];

    // 获取其他故障相关信息
    uint32_t cfsr = SCB->CFSR;    // Configurable Fault Status Register
    uint32_t hfsr = SCB->HFSR;    // HardFault Status Register
    uint32_t dfsr = SCB->DFSR;    // Debug Fault Status Register
    uint32_t afsr = SCB->AFSR;    // Auxiliary Fault Status Register
    uint32_t mmfar = SCB->MMFAR;  // MemManage Fault Address Register
    uint32_t bfar = SCB->BFAR;    // BusFault Address Register

    char msg[512];  // 增加缓冲区大小以容纳更多信息
    int len = snprintf(msg, sizeof(msg),
        "\n*** HARD FAULT DETECTED ***\n"
        "=== Saved Registers ===\n"
        "R0:  0x%08X    R1:  0x%08X\n"
        "R2:  0x%08X    R3:  0x%08X\n"
        "R12: 0x%08X    LR:  0x%08X\n"
        "PC:  0x%08X    PSR: 0x%08X\n"
        "=== Fault Status ===\n"
        "CFSR: 0x%08X    HFSR: 0x%08X\n"
        "MMFAR:0x%08X    BFAR: 0x%08X\n"
        "DFSR: 0x%08X    AFSR: 0x%08X\n"
        "*************************\n",
        stacked_r0, stacked_r1, stacked_r2, stacked_r3,
        stacked_r12, stacked_lr, stacked_pc, stacked_psr,
        cfsr, hfsr, mmfar, bfar, dfsr, afsr
    );

    // 输出到SEGGER RTT（调试更高效）
#ifdef SEGGER_RTT
    #include "SEGGER_RTT.h"
    LOG_ERR(msg);  // 修复：使用msg而不是buffer，并使用len而不是strlen(msg)
#elif defined(HAL_UART_MODULE_ENABLED)
    // 输出到串口（需确保huart已初始化，这里用USART6示例）
    #include "usart.h"
    HAL_UART_Transmit(&huart6, (uint8_t*)msg, len, HAL_MAX_DELAY);  // 使用len而不是strlen(msg)
    HAL_Delay(100);
#endif

    while(1);
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  time_count++;

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */
  if(__HAL_UART_GET_FLAG(&huart6, UART_FLAG_IDLE ) != RESET) //中断中判断产生了空闲中断标志认为一轮结束
  {
    Rxflag = 1;
    Rxlen += BUFFSIZE - huart6.RxXferCount;    //累计达不到BUFFSIZE个数的部分
//
//		printf("RxXferCount:%d\r\n",huart6.RxXferCount);
    __HAL_UART_CLEAR_IDLEFLAG(&huart6);
//    HAL_UART_AbortReceive_IT(&huart6);

    return ;
  }

  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
