#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include "bno055.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include "can.h"
#include "talon_fx.h"

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);
static const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);

        talon_fx_t motor = talon_fx_init(dev_can, 27);

        while (1)
        {
                send_global_enable_frame(dev_can);
                k_msleep(10);
                motor.set(&motor, 0.5);
                k_msleep(10);
        }

        return 0;
}
