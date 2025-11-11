/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "FreeRTOS.h"
#define UART_TX_BUFFER_SIZE 256

// 双缓冲区A和B
static char buffer_a[UART_TX_BUFFER_SIZE];
static char buffer_b[UART_TX_BUFFER_SIZE];

// DMA发送忙碌标志，0=空闲，1=忙碌
static volatile uint8_t is_tx_busy = 0;
// 空闲缓冲区中待发送的数据长度
static volatile uint16_t idle_buf_len = 0;
// 当前DMA正在使用的缓冲区索引，0=buffer_a，1=buffer_b
static volatile uint8_t active_buf_index = 0;

// 互斥锁，保护缓冲区切换和状态变量
static osMutexId uart_tx_mutex = NULL;

uint8_t rx_byte; // 用于接收单个字节

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */
  // 设置MD0(PD3)输出低电平
  HAL_GPIO_WritePin(GPIOD, MD0_Pin, GPIO_PIN_RESET);
  // 设置MD1(PD4)输出低电平
  HAL_GPIO_WritePin(GPIOD, MD1_Pin, GPIO_PIN_RESET);
  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
 * @brief 发送格式化消息，串口2，使用DMA非阻塞方式，双缓冲（旧版JSON格式）
 * @param format 消息格式
 * @param ...   可变参数
 */
void send_message(const char *format, ...)
{
    va_list args;
    int len;
    
    // 开始处理可变参数
    va_start(args, format);
    
    // 首次调用时创建互斥锁
    if (uart_tx_mutex == NULL) {
        osMutexDef(uart_tx_mutex);
        uart_tx_mutex = osMutexCreate(osMutex(uart_tx_mutex));
    }
    
    // 尝试获取互斥锁，超时为0立即返回，获取失败则丢弃本次发送
    if (osMutexWait(uart_tx_mutex, 0) != osOK) {
        va_end(args);
        return;
    }
    
    // 确定空闲缓冲区(与当前活动缓冲区相反)
    char *idle_buffer = (active_buf_index == 0) ? buffer_b : buffer_a;
    
    // 清除空闲缓冲区,防止数据残留
    memset(idle_buffer, 0, UART_TX_BUFFER_SIZE);
    
    // 将格式化字符串写入空闲缓冲区
    len = vsnprintf(idle_buffer, UART_TX_BUFFER_SIZE, format, args);
    va_end(args);
    
    // 验证长度有效性
    if (len <= 0 || len >= UART_TX_BUFFER_SIZE) {
        osMutexRelease(uart_tx_mutex);
        return;
    }
    
    // 保存空闲缓冲区的数据长度
    idle_buf_len = len;
    
    // 检查DMA是否空闲
    if (is_tx_busy == 0) {
        // DMA空闲，切换缓冲区（空闲区变为活动区）
        active_buf_index = (active_buf_index == 0) ? 1 : 0;
        char *active_buffer = (active_buf_index == 0) ? buffer_a : buffer_b;
        
        // 获取待发送长度并清零空闲区长度
        uint16_t send_len = idle_buf_len;
        idle_buf_len = 0;
        
        // 设置DMA忙碌标志
        is_tx_busy = 1;
        
        // 释放互斥锁
        osMutexRelease(uart_tx_mutex);
        
        // 启动DMA发送
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active_buffer, send_len);
    } else {
        // DMA忙碌，数据已写入空闲区，等待回调函数处理
        osMutexRelease(uart_tx_mutex);
    }
}

/**
 * @brief 发送格式化消息，串口2，使用DMA非阻塞方式，双缓冲，带命令ID前缀
 * @param cmd_id 命令ID，用于添加数据类型前缀
 * @param format 消息格式
 * @param ...   可变参数
 */
