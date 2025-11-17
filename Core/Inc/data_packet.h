#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include "NTC.h"
#include "WF5803F.h"
#include "V_Detect.h"
#include "temp_pid_ctrl.h"


#include <string.h>


typedef struct {
    int16_t voltage;      // 电压 * 100 (V)
#if NTC_CHANNEL0_ENABLE
    int16_t ntc_temp_ch0;  // 温度 * 100 (℃)
#endif

#if NTC_CHANNEL1_ENABLE
    int16_t ntc_temp_ch1;  // 温度 * 100 (℃)
#endif

#if WF5803F_Enable
    int16_t wf_temperature;  // 温度 * 100 (℃)
    int32_t wf_pressure;     // 压力 * 100 (kPa)
#endif
#if PID_CONTROL_ENABLE
    int16_t pid_output_ch0; // PID输出值 (0-1000ms)
#endif

// PID 部分如有需要可在此添加
} PacketData_t;

extern PacketData_t packet_data;

void pack_data_reset(PacketData_t *packet);
void pack_data(PacketData_t *packet);

#endif // __DATA_PACKET_H__