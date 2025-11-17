#ifndef SERIAL_TO_PC_H
#define SERIAL_TO_PC_H

#include "usart.h"
#include "data_packet.h"
#include "V_Detect.h"
#include "temp_pid_ctrl.h"
#include <string.h>
#include <stdint.h>



uint8_t crc8_calculate(const uint8_t *data, uint16_t length);// CRC8 计算函数（常见 x^8 + x^2 + x + 1 多项式）
uint16_t get_packet_data_length(uint8_t cmd_id, PacketData_t *data);// 根据命令ID获取数据长度
void send2pc(uint8_t cmd_id, PacketData_t *data,const char *format, ...);// 发送数据到上位机,带命令ID判断
void send_response(void);// 发送应答消息到上位机




#endif /* SERIAL_TO_PC_H */