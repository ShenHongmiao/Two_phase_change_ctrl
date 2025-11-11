#ifndef SERIAL_TO_PC_H
#define SERIAL_TO_PC_H

#include "usart.h"
#include <stdint.h>
#include "data_packet.h"



uint8_t crc8_calculate(const uint8_t *data, uint16_t length);// CRC8 计算函数（常见 x^8 + x^2 + x + 1 多项式）
void send2pc(uint8_t cmd_id, PacketData_t *data, uint16_t len);// 发送数据到上位机,带命令ID判断




#endif /* SERIAL_TO_PC_H */