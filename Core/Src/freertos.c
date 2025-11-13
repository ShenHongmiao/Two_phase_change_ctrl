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
#include "V_Detect.h"
#include "data_packet.h"
#include "serial_to_pc.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for MonitorTask */
osThreadId_t MonitorTaskHandle;
const osThreadAttr_t MonitorTask_attributes = {
  .name = "MonitorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for VoltageWarning */
osThreadId_t VoltageWarningHandle;
const osThreadAttr_t VoltageWarning_attributes = {
  .name = "VoltageWarning",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Receive_Target_ */
osThreadId_t Receive_Target_Handle;
const osThreadAttr_t Receive_Target__attributes = {
  .name = "Receive_Target_",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for Ctrl_task */
osThreadId_t Ctrl_taskHandle;
const osThreadAttr_t Ctrl_task_attributes = {
  .name = "Ctrl_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for usart2_rx_queue */
osMessageQueueId_t usart2_rx_queueHandle;
const osMessageQueueAttr_t usart2_rx_queue_attributes = {
  .name = "usart2_rx_queue"
};
/* Definitions for usart1_rx_queue */
osMessageQueueId_t usart1_rx_queueHandle;
const osMessageQueueAttr_t usart1_rx_queue_attributes = {
  .name = "usart1_rx_queue"
};
/* Definitions for usart2_rx_mutexes */
osMutexId_t usart2_rx_mutexesHandle;
const osMutexAttr_t usart2_rx_mutexes_attributes = {
  .name = "usart2_rx_mutexes"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartMonitorTask(void *argument);
void StartvoltageWarningtask(void *argument);
void StartReceive_Target_change(void *argument);
void StartCtrl_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of usart2_rx_mutexes */
  usart2_rx_mutexesHandle = osMutexNew(&usart2_rx_mutexes_attributes);

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
  /* creation of usart2_rx_queue */
  usart2_rx_queueHandle = osMessageQueueNew (16, sizeof(uint32_t), &usart2_rx_queue_attributes);

  /* creation of usart1_rx_queue */
  usart1_rx_queueHandle = osMessageQueueNew (16, sizeof(uint32_t), &usart1_rx_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MonitorTask */
  MonitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &MonitorTask_attributes);

  /* creation of VoltageWarning */
  VoltageWarningHandle = osThreadNew(StartvoltageWarningtask, NULL, &VoltageWarning_attributes);

  /* creation of Receive_Target_ */
  Receive_Target_Handle = osThreadNew(StartReceive_Target_change, NULL, &Receive_Target__attributes);

  /* creation of Ctrl_task */
  Ctrl_taskHandle = osThreadNew(StartCtrl_task, NULL, &Ctrl_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  // 其他任务创建后立即挂起，等待初始化完成
  osThreadSuspend(MonitorTaskHandle);
  osThreadSuspend(VoltageWarningHandle);
  osThreadSuspend(Receive_Target_Handle);
  osThreadSuspend(Ctrl_taskHandle);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  // 初始化外设
  osDelay(10); // 确保系统稳定
  // 发送启动消息（使用非阻塞方式，但此时已经没有竞争）
  //send_ready(CMD_TEXT_INFO, "system start\n"); // 发送时间: ~1.56ms @115200 (18字节)
  send2pc(CMD_TEXT_INFO, NULL, "system start\n");
  //HAL_UART_Receive_IT(&huart2, &rx_byte, 1);//打开串口2接收中断，接收到的数据放入rx_byte变量，相当于初始化接收

  #if WF5803F_Enable
  WF5803F_Init();//初始化WF5803F模块
  #endif
  NTC_Init();//初始化NTC模块
  Voltage_Init();//初始化电压检测模块

  // 再延迟一下确保消息发送
  osDelay(50);
  
  /* Infinite loop */
  for(;;)
  { 
    // 恢复其他任务
    osThreadResume(MonitorTaskHandle);
    osThreadResume(VoltageWarningHandle);
    osThreadResume(Receive_Target_Handle);
    osThreadResume(Ctrl_taskHandle);
    
    //删除自己CMSIS_V1接口
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
void StartMonitorTask(void *argument)
{
  /* USER CODE BEGIN StartMonitorTask */
  
  
  /* Infinite loop */
  for(;;)
  { 

    NTC_StartDMA();//数据利用完毕重新启动DMA转换
    HAL_ADC_PollForConversion(&hadc1, 1); //死等转换完成，确保数据有效(osdelay会打断任务执行逻辑)
    #if WF5803F_Enable
    WF5803F_ReadData(&WF5803F_DataBuffer); // 读取原始数据
    WF5803F_Calculate(&WF5803F_DataBuffer); // 计算温度和压力
    #endif

    NTC_Calculate(&NTC_DataBuffer);//计算温度

    //打包数据（会根据宏定义来自行打包该打包的数据）
    pack_data(&packet_data);
    //判断宏定义来发送信息
    #if NTC_CHANNEL0_ENABLE
    //send_ready(CMD_NTC, "CH0:%.2f\n", packet_data.ntc_temp_ch0); // 发送时间: ~1.30ms @115200 (15字节)
  send2pc(CMD_NTC, &packet_data, NULL);
    #endif
    #if NTC_CHANNEL1_ENABLE
    //send_ready(CMD_NTC, "CH1:%.2f\n", packet_data.ntc_temp_ch1); // 发送时间: ~1.30ms @115200 (15字节)
    
    #endif
    //发送WF5803F数据
    #if WF5803F_Enable
    //send_ready(CMD_WF5803F, "T:%.2f,P:%.2f\n", packet_data.wf_temperature, packet_data.wf_pressure); // 发送时间: ~2.00ms @115200 (23字节)
  send2pc(CMD_WF5803F, &packet_data, NULL);
    #endif

   
    
    //延迟时间注意要大于ADC转换时间（大概 100ns）+消息发送时间，否则会出现发送信息重叠的问题，以及ADC数据错乱的问题（目前双传感器不带PID发送已经测试最小间隔5ms）
    osDelay(1000);
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
void StartvoltageWarningtask(void *argument)
{
  /* USER CODE BEGIN StartvoltageWarningtask */
    Voltage_StartDMA();//启动电压检测的ADC DMA转换
    osDelay(1); // 等待ADC转换完成（1ms足够，实际只需约100μs）
    
    Voltage_Calculate(&Voltage_DataBuffer);//计算电源电压
    pack_data(&packet_data);
  send2pc(CMD_VOLTAGE, &packet_data, NULL);
    //Voltage_info_send(&Voltage_DataBuffer);//发送电源电压信息: ~1.30ms(OK) 或 ~1.39ms(LOW) @115200
    // send_message("Voltage Check Done\n");//发送电源电压检查完成消息，约1.20ms @115200
  /* Infinite loop */
  for(;;)
  { osDelay(600000);//每10分钟检查一次电压
    Voltage_StartDMA();//启动电压检测的ADC DMA转换
    osDelay(1); // 等待ADC转换完成（1ms足够，实际只需约100μs）
    
    Voltage_Calculate(&Voltage_DataBuffer);//计算电源电压
  send2pc(CMD_VOLTAGE, &packet_data, NULL);
    //Voltage_info_send(&Voltage_DataBuffer);//发送电源电压信息: ~1.30ms(OK) 或 ~1.39ms(LOW) @115200
    //send_message("Voltage Check Done\n");//发送电源电压检查完成消息，约1.20ms @115200
    
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
void StartReceive_Target_change(void *argument)
{
  /* USER CODE BEGIN StartReceive_Target_change */
  uint32_t received_value;
  osStatus_t status;
  
  send_message("=== USART Receive Task Started (Priority: Realtime) ===\n");
  /* Infinite loop */
  for(;;)
  {
    // 阻塞读取队列，永久等待直到有数据到来 (CMSIS V2 API)
    status = osMessageQueueGet(usart2_rx_queueHandle, &received_value, NULL, osWaitForever);
    // ========== 在此处添加命令处理逻辑 ==========
    if (status == osOK) {
      // 从队列中获取环形缓冲池中的缓冲区指针
      uint8_t *received_data = (uint8_t *)received_value;
      
      // 将接收到的数据以文本形式发送到上位机进行验证
      // 假设接收到的是文本数据，使用字符串格式发送
      send2pc(CMD_TEXT_INFO, NULL, "Received: %s\n", (char *)received_data);
      
      // 注意：不需要手动清空缓冲区，下次中断会自动清空并复用
    }
    
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
void StartCtrl_task(void *argument)
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

