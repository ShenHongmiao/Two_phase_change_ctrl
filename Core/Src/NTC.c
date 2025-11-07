#include "NTC.h"
#include "dma.h"

// DMA 缓冲区存储ADC转换结果
static uint16_t ADC_DMA_Buffer[ADC_DMA_BUFFER_SIZE] = {0};

// NTC数据缓冲区（全局变量，可在其他文件中通过extern访问）
NTC_Data_t NTC_DataBuffer = {0};

// 外部DMA句柄声明
extern DMA_HandleTypeDef hdma_adc1;


/**
 * @brief  NTC 模块初始化
 * @note   初始化 DMA 缓冲区，关闭半满中断
 */
void NTC_Init(void)
{
    // 清空 DMA 缓冲区
    for (uint8_t i = 0; i < ADC_DMA_BUFFER_SIZE; i++)
    {
        ADC_DMA_Buffer[i] = 0;
    }
    
    // 清空数据缓冲区
    NTC_DataBuffer.temperature_ch0 = 0.0f;
    NTC_DataBuffer.temperature_ch1 = 0.0f;
    NTC_DataBuffer.adc_value_ch0 = 0;
    NTC_DataBuffer.adc_value_ch1 = 0;
    
    // 关闭DMA半满中断（只使用转换完成中断）
    __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT);
}


/**
 * @brief  启动 ADC DMA 转换
 * @note   启动 ADC1 的 DMA 转换，将结果存储到 ADC_DMA_Buffer
 */
void NTC_StartDMA(void)
{
    // 启动 ADC DMA 转换
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_DMA_Buffer, ADC_DMA_BUFFER_SIZE);
}


/**
 * @brief  计算NTC温度
 * @param  ntc_data  NTC数据结构指针，用于存储计算结果
 * @note   根据宏定义配置选择计算对应通道
 */
void NTC_Calculate(NTC_Data_t *ntc_data)
{
    if (ntc_data == NULL)
    {
        return;
    }
    
    // 从缓冲区读取ADC值
    ntc_data->adc_value_ch0 = NTC_DataBuffer.adc_value_ch0;
    ntc_data->adc_value_ch1 = NTC_DataBuffer.adc_value_ch1;
    
#if NTC_CHANNEL0_ENABLE
    // 计算通道0温度
    uint32_t adc0 = ntc_data->adc_value_ch0;
    
    if (adc0 > 0 && adc0 < ADC_MAX_VALUE)
    {
        // ADC 转电压
        float vout0 = (adc0 / ADC_MAX_VALUE) * V_REF;
        // 由分压公式求 NTC 阻值
        float r_ntc0 = (NTC_R_SERIES * vout0) / (V_REF - vout0);
        // Beta 公式换算温度 (K)
        float tempK0 = 1.0f / ((1.0f/NTC_T0) + (1.0f/NTC_BETA) * logf(r_ntc0 / NTC_R0));
        // 转换为摄氏度
        ntc_data->temperature_ch0 = tempK0 - 273.15f;
    }
    else
    {
        ntc_data->temperature_ch0 = -273.15f; // 异常值
    }
#else
    ntc_data->temperature_ch0 = -273.15f; // 通道未使能
#endif

#if NTC_CHANNEL1_ENABLE
    // 计算通道1温度
    uint32_t adc1 = ntc_data->adc_value_ch1;
    
    if (adc1 > 0 && adc1 < ADC_MAX_VALUE)
    {
        // ADC 转电压
        float vout1 = (adc1 / ADC_MAX_VALUE) * V_REF;
        // 由分压公式求 NTC 阻值
        float r_ntc1 = (NTC_R_SERIES * vout1) / (V_REF - vout1);
        // Beta 公式换算温度 (K)
        float tempK1 = 1.0f / ((1.0f/NTC_T0) + (1.0f/NTC_BETA) * logf(r_ntc1 / NTC_R0));
        // 转换为摄氏度
        ntc_data->temperature_ch1 = tempK1 - 273.15f;
    }
    else
    {
        ntc_data->temperature_ch1 = -273.15f; // 异常值
    }
#else
    ntc_data->temperature_ch1 = -273.15f; // 通道未使能
#endif
}


/**
 * @brief  ADC 转换完成回调函数
 * @param  hadc  ADC 句柄
 * @note   DMA转换完成后自动调用，将ADC值存入数据缓冲区
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        // 存储两个通道的ADC值到数据缓冲区
        NTC_DataBuffer.adc_value_ch0 = ADC_DMA_Buffer[0];
        NTC_DataBuffer.adc_value_ch1 = ADC_DMA_Buffer[1];
    }
    if(hadc->Instance == ADC2)
    {
        // 可扩展其他ADC的处理逻辑
    }
}