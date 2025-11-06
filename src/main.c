#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include "control.h"
#include "serial.h"

LOG_MODULE_REGISTER(main);

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);
const struct device *dev_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

#define STACK_SIZE 1024

K_MSGQ_DEFINE(serial_msgq, sizeof(serial_packet_t), 4, 4);

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
                serial_packet_t pkt;
                int err = k_msgq_get(&serial_msgq, &pkt, K_MSEC(CONTROL_TIMEOUT_MS));
                if (err != 0)
                {
                        pkt = (serial_packet_t){
                            .front_left_wheel = 0x7f,
                            .back_left_wheel = 0x7f,
                            .front_right_wheel = 0x7f,
                            .back_right_wheel = 0x7f,
                            .drum = 0x7f,
                            .actuator = 0x7f,
                        };
                }

                LOG_INF("Packet - FL: %d, BL: %d, FR: %d, BR: %d, Drum: %d, Actuator: %d",
                        pkt.front_left_wheel,
                        pkt.back_left_wheel,
                        pkt.front_right_wheel,
                        pkt.back_right_wheel,
                        pkt.drum,
                        pkt.actuator);

                direct_control(&pkt);
                k_msleep(10);
        }
        return 0;
}

K_THREAD_DEFINE(control_thread_id, STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);
        configure_uart_device(dev_uart, &serial_msgq);

        initialize_talons(dev_can);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
