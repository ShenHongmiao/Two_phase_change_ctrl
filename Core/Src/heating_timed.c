/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    heating_timed.c
  * @brief   定时加热控制模块实现文件
  *          满功率定时加热控制
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
#include "heating_timed.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

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
void HeatingTimed_FullPower(uint32_t duration_ms)
{
    uint32_t tick_start;
    uint32_t tick_target;
    
    // 获取当前tick计数
    tick_start = osKernelGetTickCount();
    tick_target = tick_start + duration_ms;
    
    // 开启满功率加热
    Set_Heating_PWM(HEATING_PWM_FULL_POWER);
    
    // 精确延时到目标时间
    osDelayUntil(tick_target);
    
    // 关闭加热
    Set_Heating_PWM(HEATING_PWM_OFF);
}
