#include "serial_to_pc.h"
#include <stdarg.h>
#include <stdio.h>

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
#if WF5803F_Enable
    case CMD_WF5803F:
        length += sizeof(data->wf_temperature);
        length += sizeof(data->wf_pressure);
        break;
#endif
#if PID_CONTROL_ENABLE              
    case CMD_PID_VALUE:
        length += sizeof(data->pid_output_ch0);
        break;
#endif
    case CMD_VOLTAGE:
        length += sizeof(data->voltage);
        length += sizeof(uint8_t); // 状态字节
        break;
    default:
        break;
    }
    return length;
}

/**
 * @brief 发送数据到上位机,带命令ID判断
 * @param cmd_id 命令ID
 * @param data   数据指针
 */
 //数据结构：帧头(1字节)+命令ID(1字节)+数据长度(1字节)+数据体(n字节)+CRC8(1字节)+帧尾(1字节)
 //帧头: 0xDE，帧尾: 0xED，CRC8采用常见的x^8 + x^2 + x + 1多项式
void send2pc(uint8_t cmd_id, PacketData_t *data,const char *format, ...)
{
    uint8_t frame[256];
    uint16_t index = 0;

    // 帧头
    frame[index++] = FRAME_HEAD;

    // 命令ID
    frame[index++] = cmd_id;

    uint16_t data_len = 0;

    if (cmd_id == CMD_TEXT_INFO) {
        if (format == NULL) {
            return;
        }

        char text_buffer[256];
        va_list args;
        va_start(args, format);
        int text_len = vsnprintf(text_buffer, sizeof(text_buffer), format, args);
        va_end(args);

        if (text_len <= 0) {
            return;
        }

        if (text_len >= (int)sizeof(text_buffer)) {
            text_len = (int)strlen(text_buffer);
        }

        data_len = (uint16_t)text_len;

        if (data_len > UINT8_MAX) {
            data_len = UINT8_MAX;
        }

        size_t remaining = sizeof(frame) - index;
        if (((size_t)data_len + 3) > remaining) {
            return;
        }

        frame[index++] = (uint8_t)data_len;
        memcpy(&frame[index], text_buffer, data_len);
        index += data_len;
    } else {
        if (data == NULL) {
            return;
        }

        data_len = get_packet_data_length(cmd_id, data);
        if (data_len > UINT8_MAX) {
            return;
        }

        size_t remaining = sizeof(frame) - index;
        if (((size_t)data_len + 3) > remaining) {
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


        
#if PID_CONTROL_ENABLE  
        case CMD_PID_VALUE:             
            memcpy(payload + written, &data->pid_output_ch0, sizeof(data->pid_output_ch0));
            written += sizeof(data->pid_output_ch0);
            break;
#endif





        case CMD_VOLTAGE: {
            int16_t voltage_scaled = (int16_t)(Voltage_DataBuffer.voltage * 100.0f);
            memcpy(payload + written, &voltage_scaled, sizeof(voltage_scaled));
            written += sizeof(voltage_scaled);
            uint8_t status = Voltage_DataBuffer.is_normal ? 0x01 : 0xFF;
            payload[written++] = status;
            break;
        }
        default:
            break;
        }

        if (written != data_len) {
            return;
        }

        index += written;
    }

    // CRC8 校验（从帧头到数据体结束）
    uint8_t crc = crc8_calculate(frame, index);
    frame[index++] = crc;

    // 帧尾
    frame[index++] = FRAME_TAIL;

    send_binary_data(frame, index);
}

/**
 * @brief 发送应答消息到上位机
 * @param received_cmd_id 接收到的命令ID（保留参数以保持接口一致，但当前未使用）
 * @param format 应答消息格式（保留参数以保持接口一致，但当前未使用）
 * @param ... 可变参数（保留参数以保持接口一致，但当前未使用）
 */
//应答数据结构：帧头(1字节)+CMD_RESPENSE(1字节)+数据长度(1字节)+固定数据0xB1,0xB2(2字节)+CRC8(1字节)+帧尾(1字节)
void send_response(void)
{
    uint8_t frame[256];
    uint16_t index = 0;

    // 帧头
    frame[index++] = FRAME_HEAD;

    // 命令ID固定为 CMD_RESPENSE
    frame[index++] = CMD_RESPENSE;

    // 数据长度固定为 2 字节
    frame[index++] = 2;

    // 固定应答数据: 0xB1, 0xB2
    frame[index++] = 0xB1;
    frame[index++] = 0xB2;

    // CRC8 校验（从帧头到数据体结束）
    uint8_t crc = crc8_calculate(frame, index);
    frame[index++] = crc;

    // 帧尾
    frame[index++] = FRAME_TAIL;

    send_binary_data(frame, index);
}