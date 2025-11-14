/**
  ******************************************************************************
  * @file           : temp_pid_ctrl.c
  * @brief          : Temperature PID Control Implementation
  *                   温度PID控制器实现
  ******************************************************************************
  * @attention
  *
  * 此文件实现了基于PID算法的温度控制功能
  * 
  * 硬件配置:
  * - PC6: TIM3_CH1 (加热控制1) - 硬件PWM输出
  * - PC7: TIM3_CH2 (加热控制2) - 硬件PWM输出
  * 
  * 控制策略:
  * - 使用TIM3硬件PWM控制NMOS占空比 (周期1000ms)
  * - PID算法计算占空比 (0-1000ms)
  * - 温度过高时紧急关断加热
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "temp_pid_ctrl.h"



/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static float Clamp(float value, float min, float max);

/* Function implementations --------------------------------------------------*/

/**
 * @brief  设置加热MOS管硬件PWM占空比（0-10000）
 * @param  duty_ms: 0-1000ms（实际PWM周期为1000ms）
 * @note   使用 CubeMX 生成的 TIM3 PWM 控制
 */
void Set_Heating_PWM(uint16_t duty_ms)
{
    // 限幅：确保占空比在有效范围内
    if (duty_ms > 1000) duty_ms = 1000;
    // 将毫秒转换为 PWM 计数值 (假设 ARR=9999, 1000ms对应10000计数)
    uint32_t pulse = duty_ms * 10; // 1000ms对应10000计数

    // 设置 TIM3 通道1 PWM 占空比 (PC6)
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
    
    // 如果需要同时控制通道2 (PC7)，取消注释下面这行
    // __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
}

/**
 * @brief  初始化PID控制器
 * @param  pid: PID控制器结构体指针
 * @retval None
 */
void PID_Init(PID_Controller_t *pid)
{
    if (pid == NULL) return;
    
    // 设置PID参数
    pid->Kp = PID_KP;
    pid->Ki = PID_KI;
    pid->Kd = PID_KD;
    
    // 设置目标温度
    pid->setpoint = TARGET_TEMP_1;
    
    // 初始化内部状态
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
    
    // 设置输出限幅
    pid->output_limit_max = PID_OUTPUT_MAX;
    pid->output_limit_min = PID_OUTPUT_MIN;
    
    // 设置积分限幅
    pid->integral_limit_max = PID_INTEGRAL_MAX;
    pid->integral_limit_min = PID_INTEGRAL_MIN;
    
    // 设置采样时间
    pid->sample_time_ms = PID_SAMPLE_TIME_MS;
}

/**
 * @brief  PID控制器计算（可选积分分离）
 * @param  pid: PID控制器结构体指针
 * @param  measured_value: 当前测量的温度值
 * @retval PID输出值 (0-1000ms)
 * @note   积分分离：当误差 > 阈值时，停止积分作用，避免积分饱和
 *         通过 ENABLE_INTEGRAL_SEPARATION 宏控制是否启用
 */
float PID_Compute(PID_Controller_t *pid, float measured_value)
{
    if (pid == NULL) return 0.0f;
    
    // 计算误差
    float error = pid->setpoint - measured_value;
    
    // 死区控制 - 在目标温度附近小幅波动时不调整
    if (fabsf(error) < TEMP_DEADBAND) {
        // 保持当前输出，不累积积分
        return pid->output;
    }
    
    // 计算积分项
    float dt = pid->sample_time_ms / 1000.0f;
    
#if ENABLE_INTEGRAL_SEPARATION
    // 积分分离启用：只有误差绝对值 <= 阈值时才累积积分
    if (fabsf(error) <= INTEGRAL_SEPARATION_THRESHOLD) {
        // 误差较小，允许积分累积
        pid->integral += error * dt;
        // 积分限幅，防止积分饱和
        pid->integral = Clamp(pid->integral, pid->integral_limit_min, pid->integral_limit_max);
    }
    // 误差过大时，保持积分值不变（不累积也不清零）
#else
    // 积分分离禁用：传统PID，始终累积积分
    pid->integral += error * dt;
    // 积分限幅，防止积分饱和
    pid->integral = Clamp(pid->integral, pid->integral_limit_min, pid->integral_limit_max);
#endif
    
    // 计算微分项
    float derivative = (error - pid->prev_error) / dt;
    
    // PID输出计算
    pid->output = pid->Kp * error + 
                  pid->Ki * pid->integral + 
                  pid->Kd * derivative;
    
    // 输出限幅 (0-1000ms)
    pid->output = Clamp(pid->output, pid->output_limit_min, pid->output_limit_max);
    
    // 保存当前误差供下次使用
    pid->prev_error = error;
    
    return pid->output;
}

