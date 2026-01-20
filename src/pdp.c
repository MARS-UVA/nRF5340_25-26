#include "can.h"
#include "pdp.h"

// arcane nonsense
void pdp_update_cache_words(uint8_t *cache_words, uint64_t *packet_data)
{
    cache_words[0] = (uint8_t)*packet_data;

    short *numPtr1 = cache_words;
    numPtr1[0] = (short)(numPtr1[0] << 2);

    short *numPtr2 = cache_words;
    numPtr2[0] = (short)(numPtr2[0] | ((short)((*packet_data >> 14) & ((unsigned long)3))));
    cache_words[1] = (short)((*packet_data >> 8) & 0x3f);

    short *numPtr3 = &(cache_words[1]);
    numPtr3[0] = (short)(numPtr3[0] << 4);

    short *numPtr4 = &(cache_words[1]);
    numPtr4[0] = (short)(numPtr4[0] | ((short)((*packet_data >> 20) & 15)));
    cache_words[2] = (short)((*packet_data >> 0x10) & 15);

    short *numPtr5 = &(cache_words[2]);
    numPtr5[0] = (short)(numPtr5[0] << 6);

    short *numPtr6 = &(cache_words[2]);
    numPtr6[0] = (short)(numPtr6[0] | ((short)((*packet_data >> 0x1a) & 0x3f)));
    cache_words[3] = (short)((*packet_data >> 0x18) & ((unsigned long)3));

    short *numPtr7 = &(cache_words[3]);
    numPtr7[0] = (short)(numPtr7[0] << 8);

    short *numPtr8 = &(cache_words[3]);
    numPtr8[0] = (short)(numPtr8[0] | ((uint8_t)(*packet_data >> 0x20)));
    cache_words[4] = (*packet_data >> 40);

    short *numPtr9 = &(cache_words[4]);
    numPtr9[0] = (short)(numPtr9[0] << 2);

    short *numPtr10 = &(cache_words[4]);
    numPtr10[0] = (short)(numPtr10[0] | ((short)((*packet_data >> 0x36) & ((unsigned long)3))));
    cache_words[5] = (short)((*packet_data >> 0x30) & 0x3f);

    short *numPtr11 = &(cache_words[5]);
    numPtr11[0] = (short)(numPtr11[0] << 4);

    short *numPtr12 = &(cache_words[5]);
    numPtr12[0] = (short)(numPtr12[0] | ((short)((*packet_data >> 60) & 15)));
}

void pdp_update_channel_currents(PDP *pdp)
{
    uint32_t identifier = 0x8041640 | pdp->identifier;
    char *message = "\x00\x00\x00\x00\x20\x00";
    int len = 6;
    send_can_message(pdp->can_device, identifier, message, len);
}

float pdp_get_channel_current(PDP *pdp, int channel_id)
{
    return pdp->channel_currents[channel_id] * 0.125f;
}

// maybe unnecessary function
float pdp_get_bus_voltage(PDP *pdp)
{
    return pdp->bus_voltage;
}

void pdp_rx_current_callback(const struct device *dev, struct can_frame *frame, pdp_callback_data *info)
{
    pdp_update_cache_words(info->current_arr, (uint64_t *)&frame->data);

    if (info->voltage_ptr != NULL)
    {
        *info->voltage_ptr = ((frame->data[6] << 8) | frame->data[7]) / 3444.0;
    }

    k_sem_give(&info->received);
}

void pdp_init(PDP *pdp, const struct device *can_device, uint32_t identifier)
{
    pdp->can_device = can_device;
    pdp->identifier = identifier;

    pdp->bus_voltage = -1;
    for (int i = 0; i < 18; i++)
    {
        pdp->channel_currents[i] = -1;
    }

    pdp->callback00.current_arr = &(pdp->channel_currents[0]);
    pdp->callback00.voltage_ptr = NULL;
    k_sem_init(&pdp->callback00.received, 0, 1);

    pdp->callback40.current_arr = &(pdp->channel_currents[6]);
    pdp->callback40.voltage_ptr = NULL;
    k_sem_init(&pdp->callback40.received, 0, 1);

    pdp->callback80.current_arr = &(pdp->channel_currents[12]);
    pdp->callback80.voltage_ptr = &pdp->bus_voltage;
    k_sem_init(&pdp->callback80.received, 0, 1);

    can_start_receiving_callback(can_device, pdp_rx_current_callback, &pdp->callback00, 0x8041400 | pdp->identifier);
    can_start_receiving_callback(can_device, pdp_rx_current_callback, &pdp->callback40, 0x8041440 | pdp->identifier);
    can_start_receiving_callback(can_device, pdp_rx_current_callback, &pdp->callback80, 0x8041480 | pdp->identifier);
}