void send_ready(uint8_t cmd_id, const char *format, ...)
{
    va_list args;
    int len;
    int prefix_len;
    
    // 开始处理可变参数
    va_start(args, format);
    
    // 首次调用时创建互斥锁
    if (uart_tx_mutex == NULL) {
        osMutexDef(uart_tx_mutex);
        uart_tx_mutex = osMutexCreate(osMutex(uart_tx_mutex));
    }
    
    // 尝试获取互斥锁，超时为0立即返回，获取失败则丢弃本次发送
    if (osMutexWait(uart_tx_mutex, 0) != osOK) {
        va_end(args);
        return;
    }
    
    // 确定空闲缓冲区(与当前活动缓冲区相反)
    char *idle_buffer = (active_buf_index == 0) ? buffer_b : buffer_a;
    
    // 清除空闲缓冲区,防止数据残留
    memset(idle_buffer, 0, UART_TX_BUFFER_SIZE);
    
    // 添加前缀 [ID]
    prefix_len = snprintf(idle_buffer, UART_TX_BUFFER_SIZE, "[%02X]", cmd_id);
    if (prefix_len <= 0 || prefix_len >= UART_TX_BUFFER_SIZE) {
        va_end(args);
        osMutexRelease(uart_tx_mutex);
        return;
    }
    
    // 将格式化字符串写入空闲缓冲区（在前缀之后）
    len = vsnprintf(idle_buffer + prefix_len, UART_TX_BUFFER_SIZE - prefix_len, format, args);
    va_end(args);
    
    // 验证长度有效性
    if (len <= 0 || (prefix_len + len) >= UART_TX_BUFFER_SIZE) {
        osMutexRelease(uart_tx_mutex);
        return;
    }
    
    // 保存空闲缓冲区的数据长度（前缀 + 数据）
    idle_buf_len = prefix_len + len;
    
    // 检查DMA是否空闲
    if (is_tx_busy == 0) {
        // DMA空闲，切换缓冲区（空闲区变为活动区）
        active_buf_index = (active_buf_index == 0) ? 1 : 0;
        char *active_buffer = (active_buf_index == 0) ? buffer_a : buffer_b;
        
        // 获取待发送长度并清零空闲区长度
        uint16_t send_len = idle_buf_len;
        idle_buf_len = 0;
        
        // 设置DMA忙碌标志
        is_tx_busy = 1;
        
        // 释放互斥锁
        osMutexRelease(uart_tx_mutex);
        
        // 启动DMA发送
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active_buffer, send_len);
    } else {
        // DMA忙碌，数据已写入空闲区，等待回调函数处理
        osMutexRelease(uart_tx_mutex);
    }
}

/**
 * @brief 发送格式化消息，串口2，阻塞方式
 * @param format 消息格式
 * @param ...   可变参数
 */
void send_message_direct(const char *format, ...)
{
    char temp_buffer[UART_TX_BUFFER_SIZE];
    va_list args;
    int len;

    // 开始处理可变参数
    va_start(args, format);
    // 将格式化字符串写入临时缓冲区
    len = vsnprintf(temp_buffer, UART_TX_BUFFER_SIZE, format, args);
    va_end(args);

    // 验证长度有效性后使用阻塞方式发送
    if (len > 0 && len < UART_TX_BUFFER_SIZE) {
        HAL_UART_Transmit(&huart2, (uint8_t*)temp_buffer, len, 1000);
    }
}


/**
 * @brief UART DMA发送完成回调函数
 * @param huart UART句柄
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // 清除DMA忙碌标志
        is_tx_busy = 0;
        
        // 清除刚发送完的缓冲区,防止数据残留
        char *just_sent_buffer = (active_buf_index == 0) ? buffer_a : buffer_b;
        memset(just_sent_buffer, 0, UART_TX_BUFFER_SIZE);
        
        // 检查空闲缓冲区是否有待发送数据
        if (idle_buf_len > 0) {
            // 切换缓冲区（空闲区变为活动区）
            active_buf_index = (active_buf_index == 0) ? 1 : 0;
            char *active_buffer = (active_buf_index == 0) ? buffer_a : buffer_b;
            
            // 获取待发送长度并清零
            uint16_t send_len = idle_buf_len;
            idle_buf_len = 0;
            
            // 设置DMA忙碌标志
            is_tx_busy = 1;
            
            // 启动DMA发送
            HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active_buffer, send_len);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // 将接收到的字节放入队列
        // 注意：osMessagePut 可以在 ISR 中调用，但中断优先级必须正确配置
        // 队列满时会返回错误，不会阻塞
        osMessagePut(usart2_rx_queueHandle, (uint32_t)rx_byte, 0);
        
        // 重新启动接收
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

/* USER CODE END 1 */
