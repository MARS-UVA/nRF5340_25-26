#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include <zephyr/logging/log.h>
#include "control.h"

LOG_MODULE_REGISTER(control, LOG_LEVEL_INF);

extern const struct device *dev_can;

talon_fx_t front_left_motor;
talon_fx_t back_left_motor;
talon_fx_t front_right_motor;
talon_fx_t back_right_motor;
talon_fx_t bucket_drum_motor;

talon_srx_t left_actuator;
talon_srx_t right_actuator;

void direct_control(serial_packet_t *pkt)
{
    send_global_enable_frame(dev_can);

    uint8_t left_speed = pkt->front_left_wheel;
    uint8_t right_speed = pkt->front_right_wheel;

    front_left_motor.set(&front_left_motor, (left_speed - 127) / 128.0);
    back_left_motor.set(&back_left_motor, (left_speed - 127) / 128.0);

    front_right_motor.set(&front_right_motor, (right_speed - 127) / 128.0);
    back_right_motor.set(&back_right_motor, (right_speed - 127) / 128.0);

    bucket_drum_motor.set(&bucket_drum_motor, (pkt->drum - 127) / 128.0);

    if (DIRECT_ACTUATOR_CONTROL)
    {
        float actuator_output = 0.0;
        if (pkt->actuator > 0x7f)
            actuator_output = 1.0;
        else if (pkt->actuator < 0x7f)
            actuator_output = -1.0;

        left_actuator.set(&left_actuator, actuator_output);
        right_actuator.set(&right_actuator, actuator_output);
    }
    else
    {
        // int8_t actuator_position = pkt->actuator;
        // float percent_extension = actuator_position / 255.0;
        // Position control logic can be implemented here
        LOG_INF("Actuator position control not implemented.");
    }
}
