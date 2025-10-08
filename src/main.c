#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include "bno055.h"

// #define I2C1_NODE DT_NODELABEL(imu)
// static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

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

        if (!adc_is_ready_dt(&adc_channel))
        {
                printk("ADC controller device %s not ready\n", adc_channel.dev->name);
                return 0;
        }

        int err = adc_channel_setup_dt(&adc_channel);
        if (err < 0)
        {
                printk("Could not setup channel #%d (%d)\n", 0, err);
                return 0;
        }

        int32_t buf;
        struct adc_sequence sequence = {
            .buffer = &buf,
            /* buffer size in bytes, not number of samples */
            .buffer_size = sizeof(buf),
            // Optional
            //.calibrate = true,
        };

        err = adc_sequence_init_dt(&adc_channel, &sequence);
        if (err < 0)
        {
                printk("Could not initalize sequnce\n");
                return 0;
        }

        while (1)
        {
                err = adc_read(adc_channel.dev, &sequence);
                if (err < 0)
                {
                        printk("Could not read (%d)\n", err);
                        continue;
                }

                err = adc_raw_to_millivolts_dt(&adc_channel, &buf);
                /* conversion to mV may not be supported, skip if not */
                if (err < 0)
                {
                        printk(" (value in mV not available)\n");
                }
                else
                {
                        printf("%d mV\n", buf);
                }

                k_msleep(100);
        }

        return 0;
}
