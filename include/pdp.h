#ifndef INC_PDP_H_
#define INC_PDP_H_

#include <stdint.h>
#include <zephyr/kernel.h>
#include "can.h"

typedef struct
{
    uint8_t* current_arr;  // pointer into PDP.channel_currents
    float* voltage_ptr;
    struct k_sem received;
} pdp_callback_data;

typedef struct
{
    struct device *can_device;
    uint32_t identifier;
    pdp_callback_data callback00;
    pdp_callback_data callback40;
    pdp_callback_data callback80;
    uint8_t channel_currents[18];
    float bus_voltage;
} PDP;

void pdp_update_channel_currents(PDP *pdp);
float pdp_get_channel_current(PDP *pdp, int channel_id);
float pdp_get_bus_voltage(PDP *pdp);
void pdp_init(PDP *pdp, const struct device *can_device, uint32_t identifier);

#endif