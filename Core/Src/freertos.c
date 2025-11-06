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
osThreadId Sensors_computeHandle;
osThreadId VoltageMonitorHandle;
osThreadId ReceiveAndTargeHandle;
osMessageQId usart2_rx_queueHandle;
osMessageQId usart1_rx_queueHandle;
osMutexId usart2_rx_mutexesHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartSensors_compute(void const * argument);
void StartVoltageMonitor(void const * argument);
void StartReceiveAndTarge(void const * argument);

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

  /* definition and creation of Sensors_compute */
  osThreadDef(Sensors_compute, StartSensors_compute, osPriorityHigh, 0, 512);
  Sensors_computeHandle = osThreadCreate(osThread(Sensors_compute), NULL);

  /* definition and creation of VoltageMonitor */
  osThreadDef(VoltageMonitor, StartVoltageMonitor, osPriorityLow, 0, 256);
  VoltageMonitorHandle = osThreadCreate(osThread(VoltageMonitor), NULL);

  /* definition and creation of ReceiveAndTarge */
  osThreadDef(ReceiveAndTarge, StartReceiveAndTarge, osPriorityIdle, 0, 256);
  ReceiveAndTargeHandle = osThreadCreate(osThread(ReceiveAndTarge), NULL);

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
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  /* Infinite loop */
  for(;;)
  { //删除自己CMSIS_V1接口
    osThreadTerminate(NULL);
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartSensors_compute */
/**
* @brief Function implementing the Sensors_compute thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensors_compute */
void StartSensors_compute(void const * argument)
{
  /* USER CODE BEGIN StartSensors_compute */
  float temperature = 0.0f;
  float pressure = 0.0f;
  send_message("=== Sensors_and_compute Task Started! ===\n");
  osDelay(10);
  /* Infinite loop */
  for(;;)
  {
    // 读取温度和气压数据（使用DMA方式，非阻塞）
    WF5803F_GetData(&temperature, &pressure);
    
    // 检查是否有错误
    if (g_i2c_rx_status == I2C_DMA_ERROR || g_i2c_tx_status == I2C_DMA_ERROR) {
      // 错误处理：可以记录日志、重试或其他操作
      //阻塞发送，HAL
      send_message("WF5803F I2C DMA Error!\n");
      // 重置状态标志，准备下次读取
      g_i2c_rx_status = I2C_DMA_IDLE;
      g_i2c_tx_status = I2C_DMA_IDLE;
    } else {
      //重置状态标志，准备下次读取
      //g_i2c_rx_status = I2C_DMA_IDLE;
      g_i2c_tx_status = I2C_DMA_IDLE;
      // 数据读取成功，可以在这里使用temperature和pressure
      // 例如：发送到串口、存储、控制逻辑等
     send_message("{\"type\":\"data\",\"sensor\":\"WF5803\",\"temp\":%.2f,\"press\":%.2f}\n", temperature, pressure);
      // 示例：通过串口打印（如果需要的话）
      // printf("Temp: %.2f °C, Press: %.2f kPa\r\n", temperature, pressure);
    }
    
    // 可以根据需要调整读取频率，最小间隔5ms
    osDelay(100);
  }
  /* USER CODE END StartSensors_compute */
}

/* USER CODE BEGIN Header_StartVoltageMonitor */
/**
* @brief Function implementing the VoltageMonitor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartVoltageMonitor */
void StartVoltageMonitor(void const * argument)
{
  /* USER CODE BEGIN StartVoltageMonitor */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartVoltageMonitor */
}

/* USER CODE BEGIN Header_StartReceiveAndTarge */
/**
* @brief Function implementing the ReceiveAndTarge thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartReceiveAndTarge */
void StartReceiveAndTarge(void const * argument)
{
  /* USER CODE BEGIN StartReceiveAndTarge */
  osEvent event;
  uint8_t received_byte;
  send_message("=== USART Receive Task Started (Priority: Realtime) ===\n");
  /* Infinite loop */
  for(;;)
  {    // 阻塞读取队列，永久等待直到有数据到来
    event = osMessageGet(usart2_rx_queueHandle, osWaitForever);
    
    if (event.status == osEventMessage) {
      // 成功接收到数据
      received_byte = (uint8_t)event.value.v;
      
      // 发送接收到的数据信息
      send_message("Received byte from USART2: '%c' (0x%02X)\n", 
                   received_byte, received_byte);
      // ========== 在此处添加命令处理逻辑 ==========
    
      }
    
    // 注意：不需要 osDelay，因为 osMessageGet 本身就是阻塞的
    // 当没有数据时，任务会自动进入阻塞状态，让出 CPU
    osDelay(1);
  }
  /* USER CODE END StartReceiveAndTarge */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
