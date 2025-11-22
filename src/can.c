#include "can.h"
#include <zephyr/logging/log.h>

#define GLOBAL_ENABLE_FRAME "\x01\x00"

LOG_MODULE_REGISTER(can_bus);

void configure_can_device(const struct device *dev)
{
    int ret;

    if (!device_is_ready(dev))
    {
        LOG_ERR("CAN device %s is not ready!", dev->name);
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
        return;
    }

    k_msleep(10);

    ret = can_start(dev);

    if (ret != 0)
    {
        LOG_ERR("Error starting CAN controller [%d]", ret);
        return;
    }

    LOG_INF("CAN device %s configured.", dev->name);
}

void send_can_message(const struct device *dev, int identifier, char *message, uint8_t length)
{
    struct can_frame frame = {
        .flags = CAN_FRAME_IDE,
        .id = identifier,
        .dlc = length,
    };
    memcpy(frame.data, message, length);

    LOG_DBG("Sending CAN message with ID: 0x%X (%d bytes)", identifier, length);

    int ret = can_send(dev, &frame, K_FOREVER, NULL, NULL);

    if (ret != 0)
    {
        LOG_ERR("Failed to send CAN message: %d", ret);
    }
}

void _rx_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
    /* !!! n.b. "The callback function is called from an interrupt context,
     *           which means that the callback function should be as short
     *           as possible and must not block." !!!
     */
    LOG_INF("TODO: Do something with can message (recvd btw)");
}

int can_receive_async(const struct device *dev, int identifier)
{
    LOG_INF("Setting up can receive, id %d", identifier);
    const struct can_filter filter = {
        .flags = 0U,
        .id = identifier,
        .mask = CAN_STD_ID_MASK,
    };

    void *user_data_for_callback_fn = NULL;

    int filter_id = can_add_rx_filter(dev, _rx_callback, user_data_for_callback_fn, &filter);

    if (filter_id < 0)
    {
        LOG_ERR("Unable to add rx filter [%d]", filter_id);
    }
    return filter_id;
}

void stop_receiving(const struct device *dev, int filter_id)
{
    can_remove_rx_filter(dev, filter_id);
}

void send_global_enable_frame(const struct device *dev)
{
    send_can_message(dev, 0x401bf, GLOBAL_ENABLE_FRAME, 2);
}
