#include <zephyr/drivers/can.h>

#ifndef INC_CAN_H_
#define INC_CAN_H_

#define CONFIG_CAN_LOOPBACK_MODE (1)

void configure_can_device(const struct device *dev);
void send_can_message(const struct device *dev, int identifier, char *message, uint8_t length);
void send_global_enable_frame(const struct device *dev);
int can_receive_async(const struct device *dev, int identifier);
void stop_receiving(const struct device *dev, int filter_id);

#endif /* INC_CAN_H_ */
