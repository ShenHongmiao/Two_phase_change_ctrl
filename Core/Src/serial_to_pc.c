#include "serial_to_pc.h"
#include <string.h>

uint8_t crc8_calculate(const uint8_t *data, uint16_t length) {
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
        }
    }
    return crc;
}

uint16_t get_packet_data_length(PacketData_t *data)
{
    uint16_t length = 0;

#if NTC_CHANNEL0_ENABLE
    length += sizeof(data->ntc_temp_ch0);
#endif

#if NTC_CHANNEL1_ENABLE
    length += sizeof(data->ntc_temp_ch1);
#endif

#if WF5803F_Enable
    length += sizeof(data->wf_temperature);
    length += sizeof(data->wf_pressure);
#endif
    return length;
}


void send2pc(uint8_t cmd_id, PacketData_t *data)
{
    uint8_t frame[256];  // 根据需要调整最大长度
    uint16_t index = 0;

    // 帧头
    frame[index++] = FRAME_HEAD;

    // 命令ID
    frame[index++] = cmd_id;

    // 数据长度
    uint16_t data_len = get_packet_data_length(data);
    frame[index++] = (uint8_t)data_len;

    // 数据体 - 按照宏定义顺序打包
    uint16_t offset = 0;
#if NTC_CHANNEL0_ENABLE
    memcpy(&frame[index + offset], &data->ntc_temp_ch0, sizeof(data->ntc_temp_ch0));
    offset += sizeof(data->ntc_temp_ch0);
#endif

#if NTC_CHANNEL1_ENABLE
    memcpy(&frame[index + offset], &data->ntc_temp_ch1, sizeof(data->ntc_temp_ch1));
    offset += sizeof(data->ntc_temp_ch1);
#endif

#if WF5803F_Enable
    memcpy(&frame[index + offset], &data->wf_temperature, sizeof(data->wf_temperature));
    offset += sizeof(data->wf_temperature);
    memcpy(&frame[index + offset], &data->wf_pressure, sizeof(data->wf_pressure));
    offset += sizeof(data->wf_pressure);
#endif

    index += data_len;

    // CRC8 校验（从帧头到数据体结束）
    uint8_t crc = crc8_calculate(frame, index);
    frame[index++] = crc;

    // 帧尾
    frame[index++] = FRAME_TAIL;

    // 使用 send_message_direct 直接发送二进制数据
    // 将二进制数据转换为可打印的十六进制字符串形式，或直接发送原始字节
    // 方法1：直接使用阻塞发送（适合二进制数据）
    HAL_UART_Transmit(&huart2, frame, index, 1000);
}