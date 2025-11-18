/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Command_Action.c
  * @brief   上位机命令处理模块实现文件
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

/* Includes ------------------------------------------------------------------*/
#include "Command_Action.h"
#include "temp_pid_ctrl.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 解析并校验接收到的命令帧
 * @param raw_data 原始数据缓冲区
 * @param length   数据长度
 * @param frame    输出的命令帧结构
 * @return CommandResult_t 解析结果
 */
CommandResult_t Command_ParseFrame(const uint8_t *raw_data, uint16_t length, CommandFrame_t *frame)
{
    // 最小帧长度检查：帧头(1) + 命令ID(1) + 数据长度(1) + CRC(1) + 帧尾(1) = 5字节
    if (raw_data == NULL || frame == NULL || length < 5) {
        return CMD_RESULT_INVALID_FRAME;
    }

    // 检查帧头
    if (raw_data[0] != FRAME_HEAD) {
        return CMD_RESULT_INVALID_FRAME;
    }

    // 提取基本字段
    frame->frame_head = raw_data[0];
    frame->cmd_id = raw_data[1];
    frame->data_length = raw_data[2];

    // 检查帧长度是否匹配
    // 帧头(1) + 命令ID(1) + 数据长度(1) + 数据体(data_length) + CRC(1) + 帧尾(1)
    uint16_t expected_length = 5 + frame->data_length;
    if (length < expected_length) {
        return CMD_RESULT_INVALID_FRAME;
    }

    // 提取数据体
    if (frame->data_length > 0) {
        memcpy(frame->data, &raw_data[3], frame->data_length);
    }

    // 提取CRC和帧尾
    frame->crc = raw_data[3 + frame->data_length];
    frame->frame_tail = raw_data[4 + frame->data_length];

    // 检查帧尾
    if (frame->frame_tail != FRAME_TAIL) {
        return CMD_RESULT_INVALID_FRAME;
    }

    // CRC校验（从帧头到数据体结束）
    uint16_t crc_length = 3 + frame->data_length; // 帧头 + 命令ID + 数据长度 + 数据体
    uint8_t calculated_crc = crc8_calculate(raw_data, crc_length);
    
    if (calculated_crc != frame->crc) {
        return CMD_RESULT_CRC_ERROR;
    }

    return CMD_RESULT_OK;
}

/**
 * @brief 执行命令动作
 * @param frame 已解析的命令帧
 * @return CommandResult_t 执行结果
 */
CommandResult_t Command_Execute(const CommandFrame_t *frame)
{
    if (frame == NULL) {
        return CMD_RESULT_INVALID_DATA;
    }

    bool result = false;
    float float_value;
    uint32_t uint32_value;

    switch (frame->cmd_id) {
        case CMD_SET_PID_KP:
            // 期望数据长度：4字节（float）
            if (frame->data_length != 4) {
                return CMD_RESULT_INVALID_DATA;
            }
            memcpy(&float_value, frame->data, sizeof(float));
            result = Command_SetPID_Kp(float_value);
            break;

        case CMD_SET_PID_KI:
            // 期望数据长度：4字节（float）
            if (frame->data_length != 4) {
                return CMD_RESULT_INVALID_DATA;
            }
            memcpy(&float_value, frame->data, sizeof(float));
            result = Command_SetPID_Ki(float_value);
            break;

        case CMD_SET_PID_KD:
            // 期望数据长度：4字节（float）
            if (frame->data_length != 4) {
                return CMD_RESULT_INVALID_DATA;
            }
            memcpy(&float_value, frame->data, sizeof(float));
            result = Command_SetPID_Kd(float_value);
            break;

        case CMD_SET_TARGET_TEMP:
            // 期望数据长度：4字节（float）
            if (frame->data_length != 4) {
                return CMD_RESULT_INVALID_DATA;
            }
            memcpy(&float_value, frame->data, sizeof(float));
            result = Command_SetTargetTemp(float_value);
            break;

        case CMD_CHANGE_HEATING_TIME:
            // 期望数据长度：4字节（uint32_t）
            if (frame->data_length != 4) {
                return CMD_RESULT_INVALID_DATA;
            }
            memcpy(&uint32_value, frame->data, sizeof(uint32_t));
            result = Command_ChangeHeatingTime(uint32_value);
            break;

        default:
            return CMD_RESULT_UNKNOWN_CMD;
    }

    // 根据执行结果返回
    if (result) {
        send_response(); // 发送应答
        return CMD_RESULT_OK;
    } else {
        return CMD_RESULT_EXECUTE_FAILED;
    }
}

