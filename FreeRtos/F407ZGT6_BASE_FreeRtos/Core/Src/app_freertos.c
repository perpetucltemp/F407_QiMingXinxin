/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : APP_FREERTOS.c
  * @brief          : Main program body
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
#include "app_freertos.h"
#include "string.h"
#include "usart.h"




#define  QUEUE_DELAY  20           /* 队列等待时间 */

#define  UART_TX_QUEUE_SIZE   sizeof(uart_tx_request_t)   /* 队列中每个消息大小（字节） */
#define  UART_TX_QUEUE_LEN    10   /* UART发送请求队列长度 */

#define  UART_RX_QUEUE_SIZE   sizeof(uint8_t)   /* 队列中每个消息大小（字节） */
#define  UART_RX_QUEUE_LEN    BUFFSIZE   /* UART发送请求队列长度 */


#define  COUNTSEM_NUM  5
#define  COUNTSEM_DEFAULT  5

#define  KEY1_EVENT  (0x01 << 0)//设置事件掩码的位0
#define  KEY2_EVENT  (0x01 << 1)//设置事件掩码的位1

// UART传输请求类型枚举
typedef enum
{
  UART_TX_TYPE_MALLOC_INFO = 0,       // 内存分配信息
  UART_TX_TYPE_RX_DATA,
  UART_TX_TYPE_CUSTOM_DATA       // 自定义数据
} uart_tx_type_t;

// UART传输请求结构体
typedef struct
{
  uart_tx_type_t type;           // 数据类型
  void* data;                    // 数据指针
  uint16_t length;               // 数据长度
} uart_tx_request_t;

/* 任务句柄 */
static TaskHandle_t UART_SEND_Task_Handle      = NULL;
static TaskHandle_t AppTaskCreate_Handle       = NULL;
static TaskHandle_t UART_RECEIVE_Task_Handle   = NULL;


static SemaphoreHandle_t  BinarySem_Handle       =  NULL;
static SemaphoreHandle_t  CountSem_Handle        =  NULL;
static SemaphoreHandle_t  MuxSem_Handle          =  NULL;
static QueueHandle_t      Uart_Tx_Request_Queue  =  NULL;
static EventGroupHandle_t Event_Handle           =  NULL;
SemaphoreHandle_t  BinaryRxSem_Handle       =  NULL;
QueueHandle_t      Uart_Rx_Request_Queue    =  NULL;


void SEND_OULA_ANGLE(short row,short pit,short yaw);



void uart_tx_thread(void* arg)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  uart_tx_request_t tx_request;

  while(1)
  {
    xReturn = xQueueReceive( Uart_Tx_Request_Queue,    /* 消息队列的句柄 */
                             &tx_request,      /* 发送的消息内容 */
                             portMAX_DELAY); /* 等待时间 */
    if(pdPASS == xReturn)
    {
      switch(tx_request.type)
      {
      case UART_TX_TYPE_MALLOC_INFO:
      {
        if(tx_request.data != NULL)
        {
          uint32_t malloc_size = *((uint32_t*)tx_request.data);
          printf("malloc : %d KB!\r\n", malloc_size);
          // 打印实际heap大小
          printf("Configured heap size: %d bytes\r\n", configTOTAL_HEAP_SIZE);
          printf("Current Handle: %s!\r\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
          vPortFree(tx_request.data);
        }
        break;
      }
      case UART_TX_TYPE_RX_DATA:
      {
        if(tx_request.data != NULL)
        {
          printf("Receive data:\n%s!\r\n", (uint8_t *)tx_request.data);
          vPortFree(tx_request.data);
        }
//          volatile uint32_t test = 0x99;
//          test = *(uint32_t*)0x56785678;
        break;
      }
      default:
        printf("Unknown TX request type: %d !\r\n", tx_request.type);
        break;
      }
    }
  }
}

void uart_rx_thread(void* arg)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  uint8_t receive[BUFFSIZE+1] = {0};
  uint8_t byte;
  uint16_t index = 0;

  while(1)
  {
    //获取二值信号量 xSemaphore,没获取到则一直等待
    xReturn = xSemaphoreTake(BinaryRxSem_Handle,/* 二值信号量句柄 */
                             portMAX_DELAY); /* 等待时间 */
    if(pdPASS == xReturn)
    {
      index = 0;
      // 从队列中读取所有字节
      while(xQueueReceive(Uart_Rx_Request_Queue, &byte, 0) == pdTRUE && index < BUFFSIZE)
      {
        receive[index++] = byte;
      }
      receive[index] = '\0';

      //数据解析

      // 创建发送请求
      uart_tx_request_t tx_request;
      tx_request.type = UART_TX_TYPE_RX_DATA;
      tx_request.data = pvPortMalloc(index + 1);
      tx_request.length = index;
      if(tx_request.data == NULL)
      {
        printf("%s Malloc error!\r\n", __func__);
        continue;
      }
      memcpy(tx_request.data, receive, index + 1);
      memset(receive, 0x00, sizeof(receive));
      // 将请求发送到统一的发送队列
      xReturn = xQueueSend(Uart_Tx_Request_Queue,
                            &tx_request,
                            portMAX_DELAY);
      if(pdPASS != xReturn)
      {
        printf("%s send error!\r\n", __func__);
        vPortFree(tx_request.data);
      }
    }
  }

}

