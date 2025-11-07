/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "WF5803F.h"
#include "usart.h"
#include "NTC.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId MonitorTaskHandle;
osThreadId VoltageWarningHandle;
osThreadId Receive_Target_Handle;
osThreadId Ctrl_taskHandle;
osMessageQId usart2_rx_queueHandle;
osMessageQId usart1_rx_queueHandle;
osMutexId usart2_rx_mutexesHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartMonitorTask(void const * argument);
void StartvoltageWarningtask(void const * argument);
void StartReceive_Target_change(void const * argument);
void StartCtrl_task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of usart2_rx_mutexes */
  osMutexDef(usart2_rx_mutexes);
  usart2_rx_mutexesHandle = osMutexCreate(osMutex(usart2_rx_mutexes));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of usart2_rx_queue */
  osMessageQDef(usart2_rx_queue, 16, uint32_t);
  usart2_rx_queueHandle = osMessageCreate(osMessageQ(usart2_rx_queue), NULL);

  /* definition and creation of usart1_rx_queue */
  osMessageQDef(usart1_rx_queue, 16, uint32_t);
  usart1_rx_queueHandle = osMessageCreate(osMessageQ(usart1_rx_queue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityRealtime, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of MonitorTask */
  osThreadDef(MonitorTask, StartMonitorTask, osPriorityHigh, 0, 256);
  MonitorTaskHandle = osThreadCreate(osThread(MonitorTask), NULL);

  /* definition and creation of VoltageWarning */
  osThreadDef(VoltageWarning, StartvoltageWarningtask, osPriorityLow, 0, 256);
  VoltageWarningHandle = osThreadCreate(osThread(VoltageWarning), NULL);

  /* definition and creation of Receive_Target_ */
  osThreadDef(Receive_Target_, StartReceive_Target_change, osPriorityLow, 0, 256);
  Receive_Target_Handle = osThreadCreate(osThread(Receive_Target_), NULL);

  /* definition and creation of Ctrl_task */
  osThreadDef(Ctrl_task, StartCtrl_task, osPriorityHigh, 0, 512);
  Ctrl_taskHandle = osThreadCreate(osThread(Ctrl_task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  // 初始化外设
  osDelay(10); // 确保系统稳定
  // 发送启动消息（使用非阻塞方式，但此时已经没有竞争）
  send_message("system start\n");
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);//打开串口2接收中断，接收到的数据放入rx_byte变量，相当于初始化接收
  NTC_Init();//初始化NTC模块
  

  
  // 再延迟一下确保消息发送
  osDelay(50);
  
  /* Infinite loop */
  for(;;)
  { //删除自己CMSIS_V1接口
    osThreadTerminate(NULL);
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartMonitorTask */
/**
* @brief Function implementing the MonitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void const * argument)
{
  /* USER CODE BEGIN StartMonitorTask */
  float temperature;
  float pressure;
  
  // 等待DefaultTask完成初始化和发送启动消息
  osDelay(100);
  
  NTC_StartDMA();//启动NTC的ADC DMA转换
  /* Infinite loop */
  for(;;)
  { 
    #if WF5803F_Enable
    WF5803F_GetData(&temperature, &pressure);//读取WF5803F传感器数据
    #endif

    NTC_Calculate(&NTC_DataBuffer);//计算温度
    //判断宏定义来发送信息
    #if NTC_CHANNEL0_ENABLE
    send_message("{\"type\":\"data\",\"sensor\":\"NTC_0\",\"temp\":%.2f}\n", NTC_DataBuffer.temperature_ch0);
    #endif
    #if NTC_CHANNEL1_ENABLE
    send_message("{\"type\":\"data\",\"sensor\":\"NTC_1\",\"temp\":%.2f}\n", NTC_DataBuffer.temperature_ch1);
    #endif
    //发送WF5803F数据
    #if WF5803F_Enable
    send_message("{\"type\":\"data\",\"sensor\":\"WF5803F\",\"temp\":%.2f,\"pressure\":%.2f}\n", temperature, pressure);
    #endif
    NTC_StartDMA();//数据利用完毕重新启动DMA转换
    //绝对时间控制延迟
    osDelay(100);
  }
  /* USER CODE END StartMonitorTask */
}

/* USER CODE BEGIN Header_StartvoltageWarningtask */
/**
* @brief Function implementing the VoltageWarning thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartvoltageWarningtask */
void StartvoltageWarningtask(void const * argument)
{
  /* USER CODE BEGIN StartvoltageWarningtask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartvoltageWarningtask */
}

/* USER CODE BEGIN Header_StartReceive_Target_change */
/**
* @brief Function implementing the Receive_Target_ thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReceive_Target_change */
void StartReceive_Target_change(void const * argument)
{
  /* USER CODE BEGIN StartReceive_Target_change */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartReceive_Target_change */
}

/* USER CODE BEGIN Header_StartCtrl_task */
/**
* @brief Function implementing the Ctrl_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCtrl_task */
void StartCtrl_task(void const * argument)
{
  /* USER CODE BEGIN StartCtrl_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCtrl_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */