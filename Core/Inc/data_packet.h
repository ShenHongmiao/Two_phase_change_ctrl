#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__

#include "NTC.h"
#include "WF5803F.h"
// #include "PID.h"

typedef struct {
#if NTC_CHANNEL0_ENABLE
    float ntc_temp_ch0;
#endif

#if NTC_CHANNEL1_ENABLE
    float ntc_temp_ch1;
#endif

#if WF5803F_Enable
    float wf_temperature;
    float wf_pressure;
#endif

#if ENABLE_PID
    float pid_output;
    float pid_target;
    float pid_error;
#endif
} PacketData_t;

extern PacketData_t packet_data;

void pack_data_reset(PacketData_t *packet);
void pack_data(PacketData_t *packet);

#endif // __DATA_PACKET_H__