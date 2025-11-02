#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include "control.h"
#include "serial.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);

#define STACK_SIZE 1024

talon_fx_t motor;
talon_srx_t actuator;

int control_thread(void)
{
        while (!talons_initialized())
        {
                k_msleep(100);
        }

        while (1)
        {
                serial_packet_t pkt = {
                    .front_left_wheel = 0x00,
                    .back_left_wheel = 0x80,
                    .front_right_wheel = 0x80,
                    .back_right_wheel = 0x80,
                    .drum = 0x80,
                    .actuator = 0xFF,
                };
                direct_control(&pkt);
        }
        return 0;
}

K_THREAD_DEFINE(control_thread_id, STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);

        initialize_talons(dev_can);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
