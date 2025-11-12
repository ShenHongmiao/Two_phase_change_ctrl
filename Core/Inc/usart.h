/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdarg.h>
#include <stdio.h>
#include "cmsis_os.h"
#include <string.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
//数据帧格式定义
#define FRAME_HEAD 0xDE
#define FRAME_TAIL 0xED
//0x0...为向上位机发送数据的命令ID
#define CMD_NTC            0x01
#define CMD_WF5803F        0x02
#define CMD_VOLTAGE        0x03
#define CMD_PID_VALUE      0x04
#define CMD_TEXT_INFO      0x0F  // 通用信息
//0xA...从上位机接收任务指令的命令ID
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
extern osMessageQId usart2_rx_queueHandle;
extern osMessageQId usart1_rx_queueHandle;
extern uint8_t rx_byte;

void send_message(const char *format, ...);
void send_ready(uint8_t cmd_id, const char *format, ...);
void send_message_direct(const char *format, ...);
void send_binary_data(const void *data, size_t len);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

