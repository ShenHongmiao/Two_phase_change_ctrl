#include "data_packet.h"



PacketData_t packet_data={0};

void pack_data_reset(PacketData_t *packet)//重置，全部置为0
{
    memset(packet, 0, sizeof(PacketData_t));
}


void pack_data(PacketData_t *packet)
{
    pack_data_reset(packet);

    packet->voltage = (int16_t)(Voltage_DataBuffer.voltage * 100);
#if NTC_CHANNEL0_ENABLE
    packet->ntc_temp_ch0 = (int16_t)(NTC_DataBuffer.temperature_ch0 * 100);
#endif

#if NTC_CHANNEL1_ENABLE
    packet->ntc_temp_ch1 = (int16_t)(NTC_DataBuffer.temperature_ch1 * 100);
#endif

#if WF5803F_Enable
    packet->wf_temperature = (int16_t)(WF5803F_DataBuffer.temperature * 100);
    packet->wf_pressure    = (int32_t)(WF5803F_DataBuffer.pressure * 100);
#endif

#if PID_CONTROL_ENABLE
    packet->pid_output_ch0 = (int16_t)(Temp_PID_Controller_CH0.output);
#endif

//PID部分如有需要可在此添加
}   