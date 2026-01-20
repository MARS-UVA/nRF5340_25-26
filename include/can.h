#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>

#define CONFIG_CAN_LOOPBACK_MODE (0)

#define CAN_THREAD_PRIORITY 6

void configure_can_device(const struct device *dev);
void send_can_message(const struct device *dev, uint32_t identifier, char *message, uint8_t length);
void send_global_enable_frame(const struct device *dev);
int can_start_receiving_msgq(const struct device *dev, struct k_msgq *message_queue, uint32_t identifier);
int can_start_receiving_callback(const struct device *dev, can_rx_callback_t callback_fn, void *callback_args, uint32_t identifier);
void can_stop_receiving(const struct device *dev, int filter_id);

#endif /* INC_CAN_H_ */
