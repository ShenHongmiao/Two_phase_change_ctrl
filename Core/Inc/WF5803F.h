#ifndef __WF5803F_H__
#define __WF5803F_H__

#include "i2c.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "math.h"
//需要对传感器寄存器0x30写入000b开始转换单次温度转换，001b开始单次气压转换
//通过读取0x02寄存器的bit0(DRDY)值来判断是否转换完成,1表示完成
//转换完成后读取温度和气压数据，原始温度数据Temp_out为16位，分为MSB和LSB。地址为0x09~0x0A
//原始气压数据Press_out为24位，分为MSB、CSB和LSB。地址为0x06~0x08

// DMA传输状态标志
typedef enum {
    I2C_DMA_IDLE = 0,       // 空闲状态
    I2C_DMA_BUSY,           // 传输中
    I2C_DMA_COMPLETE,       // 传输完成
    I2C_DMA_ERROR           // 传输错误
} I2C_DMA_Status_t;

// WF5803F 寄存器地址定义
#define WF5803F_ADDR          (0x6C<<1)   // I2C 设备地址 (7位地址左移)
#define WF5803F_REG_CTRL      0x30        // 控制寄存器
#define WF5803F_REG_STATUS    0x02        // 状态寄存器
#define WF5803F_REG_PRESS_MSB 0x06        // 气压数据 MSB
#define WF5803F_REG_TEMP_MSB  0x09        // 温度数据 MSB
#define WF5803F_Enable 1  // 1: 使能WF5803F传感器读取，0: 禁用

// WF5803F 数据结构
typedef struct {
    float temperature;      // 温度值 (℃)
    float pressure;         // 压力值 (kPa)
    int16_t raw_temp;       // 原始温度数据
    int32_t raw_pressure;   // 原始压力数据
} WF5803F_Data_t;

// 外部变量声明 - 可直接在其他文件中访问 WF5803F 数据缓冲区
extern WF5803F_Data_t WF5803F_DataBuffer;

// 全局DMA状态标志
extern volatile I2C_DMA_Status_t g_i2c_tx_status;
extern volatile I2C_DMA_Status_t g_i2c_rx_status;

// 函数声明
void WF5803F_Init(void);
void WF5803F_ReadData(WF5803F_Data_t *wf5803f_data);
void WF5803F_Calculate(WF5803F_Data_t *wf5803f_data);
float compute_pressure_WF5803F_2BAR_fromInt(int32_t rawData);
float compute_temperature_WF5803F_fromInt(int16_t rawData);

#endif // __WF5803F_H__