#include "V_detect.h"

// DMA缓冲区存储ADC转换结果
static uint16_t ADC_DMA_Buffer = 0;

// 电压数据缓冲区（全局变量，可在其他文件中通过extern访问）
Voltage_Data_t Voltage_DataBuffer = {0};

// 外部DMA句柄声明
extern DMA_HandleTypeDef hdma_adc2;


/**
 * @brief  获取电压检测DMA缓冲区指针
 * @return DMA缓冲区指针（供回调函数使用）
 */
uint16_t* Voltage_GetDMABuffer(void)
{
    return &ADC_DMA_Buffer;
}


/**
 * @brief  电压检测模块初始化
 * @note   初始化DMA缓冲区，关闭半满中断
 */
void Voltage_Init(void)
{
    ADC_DMA_Buffer = 0;
    
    Voltage_DataBuffer.voltage = 0.0f;
    Voltage_DataBuffer.adc_value = 0;
    Voltage_DataBuffer.is_normal = 1;  // 默认正常
    
    // 关闭DMA半满中断（只使用转换完成中断）
    __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_HT);
}


/**
 * @brief  启动ADC2 DMA转换
 * @note   启动ADC2的DMA转换，将结果存储到ADC_DMA_Buffer
 */
void Voltage_StartDMA(void)
{
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&ADC_DMA_Buffer, 1);
}


/**
 * @brief  计算电源电压
 * @param  voltage_data  电压数据结构指针，用于存储计算结果
 * @note   根据ADC值计算电压并判断是否正常
 */
void Voltage_Calculate(Voltage_Data_t *voltage_data)
{
    if (voltage_data == NULL)
    {
        return;
    }
    
    // 从缓冲区读取ADC值
    voltage_data->adc_value = Voltage_DataBuffer.adc_value;
    
    // 计算电压
    if (voltage_data->adc_value > 0 && voltage_data->adc_value <= VOLTAGE_ADC_MAX)
    {
        // ADC值转换为电压
        float vAdc = ((float)voltage_data->adc_value / VOLTAGE_ADC_MAX) * VOLTAGE_ADC_VREF;
        
        // 根据分压公式反推电源电压: VCC = VADC / VOLTAGE_RATIO
        voltage_data->voltage = vAdc / VOLTAGE_RATIO;
        
        // 判断电压是否正常
        voltage_data->is_normal = (voltage_data->voltage >= VOLTAGE_THRESHOLD) ? 1 : 0;
    }
    else
    {
        voltage_data->voltage = 0.0f;
        voltage_data->is_normal = 0;  // 异常
    }
}


void Voltage_info_send(Voltage_Data_t *voltage_data)
{
    if (voltage_data == NULL)
    {
        return;
    }
    
    send_message("{\"type\":\"data\",\"sensor\":\"Voltage_Detect\",\"voltage\":%.2f,\"status\":%s}\n",
                 voltage_data->voltage,
                 voltage_data->is_normal ? "normal" : "low");
}

// 注意：HAL_ADC_ConvCpltCallback 回调函数已在 NTC.c 中统一定义，处理 ADC1 和 ADC2
