#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include "bno055.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include "can.h"
#include "talon_fx.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

static const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);

#define STACK_SIZE 1024
#define CONTROL_THREAD_PRIORITY 5

talon_fx_t motor;

int control_thread(void)
{
        while (!motor.initialized)
        {
                k_msleep(100);
        }

        while (1)
        {
                send_global_enable_frame(dev_can);
                motor.set(&motor, 0.5);
        }
        return 0;
}

K_THREAD_DEFINE(control_thread_id, STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);

        talon_fx_init(&motor, dev_can, 27);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
