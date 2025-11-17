/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Command_Action.h
  * @brief   上位机命令处理模块头文件
  *          负责接收、解析和执行来自上位机的控制命令
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __COMMAND_ACTION_H__
#define __COMMAND_ACTION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "serial_to_pc.h"
#include "temp_pid_ctrl.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 命令帧结构体
 */
typedef struct {
    uint8_t frame_head;      // 帧头 0xDE
    uint8_t cmd_id;          // 命令ID
    uint8_t data_length;     // 数据长度
    uint8_t data[256];       // 数据体
    uint8_t crc;             // CRC8校验
    uint8_t frame_tail;      // 帧尾 0xED
} CommandFrame_t;

/**
 * @brief 命令处理结果枚举
 */
typedef enum {
    CMD_RESULT_OK = 0,           // 处理成功
    CMD_RESULT_INVALID_FRAME,    // 无效帧格式
    CMD_RESULT_CRC_ERROR,        // CRC校验失败
    CMD_RESULT_UNKNOWN_CMD,      // 未知命令
    CMD_RESULT_INVALID_DATA,     // 无效数据
    CMD_RESULT_EXECUTE_FAILED    // 执行失败
} CommandResult_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 解析并校验接收到的命令帧
 * @param raw_data 原始数据缓冲区
 * @param length   数据长度
 * @param frame    输出的命令帧结构
 * @return CommandResult_t 解析结果
 */
CommandResult_t Command_ParseFrame(const uint8_t *raw_data, uint16_t length, CommandFrame_t *frame);

/**
 * @brief 执行命令动作
 * @param frame 已解析的命令帧
 * @return CommandResult_t 执行结果
 */
CommandResult_t Command_Execute(const CommandFrame_t *frame);

/**
 * @brief 处理接收到的完整数据包（解析+执行）
 * @param raw_data 原始数据缓冲区
 * @param length   数据长度
 * @return CommandResult_t 处理结果
 */
CommandResult_t Command_Process(const uint8_t *raw_data, uint16_t length);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 设置PID Kp参数
 * @param data 数据指针（4字节float）
 * @return bool 设置是否成功
 */
bool Command_SetPID_Kp(const uint8_t *data);

/**
 * @brief 设置PID Ki参数
 * @param data 数据指针（4字节float）
 * @return bool 设置是否成功
 */
bool Command_SetPID_Ki(const uint8_t *data);

/**
 * @brief 设置PID Kd参数
 * @param data 数据指针（4字节float）
 * @return bool 设置是否成功
 */
bool Command_SetPID_Kd(const uint8_t *data);

/**
 * @brief 设置目标温度
 * @param data 数据指针（4字节float）
 * @return bool 设置是否成功
 */
bool Command_SetTargetTemp(const uint8_t *data);

/**
 * @brief 切换工作模式
 * @param data 数据指针（1字节mode）
 * @return bool 设置是否成功
 */
bool Command_ChangeMode(const uint8_t *data);

/**
 * @brief 设置加热时间
 * @param data 数据指针（4字节uint32_t，单位ms）
 * @return bool 设置是否成功
 */
bool Command_ChangeHeatingTime(const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_ACTION_H__ */
