#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include "bno055.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)
#define CONFIG_LOOPBACK_MODE (0)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C1_NODE);
// static const struct spi_dt_spec dev_can = SPI_DT_SPEC_GET(SPI1_NODE, SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0);
static const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);

void write_register(const struct i2c_dt_spec *spec, uint8_t reg, uint8_t value)
{
        uint8_t buf[2] = {reg, value};
        int ret = i2c_write_dt(spec, buf, sizeof(buf));
        if (ret)
        {
                printk("Failed to write to device: %d\n", ret);
        }
}

#define GLOBAL_ENABLE_FRAME "\x01\x00"

void send_can_message(const struct device *dev, int identifier, char *message, uint8_t length)
{
        struct can_frame frame = {
            .flags = CAN_FRAME_IDE,
            .id = identifier,
            .dlc = length,
        };
        memcpy(frame.data, message, length);

        printk("Sending CAN message with ID: 0x%X\n", identifier);
        printk("Data: ");
        for (int i = 0; i < length; i++)
        {
                printk("0x%02X ", frame.data[i]);
        }
        printk("\n");

        int ret = can_send(dev, &frame, K_FOREVER, NULL, NULL);

        printk("can_send returned: %d\n", ret);

        if (ret != 0)
        {
                printk("Failed to send CAN message: %d\n", ret);
        }
}

// void sendCANMessage(CAN_HandleTypeDef *hcan, int identifier, char *message, uint8_t length)
// {
// 	  uint32_t mb;
// 	  CAN_TxHeaderTypeDef hdr;

// 	  hdr.ExtId = identifier;
// 	  hdr.IDE = CAN_ID_EXT;
// 	  hdr.RTR = CAN_RTR_DATA;
// 	  hdr.DLC = length;
// 	  hdr.TransmitGlobalTime = DISABLE;

// 	  if (HAL_CAN_AddTxMessage(hcan, &hdr, (unsigned char *) message, &mb) != HAL_OK)
// 		Error_Handler();
// }

// void sendGlobalEnableFrame(CAN_HandleTypeDef *hcan)
// {
// 	  uint32_t mb;
// 	  CAN_TxHeaderTypeDef hdr;

// 	  hdr.ExtId = 0x401bf;
// 	  hdr.IDE = CAN_ID_EXT;
// 	  hdr.RTR = CAN_RTR_DATA;
// 	  hdr.DLC = 2;
// 	  hdr.TransmitGlobalTime = DISABLE;

// 	  if (HAL_CAN_AddTxMessage(hcan, &hdr, (unsigned char *) "\x01\x00", &mb) != HAL_OK)
// 		Error_Handler();
// }

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

        int ret;

        if (!device_is_ready(dev_can))
        {
                printk("SPI bus %s is not ready!\n\r", dev_can->name);
                return 0;
        }

        // Check that SPI CS is working
        // int ret = spi_write_dt(&dev_can, "0");
        // if (ret)
        // {
        //         printk("Failed to write to CAN device: %d\n", ret);
        //         return 0;
        // }

        k_msleep(100);

#if (CONFIG_LOOPBACK_MODE)
        ret = can_set_mode(dev_can, CAN_MODE_LOOPBACK);
#else
        ret = can_set_mode(dev_can, CAN_MODE_NORMAL);
#endif
        if (ret != 0)
        {
                printf("Error setting CAN mode [%d]", ret);
                return 0;
        }

        ret = can_start(dev_can);

        // print can mode
        printk("CAN mode: %d\n", can_get_mode(dev_can));

        if (ret != 0)
        {
                printf("Error starting CAN controller [%d]", ret);
                return 0;
        }

        k_msleep(10);

        printk("CAN initialized\n");

        while (1)
        {
                send_can_message(dev_can, 0x401bf, GLOBAL_ENABLE_FRAME, 2);
                k_msleep(10);
                int value = 127;
                send_can_message(dev_can, 27 | 0x204b540, (char[]){0, 1, 0, 0, 0, 0, value & 0xFF, (value >> 8) & 0xFF}, 8);
                k_msleep(10);
                printk("Message sent\n");
        }

        return 0;
}
