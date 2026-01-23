#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "can.h"
#include "pdp.h"
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

K_SEM_DEFINE(can_init_sem, 0, 1);

void can_thread(void)
{
        k_sem_take(&can_init_sem, K_FOREVER);

        PDP pdp;
        int pdp_id = 62;
        pdp_init(&pdp, dev_can, pdp_id);

        while (true)
        {
                pdp_update_channel_currents(&pdp);

                k_sem_take(&pdp.callback00.received, K_MSEC(500));
                k_sem_take(&pdp.callback40.received, K_MSEC(50));
                k_sem_take(&pdp.callback80.received, K_MSEC(50));

                float feedback_voltages[] = {
                    pdp_get_channel_current(&pdp, FRONT_LEFT_WHEEL_PDP_ID),
                    pdp_get_channel_current(&pdp, BACK_LEFT_WHEEL_PDP_ID),
                    pdp_get_channel_current(&pdp, FRONT_RIGHT_WHEEL_PDP_ID),
                    pdp_get_channel_current(&pdp, BACK_RIGHT_WHEEL_PDP_ID),
                    pdp_get_channel_current(&pdp, BUCKET_DRUM_LEFT_PDP_ID),
                    pdp_get_channel_current(&pdp, BUCKET_DRUM_PDP_ID),
                    pdp_get_channel_current(&pdp, LEFT_ACTUATOR_PDP_ID),
                    pdp_get_channel_current(&pdp, RIGHT_ACTUATOR_PDP_ID),
                    pdp_get_bus_voltage(&pdp),
                };

                LOG_INF("PDP Voltages - FL: %.2f, BL: %.2f, FR: %.2f, BR: %.2f, DL: %.2f, D: %.2f, LA: %.2f, RA: %.2f, Bus: %.2f",
                        (double)feedback_voltages[0],
                        (double)feedback_voltages[1],
                        (double)feedback_voltages[2],
                        (double)feedback_voltages[3],
                        (double)feedback_voltages[4],
                        (double)feedback_voltages[5],
                        (double)feedback_voltages[6],
                        (double)feedback_voltages[7],
                        (double)feedback_voltages[8]);

                uint8_t packet[sizeof(feedback_voltages) + 4] = {0};
                packet[0] = 0x01;
                memcpy(&packet[4], feedback_voltages, sizeof(feedback_voltages));
                uart_tx(dev_uart, packet, sizeof(packet), SYS_FOREVER_MS);
        }
}

K_THREAD_DEFINE(can_thread_id, 4096, can_thread, NULL, NULL, NULL, CAN_THREAD_PRIORITY, 0, 0);

int control_thread(void)
{
        while (!talons_initialized())
        {
                LOG_INF("Waiting for talons to initialize...");
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

        k_sem_give(&can_init_sem);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
