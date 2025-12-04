#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include "control.h"
#include "serial.h"
#include "wifi.h"
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(main);

// #define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);
const struct device *dev_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

#define CONTROL_STACK_SIZE 1024
#define WIFI_STACK_SIZE 4096

K_MSGQ_DEFINE(serial_msgq, sizeof(serial_packet_t), 4, 4);

talon_fx_t motor;
talon_srx_t actuator;

K_SEM_DEFINE(wifi_init_sem, 0, 1);

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

K_THREAD_DEFINE(control_thread_id, CONTROL_STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

int wifi_thread(void)
{
        k_sem_take(&wifi_init_sem, K_FOREVER);

        LOG_INF("Starting WiFi thread...");

        int sock = open_udp_socket(SERVER_IP, SERVER_PORT);
        if (sock < 0)
        {
                LOG_ERR("Failed to open UDP socket.");
                return -1;
        }

        LOG_INF("UDP socket opened successfully.");

        zsock_send(sock, "CONNECT", 7, 0);

        LOG_INF("Sent connect packet.");

        while (1)
        {
                // Receive data
                char buffer[256];

                ssize_t len = zsock_recv(sock, buffer, sizeof(buffer) - 1, 0);

                LOG_DBG("Received %zd bytes over UDP", len);

                // Check for errors
                if (len < 0)
                {
                        LOG_ERR("Error receiving data: %d", len);
                        continue;
                }

                // Check for start byte
                for (int i = 0; i < len; i++)
                {
                        if ((uint8_t)buffer[i] != SERIAL_START_BYTE)
                        {
                                continue;
                        }

                        // Ensure we have enough data for a full packet
                        if (i + SIZEOF_SERIAL_PACKET > len)
                        {
                                LOG_WRN("Incomplete packet received, dropping packet.");
                                break;
                        }

                        // Parse packet
                        serial_packet_t pkt;
                        memcpy(&pkt.start_byte, &buffer[i], SIZEOF_SERIAL_PACKET);

                        // Enqueue the packet
                        int err = k_msgq_put(&serial_msgq, &pkt, K_NO_WAIT);
                        if (err != 0)
                        {
                                LOG_WRN("Serial message queue full, dropping packet.");
                        }

                        i += SIZEOF_SERIAL_PACKET;
                }

                k_msleep(10);
        }

        int ret = close_udp_socket(sock);
        if (ret < 0)
        {
                LOG_ERR("Failed to close UDP socket: %d", ret);
        }

        return 0;
}

K_THREAD_DEFINE(wifi_thread_id, 4096, wifi_thread, NULL, NULL, NULL, WIFI_THREAD_PRIORITY, 0, 0);

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);
        initialize_talons(dev_can);

        // configure_uart_device(dev_uart, &serial_msgq);
        configure_wifi();
        k_sem_give(&wifi_init_sem);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
