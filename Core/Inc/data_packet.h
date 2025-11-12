#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include "NTC.h"
#include "WF5803F.h"
// #include "PID.h"

typedef struct {
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

// PID 部分如有需要可在此添加
} PacketData_t;

extern PacketData_t packet_data;

void pack_data_reset(PacketData_t *packet);
void pack_data(PacketData_t *packet);

#endif // __DATA_PACKET_H__