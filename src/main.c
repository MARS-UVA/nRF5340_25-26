#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include "bno055.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/can.h>
#include "can.h"
#include "talon_fx.h"
#include "talon_srx.h"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define I2C1_NODE DT_NODELABEL(imu)
#define CAN1_NODE DT_NODELABEL(can)

static const struct device *dev_can = DEVICE_DT_GET(CAN1_NODE);

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

#define STACK_SIZE 1024
#define CONTROL_THREAD_PRIORITY 5

talon_fx_t motor;
talon_srx_t actuator;

int control_thread(void)
{
        while (!motor.initialized)
        {
                k_msleep(100);
        }

        while (1)
        {
                send_global_enable_frame(dev_can);
                motor.apply_supply_current_limit(&motor, 50.0);
                motor.set(&motor, 1);
                // actuator.set(&actuator, 1);
                k_msleep(50);
        }
        return 0;
}

K_THREAD_DEFINE(control_thread_id, STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

#define UART_RX_BUFFER_SIZE 64

static uint8_t rx_buf[UART_RX_BUFFER_SIZE] = {0}; // A buffer to store incoming UART data

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
        switch (evt->type)
        {
        case UART_RX_RDY:
                LOG_HEXDUMP_INF(evt->data.rx.buf + evt->data.rx.offset,
                                evt->data.rx.len, "RX_RDY");
                break;
        case UART_RX_DISABLED:
                uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
                break;
        default:
                break;
        }
}

int main(void)
{
        k_msleep(10);

        configure_can_device(dev_can);

        if (!device_is_ready(uart))
                return 0;

        const struct uart_config uart_cfg = {
            .baudrate = 115200,
            .parity = UART_CFG_PARITY_NONE,
            .stop_bits = UART_CFG_STOP_BITS_1,
            .data_bits = UART_CFG_DATA_BITS_8,
            .flow_ctrl = UART_CFG_FLOW_CTRL_NONE};

        int err = uart_configure(uart, &uart_cfg);

        if (err == -ENOSYS)
                return -ENOSYS;

        err = uart_callback_set(uart, uart_cb, NULL);
        if (err)
                return err;

        uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);

        talon_fx_init(&motor, dev_can, 27);
        talon_srx_init(&actuator, dev_can, 0, false);

        LOG_INF("Devices initialized. Entering main loop.");

        while (1)
        {
                LOG_INF("Heartbeat");
                k_msleep(1000);
        }

        return 0;
}
