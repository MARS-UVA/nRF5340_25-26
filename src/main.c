#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include "bno055.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>

#define I2C1_NODE DT_NODELABEL(imu)
#define SPI1_NODE DT_NODELABEL(can)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);
static const struct spi_dt_spec dev_can = SPI_DT_SPEC_GET(SPI1_NODE, SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0);

void write_register(const struct i2c_dt_spec *spec, uint8_t reg, uint8_t value)
{
        uint8_t buf[2] = {reg, value};
        int ret = i2c_write_dt(spec, buf, sizeof(buf));
        if (ret)
        {
                printk("Failed to write to device: %d\n", ret);
        }
}

int main(void)
{
        // printk("Hello World! %s\n", CONFIG_BOARD);

        // if (!device_is_ready(dev_i2c.bus))
        // {
        //         printk("I2C bus %s is not ready!\n\r", dev_i2c.bus->name);
        //         return 0;
        // }

        // write_register(&dev_i2c, BNO055_OPR_MODE_ADDR, BNO055_OPERATION_MODE_CONFIG);

        // /* Reset */
        // // write_register(&dev_i2c, BNO055_SYS_TRIGGER_ADDR, 0x20);

        // write_register(&dev_i2c, BNO055_PWR_MODE_ADDR, BNO055_POWER_MODE_NORMAL);
        // k_msleep(10);

        // write_register(&dev_i2c, BNO055_PAGE_ID_ADDR, 0);

        // write_register(&dev_i2c, BNO055_SYS_TRIGGER_ADDR, 0x0);
        // k_msleep(10);

        // write_register(&dev_i2c, BNO055_OPR_MODE_ADDR, BNO055_OPERATION_MODE_NDOF);
        // k_msleep(500);

        // printk("IMU configured\n");

        // // read
        // while (1)
        // {
        //         int16_t data;
        //         int ret = i2c_burst_read_dt(&dev_i2c, BNO055_ACCEL_DATA_X_LSB_ADDR, (uint8_t *)&data, sizeof(data));
        //         if (ret)
        //         {
        //                 printk("Failed to read from IMU: %d\n", ret);
        //                 return 0;
        //         }

        //         printk("IMU data: %d\n", data);

        //         k_sleep(K_MSEC(100));
        // }

        if (!device_is_ready(dev_can.bus))
        {
                printk("SPI bus %s is not ready!\n\r", dev_can.bus->name);
                return 0;
        }

        printk("CAN configured\n");

        int ret;

#ifdef CONFIG_LOOPBACK_MODE
        ret = can_set_mode(dev_can, CAN_MODE_LOOPBACK);
        if (ret != 0)
        {
                printf("Error setting CAN mode [%d]", ret);
                return 0;
        }
#endif

        ret = can_start(dev_can.bus);
        if (ret != 0)
        {
                printf("Error starting CAN controller [%d]", ret);
                return 0;
        }

        return 0;
}
