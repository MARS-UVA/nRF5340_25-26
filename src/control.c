#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include <zephyr/logging/log.h>
#include "control.h"

LOG_MODULE_REGISTER(control);

extern const struct device *dev_can;

talon_fx_t front_left_motor;
talon_fx_t back_left_motor;
talon_fx_t front_right_motor;
talon_fx_t back_right_motor;
talon_fx_t bucket_drum_motor;
talon_fx_t bucket_drum_left_motor;

talon_srx_t left_actuator;
talon_srx_t right_actuator;

void direct_control(serial_packet_t *pkt)
{
    send_global_enable_frame(dev_can);

    uint8_t left_speed = pkt->front_left_wheel;
    uint8_t right_speed = pkt->front_right_wheel;

    front_left_motor.set_control(&front_left_motor, -(left_speed - 127), 0);
    back_left_motor.set_control(&back_left_motor, -(left_speed - 127), 0);

    front_right_motor.set_control(&front_right_motor, (right_speed - 127), 0);
    back_right_motor.set_control(&back_right_motor, (right_speed - 127), 0);

    bucket_drum_motor.set_control(&bucket_drum_motor, (pkt->drum - 127), 0);
    bucket_drum_left_motor.set_control(&bucket_drum_left_motor, -(pkt->drum - 127), 0);

    if (DIRECT_ACTUATOR_CONTROL)
    {
        float actuator_output = -(pkt->actuator - 127) / 127.0;

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

bool talons_initialized(void)
{
    return front_left_motor.initialized &&
           back_left_motor.initialized &&
           front_right_motor.initialized &&
           back_right_motor.initialized &&
           bucket_drum_motor.initialized &&
           bucket_drum_left_motor.initialized &&
           left_actuator.initialized &&
           right_actuator.initialized;
}

void initialize_talons(const struct device *can_dev)
{
    talon_fx_init(&front_left_motor, can_dev, FRONT_LEFT_WHEEL_ID);
    talon_fx_init(&back_left_motor, can_dev, BACK_LEFT_WHEEL_ID);
    talon_fx_init(&front_right_motor, can_dev, FRONT_RIGHT_WHEEL_ID);
    talon_fx_init(&back_right_motor, can_dev, BACK_RIGHT_WHEEL_ID);
    talon_fx_init(&bucket_drum_motor, can_dev, BUCKET_DRUM_ID);
    talon_fx_init(&bucket_drum_left_motor, can_dev, BUCKET_DRUM_LEFT_ID);

    talon_srx_init(&left_actuator, can_dev, LEFT_ACTUATOR_ID, false);
    talon_srx_init(&right_actuator, can_dev, RIGHT_ACTUATOR_ID, false);
}
