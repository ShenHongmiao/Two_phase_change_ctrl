#include "data_packet.h"
#include <string.h>


PacketData_t packet_data={0};

void pack_data_reset(PacketData_t *packet)//重置，全部置为0
{
    memset(packet, 0, sizeof(PacketData_t));
}


void pack_data(PacketData_t *packet)
{
    pack_data_reset(packet);

#if NTC_CHANNEL0_ENABLE
    packet->ntc_temp_ch0 = NTC_DataBuffer.temperature_ch0;
#endif

#if NTC_CHANNEL1_ENABLE
    packet->ntc_temp_ch1 = NTC_DataBuffer.temperature_ch1;
#endif

#if WF5803F_Enable
    packet->wf_temperature = WF5803F_DataBuffer.temperature;
    packet->wf_pressure    = WF5803F_DataBuffer.pressure;
#endif

//PID部分如有需要可在此添加
}