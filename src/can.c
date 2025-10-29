#include "can.h"
#include <zephyr/logging/log.h>

#define GLOBAL_ENABLE_FRAME "\x01\x00"

LOG_MODULE_REGISTER(can_bus, LOG_LEVEL_DBG);

void configure_can_device(const struct device *dev)
{
    int ret;

    if (!device_is_ready(dev))
    {
        printk("CAN device %s is not ready!\n\r", dev->name);
        return;
    }

#if (CONFIG_CAN_LOOPBACK_MODE)
    ret = can_set_mode(dev, CAN_MODE_LOOPBACK);
#else
    ret = can_set_mode(dev, CAN_MODE_NORMAL);
#endif
    if (ret != 0)
    {
        LOG_ERR("Error setting CAN mode [%d]", ret);
        return 0;
    }

    k_msleep(10);

    ret = can_start(dev);

    if (ret != 0)
    {
        LOG_ERR("Error starting CAN controller [%d]", ret);
        return 0;
    }

    LOG_INF("CAN device %s configured.\n", dev->name);
}

void send_can_message(const struct device *dev, int identifier, char *message, uint8_t length)
{
    struct can_frame frame = {
        .flags = CAN_FRAME_IDE,
        .id = identifier,
        .dlc = length,
    };
    memcpy(frame.data, message, length);

    LOG_DBG("Sending CAN message with ID: 0x%X\n", identifier);
    LOG_DBG("Data: ");
    for (int i = 0; i < length; i++)
    {
        LOG_DBG("0x%02X ", frame.data[i]);
    }
    LOG_DBG("\n");

    int ret = can_send(dev, &frame, K_FOREVER, NULL, NULL);

    if (ret != 0)
    {
        LOG_ERR("Failed to send CAN message: %d\n", ret);
    }
}

void send_global_enable_frame(const struct device *dev)
{
    send_can_message(dev, 0x401bf, GLOBAL_ENABLE_FRAME, 2);
}