/**
 * @brief  设置PID目标温度
 * @param  pid: PID控制器结构体指针
 * @param  setpoint: 目标温度
 * @retval None
 */
void PID_SetSetpoint(PID_Controller_t *pid, float setpoint)
{
    if (pid == NULL) return;
    pid->setpoint = setpoint;
}

/**
 * @brief  设置PID比例增益 Kp
 * @param  pid: PID控制器结构体指针
 * @param  kp: 比例增益
 * @retval None
 */
void PID_SetKp(PID_Controller_t *pid, float kp)
{
    if (pid == NULL) return;
    pid->Kp = kp;
}

/**
 * @brief  设置PID积分增益 Ki
 * @param  pid: PID控制器结构体指针
 * @param  ki: 积分增益
 * @retval None
 */
void PID_SetKi(PID_Controller_t *pid, float ki)
{
    if (pid == NULL) return;
    pid->Ki = ki;
    // 积分参数改变时，重置积分项
    pid->integral = 0.0f;
}

/**
 * @brief  设置PID微分增益 Kd
 * @param  pid: PID控制器结构体指针
 * @param  kd: 微分增益
 * @retval None
 */
void PID_SetKd(PID_Controller_t *pid, float kd)
{
    if (pid == NULL) return;
    pid->Kd = kd;
}

/**
 * @brief  设置PID参数（封装函数）
 * @param  pid: PID控制器结构体指针
 * @param  kp: 比例增益（-1表示不修改）
 * @param  ki: 积分增益（-1表示不修改）
 * @param  kd: 微分增益（-1表示不修改）
 * @retval None
 */
void PID_SetParameters(PID_Controller_t *pid, float kp, float ki, float kd)
{
    if (pid == NULL) return;
    
    // 只修改传入值不为-1的参数
    if (kp >= 0.0f) {
        PID_SetKp(pid, kp);
    }
    
    if (ki >= 0.0f) {
        PID_SetKi(pid, ki);
    }
    
    if (kd >= 0.0f) {
        PID_SetKd(pid, kd);
    }
}

/**
 * @brief  重置PID控制器
 * @param  pid: PID控制器结构体指针
 * @retval None
 */
void PID_Reset(PID_Controller_t *pid)
{
    if (pid == NULL) return;
    
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

/**
 * @brief  限幅函数
 * @param  value: 输入值
 * @param  min: 最小值
 * @param  max: 最大值
 * @retval 限幅后的值
 */
static float Clamp(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * @brief  紧急关闭加热
 * @retval None
 */
void TempCtrl_EmergencyStop(PID_Controller_t *pid)
{
    // 关闭硬件PWM输出
    Set_Heating_PWM(0);
    
    // 重置PID控制器
    PID_Reset(pid);
    
    send_message("[TEMP_CTRL] Emergency stop activated!\n");
}

/**
 * @brief  初始化温度控制系统
 * @retval None
 */
void TempCtrl_Init(PID_Controller_t *pid)
{
    // 初始化PID控制器
    PID_Init(pid);
    
    // 初始化硬件PWM为关断状态
    Set_Heating_PWM(0);
    
    send_message("Temperature Control Initialized\n");
    send_message("Target Temperature: %.2f°C\n", pid->setpoint);
    send_message("PID Parameters: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", 
           pid->Kp, pid->Ki, pid->Kd);
    send_message("Hardware PWM Mode (TIM3), Period: %dms\n", PWM_PERIOD_MS);

    // 打印温度区间信息
    // #if (TARGET_TEMP_INT < 50)
    // send_message("[TEMP_CTRL] Temperature Range: LOW (30-50°C)\n");
    // #elif (TARGET_TEMP_INT < 70)
    // send_message("[TEMP_CTRL] Temperature Range: MID (50-70°C)\n");
    // #else
    // send_message("[TEMP_CTRL] Temperature Range: HIGH (70-100°C)\n");
    // #endif
}


