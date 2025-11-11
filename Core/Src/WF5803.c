#include "WF5803F.h"

// DMA状态标志定义
volatile I2C_DMA_Status_t g_i2c_tx_status = I2C_DMA_IDLE;
volatile I2C_DMA_Status_t g_i2c_rx_status = I2C_DMA_IDLE;

// WF5803F 数据缓冲区
WF5803F_Data_t WF5803F_DataBuffer = {0};

/**
 * @brief  WF5803F 传感器初始化
 */
void WF5803F_Init(void)
{
    // 清零数据缓冲区
    WF5803F_DataBuffer.temperature = 0.0f;
    WF5803F_DataBuffer.pressure = 0.0f;
    WF5803F_DataBuffer.raw_temp = 0;
    WF5803F_DataBuffer.raw_pressure = 0;

}

/**
 * @brief  读取 WF5803F 原始数据
 * @param  wf5803f_data  指向 WF5803F_Data_t 结构体的指针
 * @note   读取温度和气压的原始值，存储到结构体的 raw_temp 和 raw_pressure 字段
 */
void WF5803F_ReadData(WF5803F_Data_t *wf5803f_data)
{
    if (wf5803f_data == NULL) {
        return;
    }
    
    static uint8_t cmd = 0x0A;                    // 单次气压+温度转换
    static uint8_t status;
    static uint8_t buf[5];
    HAL_StatusTypeDef ret;

    // 启动一次转换 - 使用DMA方式
    g_i2c_tx_status = I2C_DMA_BUSY;
    ret = HAL_I2C_Mem_Write_DMA(&hi2c1, WF5803F_ADDR, WF5803F_REG_CTRL,
                                I2C_MEMADD_SIZE_8BIT, &cmd, 1);
    
    if (ret != HAL_OK) {
        g_i2c_tx_status = I2C_DMA_ERROR;
        return;
    }
    
    // 等待写入完成
    while (g_i2c_tx_status == I2C_DMA_BUSY) {
        osDelay(1);
    }
    
    if (g_i2c_tx_status == I2C_DMA_ERROR) {
        return;
    }

    // 等待传感器转换完成
    osDelay(5);
    
    // 轮询读取状态寄存器 - 使用DMA方式
    do {
        g_i2c_rx_status = I2C_DMA_BUSY;
        ret = HAL_I2C_Mem_Read_DMA(&hi2c1, WF5803F_ADDR, WF5803F_REG_STATUS,
                                   I2C_MEMADD_SIZE_8BIT, &status, 1);
        
        if (ret != HAL_OK) {
            g_i2c_rx_status = I2C_DMA_ERROR;
            return;
        }
        
        // 等待读取完成
        while (g_i2c_rx_status == I2C_DMA_BUSY) {
            osDelay(1);
        }
        
        if (g_i2c_rx_status == I2C_DMA_ERROR) {
            return;
        }
        
        if ((status & 0x01) == 0) {
            osDelay(5);
        }
    } while ((status & 0x01) == 0);

    // 一次性读出 5 字节（3B 压力 + 2B 温度）- 使用DMA方式
    g_i2c_rx_status = I2C_DMA_BUSY;
    ret = HAL_I2C_Mem_Read_DMA(&hi2c1, WF5803F_ADDR, WF5803F_REG_PRESS_MSB,
                               I2C_MEMADD_SIZE_8BIT, buf, 5);
    
    if (ret != HAL_OK) {
        g_i2c_rx_status = I2C_DMA_ERROR;
        return;
    }
    
    // 等待读取完成
    while (g_i2c_rx_status == I2C_DMA_BUSY) {
        osDelay(1);
    }
    
    if (g_i2c_rx_status == I2C_DMA_ERROR) {
        return;
    }

    // 存储原始数据到结构体
    wf5803f_data->raw_pressure = (int32_t)((uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | buf[2]); // 24bit
    wf5803f_data->raw_temp = (int16_t)((uint16_t)buf[3] << 8 | buf[4]);                                // 16bit
}

/**
 * @brief  将 32位有符号整数(只用低24位) 转换为压力
 * @param  rawData  32位整型数，存放传感器的24位原始数据
 * @return 压力值，单位 kPa
 */
float compute_pressure_WF5803F_2BAR_fromInt(int32_t rawData) {
    // 保留 24位数据
    rawData &= 0x00FFFFFF;

    // 如果最高位 (bit23) 为1，则需要做符号扩展
    if (rawData & 0x00800000) {
        rawData |= 0xFF000000; // 扩展成32位有符号数
    }

    // 归一化到 [-1, +1)
    float factor = (float)rawData / 8388608.0f;  // 除以 2^23

    // 固定公式（2bar型号，输出 kPa）
    return 180.0f/0.81f*(factor-0.1f)+30.0f;
}

/**
 * @brief  将 16位有符号整数转换为温度
 * @param  rawData  16位整型数，存放传感器的温度原始数据
 * @return 温度值，单位 ℃
 */
float compute_temperature_WF5803F_fromInt(int16_t rawData) {
    // 如果最高位 (bit15) 为1，则需要做符号扩展
    if (rawData & 0x00008000) {   // 如果bit15是1，表示负数
        rawData |= 0xFFFF0000;    // 高16位填充1
    }

    return (float)rawData / 256.0f; // 每个单位 = 1/256 ℃
}

/**
 * @brief  计算 WF5803F 的温度和压力值
 * @param  wf5803f_data  指向 WF5803F_Data_t 结构体的指针
 * @note   从结构体中读取原始数据，计算后更新温度和压力字段
 */
void WF5803F_Calculate(WF5803F_Data_t *wf5803f_data)
{
    if (wf5803f_data == NULL) {
        return;
    }
    
    // 转换为实际物理值
    wf5803f_data->temperature = compute_temperature_WF5803F_fromInt(wf5803f_data->raw_temp);
    wf5803f_data->pressure = compute_pressure_WF5803F_2BAR_fromInt(wf5803f_data->raw_pressure);
}


