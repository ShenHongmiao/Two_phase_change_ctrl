#include "serial_to_pc.h"
#include "usart.h"
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

uint16_t get_packet_data_length(uint8_t cmd_id, PacketData_t *data)
{
    uint16_t length = 0;

    switch (cmd_id) {
    case CMD_NTC:
#if NTC_CHANNEL0_ENABLE
        length += sizeof(data->ntc_temp_ch0);
#endif
#if NTC_CHANNEL1_ENABLE
        length += sizeof(data->ntc_temp_ch1);
#endif
        break;
    case CMD_WF5803F:
#if WF5803F_Enable
        length += sizeof(data->wf_temperature);
        length += sizeof(data->wf_pressure);
#endif
        break;
    default:
        break;
    }
    return length;
}


void send2pc(uint8_t cmd_id, PacketData_t *data)
{
    uint8_t frame[256];  // 根据需要调整最大长度
    uint16_t index = 0;

    if (data == NULL) {
        return;
    }

    // 帧头
    frame[index++] = FRAME_HEAD;

    // 命令ID
    frame[index++] = cmd_id;

    // 数据长度
    uint16_t data_len = get_packet_data_length(cmd_id, data);
    if (data_len > UINT8_MAX) {
        return;
    }
    size_t remaining = sizeof(frame) - index;
    if (((size_t)data_len + 3) > remaining) { // 预留长度字节+CRC+帧尾
        return;
    }

    frame[index++] = (uint8_t)data_len;

    uint8_t *payload = &frame[index];
    uint16_t written = 0;

    switch (cmd_id) {
    case CMD_NTC:
#if NTC_CHANNEL0_ENABLE
        memcpy(payload + written, &data->ntc_temp_ch0, sizeof(data->ntc_temp_ch0));
        written += sizeof(data->ntc_temp_ch0);
#endif
#if NTC_CHANNEL1_ENABLE
        memcpy(payload + written, &data->ntc_temp_ch1, sizeof(data->ntc_temp_ch1));
        written += sizeof(data->ntc_temp_ch1);
#endif
        break;
    
#if WF5803F_Enable                  
    case CMD_WF5803F:  
        memcpy(payload + written, &data->wf_temperature, sizeof(data->wf_temperature));
        written += sizeof(data->wf_temperature);
        memcpy(payload + written, &data->wf_pressure, sizeof(data->wf_pressure));
        written += sizeof(data->wf_pressure);
        break;
#endif
    default:
        break;
    }

    if (written != data_len) {
        return;
    }

    index += written;

    // CRC8 校验（从帧头到数据体结束）
    uint8_t crc = crc8_calculate(frame, index);
    frame[index++] = crc;

    // 帧尾
    frame[index++] = FRAME_TAIL;

    // 发送数据帧
    send_binary_data(frame, index);
}