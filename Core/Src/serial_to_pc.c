#include "serial_to_pc.h"


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

void send2pc(uint8_t cmd_id, PacketData_t *data, uint16_t len)
{
    uint8_t frame[256];  // 根据需要调整最大长度
    uint16_t index = 0;

    // 帧头
    frame[index++] = FRAME_HEAD;

    // 命令ID
    frame[index++] = cmd_id;

    // 数据长度
    frame[index++] = (uint8_t)len;

    // 数据体
    memcpy(&frame[index], data, len);
    index += len;

    // CRC8 校验（从帧头到数据体结束）
    uint8_t crc = crc8_calculate(frame, index);
    frame[index++] = crc;

    // 帧尾
    frame[index++] = FRAME_TAIL;

    // 调用原 send_message() 发送（将字节流当作字符串格式发送，使用 %s 传入即可）
    // 由于 send_message() 接口是 char* 类型，可以直接强制转换
    send_message(0, "%.*s", index, (char*)frame);
}