#ifndef __V_DETECT_H__
#define __V_DETECT_H__

#include "adc.h"
#include "usart.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>

/**
*电路说明:
*VCC(24V) -- R1(51kΩ) -- PC4(ADC) -- R2(5.1kΩ) -- GND
*通道：ADC2_CHANNEL_14 (PC4引脚)
*分压公式:
* VADC = VCC × (R2 / (R1 + R2)) = VCC × 0.0909
* 反推: VCC = VADC / 0.0909
*/

// 电压检测参数
#define VOLTAGE_R1           51000.0f    // R1 = 51kΩ
#define VOLTAGE_R2           5100.0f     // R2 = 5.1kΩ
#define VOLTAGE_RATIO        (VOLTAGE_R2 / (VOLTAGE_R1 + VOLTAGE_R2))  // 0.0909
#define VOLTAGE_NORMAL       24.0f       // 标准电压 24V
#define VOLTAGE_THRESHOLD    (VOLTAGE_NORMAL * 0.7f)  // 低压阈值 16.8V (70%)
#define VOLTAGE_ADC_MAX      4095.0f     // 12-bit ADC
#define VOLTAGE_ADC_VREF     3.3f        // ADC 参考电压

// 电压数据结构
typedef struct {
    float voltage;        // 电源电压 (V)
    uint16_t adc_value;   // ADC原始值
    uint8_t is_normal;    // 电压状态: 1=正常, 0=过低
} Voltage_Data_t;

// 外部变量声明
extern Voltage_Data_t Voltage_DataBuffer;

// 函数声明
void Voltage_Init(void);
void Voltage_StartDMA(void);
void Voltage_Calculate(Voltage_Data_t *voltage_data);
void Voltage_info_send(Voltage_Data_t *voltage_data);
uint16_t* Voltage_GetDMABuffer(void);  // 供回调函数使用

#endif // __V_DETECT_H__