/**
 * @brief 处理接收到的完整数据包（解析+执行）
 * @param raw_data 原始数据缓冲区
 * @param length   数据长度
 * @return CommandResult_t 处理结果
 */
CommandResult_t Command_Process(const uint8_t *raw_data, uint16_t length)
{
    CommandFrame_t frame;
    CommandResult_t result;

    // 解析帧
    result = Command_ParseFrame(raw_data, length, &frame);
    if (result != CMD_RESULT_OK) {
        return result;
    }

    // 执行命令
    result = Command_Execute(&frame);
    return result;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 设置PID Kp参数
 * @param kp_value PID Kp值
 * @return bool 设置是否成功
 */
bool Command_SetPID_Kp(float kp_value)
{
#if PID_CONTROL_ENABLE
    // 参数合法性检查（根据实际需求调整范围）
    if (kp_value < 0.0f || kp_value > 1000.0f) {
        return false;
    }

    // 设置PID Kp参数
    PID_SetKp(&Temp_PID_Controller_CH0, kp_value);
    
    // 可选：发送调试信息
    send2pc(CMD_TEXT_INFO, NULL, "PID Kp set to: %.2f", kp_value);
    
    return true;
#else
    return false;
#endif
}

/**
 * @brief 设置PID Ki参数
 * @param ki_value PID Ki值
 * @return bool 设置是否成功
 */
bool Command_SetPID_Ki(float ki_value)
{
#if PID_CONTROL_ENABLE
    // 参数合法性检查（根据实际需求调整范围）
    if (ki_value < 0.0f || ki_value > 1000.0f) {
        return false;
    }

    // 设置PID Ki参数
    PID_SetKi(&Temp_PID_Controller_CH0, ki_value);
    
    // 可选：发送调试信息
    send2pc(CMD_TEXT_INFO, NULL, "PID Ki set to: %.2f", ki_value);
    
    return true;
#else
    return false;
#endif
}

/**
 * @brief 设置PID Kd参数
 * @param kd_value PID Kd值
 * @return bool 设置是否成功
 */
bool Command_SetPID_Kd(float kd_value)
{
#if PID_CONTROL_ENABLE
    // 参数合法性检查（根据实际需求调整范围）
    if (kd_value < 0.0f || kd_value > 1000.0f) {
        return false;
    }

    // 设置PID Kd参数
    PID_SetKd(&Temp_PID_Controller_CH0, kd_value);
    
    // 可选：发送调试信息
    send2pc(CMD_TEXT_INFO, NULL, "PID Kd set to: %.2f", kd_value);
    
    return true;
#else
    return false;
#endif
}

/**
 * @brief 设置目标温度
 * @param target_temp 目标温度值（℃）
 * @return bool 设置是否成功
 */
bool Command_SetTargetTemp(float target_temp)
{
#if PID_CONTROL_ENABLE
    // 温度合法性检查（根据实际需求调整范围，例如0-200℃）
    if (target_temp < 0.0f || target_temp > 200.0f) {
        return false;
    }

    // 设置目标温度
    PID_SetSetpoint(&Temp_PID_Controller_CH0, target_temp);
    
    // 可选：发送调试信息
    send2pc(CMD_TEXT_INFO, NULL, "Target temp set to: %.2f C", target_temp);
    
    return true;
#else
    return false;
#endif
}

/**
 * @brief 设置加热时间
 * @param heating_time_ms 加热时间（单位ms）
 * @return bool 设置是否成功
 */
bool Command_ChangeHeatingTime(uint32_t heating_time_ms)
{
    // 时间合法性检查（根据实际需求调整）
    if (heating_time_ms > 5000) { // 5s
        return false;
    }
    
    // 设置加热时间
    HeatingTimed_FullPower(heating_time_ms);
    
    // 可选：发送调试信息
    send2pc(CMD_TEXT_INFO, NULL, "Heating time set to: %lu ms", heating_time_ms);
    
    return true;
}
