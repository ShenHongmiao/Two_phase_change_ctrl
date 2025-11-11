#ifndef __NTC_H__
#define __NTC_H__

#include "cmsis_os.h"
#include "adc.h"
#include "math.h"

// NTC 热敏电阻参数
#define NTC_BETA 3380.0f      // NTC 热敏电阻的 Beta 常数 (B值 25°C/50°C)
#define NTC_R0 10000.0f       // NTC 热敏电阻在 T0 温度下的阻值 (10k Ohm @ 25°C)
#define NTC_T0 298.15f        // 参考温度 T0 (25°C = 298.15K)
#define NTC_R_SERIES 10000.0f // 串联电阻的阻值 (10k Ohm)
#define ADC_MAX_VALUE 4095.0f // 12-bit ADC 最大值
#define V_REF 3.3f            // 参考电压 (3.3V)

// ADC 通道配置 - 使能宏定义控制是否计算该通道
#define NTC_CHANNEL0_ENABLE 1  // 1: 使能通道0 (PA0), 0: 禁用
#define NTC_CHANNEL1_ENABLE 0  // 1: 使能通道1 (PA1), 0: 禁用

// DMA 缓冲区大小（根据使能的通道数量调整）
#define ADC_DMA_BUFFER_SIZE 2  // ADC1配置了2个通道

// 硬件连接: ADC1_IN0 (PA0引脚)  ADC1_IN1 (PA1引脚)
// 电路: VCC(3.3V) -- R_SERIES(10k) -- PA0/1 -- NTC(10k@25°C) -- GND

// 温度数据结构
typedef struct {
    float temperature_ch0;  // 通道0温度
    float temperature_ch1;  // 通道1温度
    uint16_t adc_value_ch0; // 通道0 ADC原始值
    uint16_t adc_value_ch1; // 通道1 ADC原始值
} NTC_Data_t;

// 外部变量声明 - 可直接在其他文件中访问NTC数据缓冲区
extern NTC_Data_t NTC_DataBuffer;

// 函数声明
void NTC_Init(void);
void NTC_StartDMA(void);
void NTC_Calculate(NTC_Data_t *ntc_data);

#endif // __NTC_H__