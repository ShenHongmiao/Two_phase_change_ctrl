/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    heating_timed.h
  * @brief   定时加热控制模块头文件
  *          支持通道PWM满功率定时加热控制
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * 功能说明:
  * - 满功率加热指定时间后自动关闭
  * - 使用osDelayUntil精确计时
  * - 调用Set_Heating_PWM控制PWM输出
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __HEATING_TIMED_H__
#define __HEATING_TIMED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "temp_pid_ctrl.h"
#include "cmsis_os.h"
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

/* 恒定开度加热配置 */
#define HEATING_TIMED_ENABLE        1       // 启用定时加热功能
#define HEATING_PWM_FULL_POWER      1000    // 满功率占空比 (1000ms = 100%)
#define HEATING_PWM_OFF             0       // 关闭加热 (0ms = 0%)

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 定时加热 - 满功率加热指定时间后自动关闭
 * @param duration_ms 加热持续时间(ms)
 * @retval None
 * @note 
 *   - 此函数会阻塞执行duration_ms时间
 *   - 加热期间PWM输出100%占空比
 *   - 时间到达后自动关闭加热
 *   - 使用osDelayUntil精确计时
 */
void HeatingTimed_FullPower(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* __HEATING_TIMED_H__ */