/**
  * 函数功能: 创建任务应用
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
static void AppTaskCreate (void)
{
  static BaseType_t xReturn = NULL;
  taskENTER_CRITICAL();           //进入临界区

  /* 创建 BinarySem */
  BinarySem_Handle = xSemaphoreCreateBinary();
  if(NULL == BinarySem_Handle)
    printf("BinarySem_Handle二值信号量创建失败！\r\n");

  /* 创建发送 BinaryRxSem */
  BinaryRxSem_Handle = xSemaphoreCreateBinary();
  if(NULL == BinaryRxSem_Handle)
    printf("BinaryRxSem_Handle二值信号量创建失败！\r\n");

  /* 创建MuxSem */
  MuxSem_Handle = xSemaphoreCreateMutex();
  if(NULL == MuxSem_Handle)
    printf("MuxSem_Handle 互斥量创建失败！");//MuxSem_Handle互斥量创建失败

  xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量

  /* 创建CountSem */
  CountSem_Handle = xSemaphoreCreateCounting(COUNTSEM_NUM, COUNTSEM_DEFAULT);
  if(NULL == CountSem_Handle)
    printf("CountSem_Handle 计数信号量创建失败！\r\n");//

  /* 创建 UART 发送请求队列 */
  Uart_Tx_Request_Queue = xQueueCreate((UBaseType_t ) UART_TX_QUEUE_LEN,
                                       (UBaseType_t ) UART_TX_QUEUE_SIZE);
  if(NULL == Uart_Tx_Request_Queue)
    printf("Uart_Tx_Request_Queue 消息队列创建失败！\r\n");

  Uart_Rx_Request_Queue = xQueueCreate((UBaseType_t ) UART_RX_QUEUE_LEN,
                                       (UBaseType_t ) UART_RX_QUEUE_SIZE);
  if(NULL == Uart_Rx_Request_Queue)
    printf("Uart_Rx_Request_Queue 消息队列创建失败！\r\n");


  /* 创建 Event_Handle */
  Event_Handle = xEventGroupCreate();
  if(NULL == Event_Handle)
    printf("Event_Handle 事件创建失败！\r\n");


  /* 创建UART_TX_Task任务 */
  xReturn=xTaskCreate( uart_tx_thread,           /* 任务函数  */
                       "vTaskUartTx",         /* 任务名    */
                       2048,                 /* 任务栈大小，单位word，也就是4字节 */
                       NULL,                /* 任务参数  */
                       6,                   /* 任务优先级*/
                       &UART_SEND_Task_Handle );  /* 任务句柄  */
  if(xReturn != pdPASS)
  {
    printf("UART_SEND_Task_Handle 创建失败！\r\n");//创建成功  指示灯显示
  }

  /* 创建UART_RX_Task任务 */
  xReturn=xTaskCreate( uart_rx_thread,           /* 任务函数  */
                       "vTaskUartRx",         /* 任务名    */
                       2048,                 /* 任务栈大小，单位word，也就是4字节 */
                       NULL,                /* 任务参数  */
                       6,                   /* 任务优先级*/
                       &UART_RECEIVE_Task_Handle );  /* 任务句柄  */
  if(xReturn != pdPASS)
  {
    printf("UART_RECEIVE_Task_Handle 创建失败！\r\n");//创建成功  指示灯显示
  }

  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务 ,只执行一次的任务及时删除

  taskEXIT_CRITICAL();            //退出临界区

}

void APP_Create(void)
{
  static BaseType_t xReturn;
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */
  /* 启动任务调度 */
  if(pdPASS == xReturn)
  {
    printf("Starting !\r\n");
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  }
  else
    printf("Start ERROR！\r\n");
}

//向上位机发送欧拉角
//详情参考匿名通讯协议
void SEND_OULA_ANGLE(short row,short pit,short yaw)
{
  unsigned char i;
  unsigned char sumcheck = 0;
  unsigned char addcheck = 0;
  unsigned char buf[13]= {0};

  buf[0]=0xaa;
  buf[1]=0xff;
  buf[2]=0x03;
  buf[3]=0x07;

  buf[4]=(unsigned char)row;
  buf[5]=(unsigned char)(row>>8);

  buf[6]=(unsigned char)pit;
  buf[7]=(unsigned char)(pit>>8);

  buf[8]=(unsigned char)yaw;
  buf[9]=(unsigned char)(yaw>>8);

  buf[10]=0x00;

  for(i=0; i < (buf[3]+4); i++)
  {
    sumcheck += buf[i]; //从帧头开始，对每一字节进行求和，直到DATA区结束
    addcheck += sumcheck; //每一字节的求和操作，进行一次sumcheck的累加
  }
  buf[11]=sumcheck;
  buf[12]=addcheck;

  HAL_UART_Transmit( &huart6,buf, 13, 0xffff);


}


