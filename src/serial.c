#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>
#include "serial.h"

LOG_MODULE_REGISTER(serial);

static size_t rx_count = 0;
static uint8_t uart_buf1[SIZEOF_UART_RX_BUFFER] = {0};
static uint8_t uart_buf2[SIZEOF_UART_RX_BUFFER] = {0};
static uint8_t rx_accumulator[SIZEOF_SERIAL_PACKET] = {0};
static uint8_t *rx_buf = uart_buf1;

void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    struct k_msgq *serial_msgq = (struct k_msgq *)user_data;

    switch (evt->type)
    {
    case UART_RX_RDY:
        for (size_t i = 0; i < evt->data.rx.len; i++)
        {
            // Wait for header byte
            if (rx_count == 0 && evt->data.rx.buf[evt->data.rx.offset + i] != SERIAL_START_BYTE)
                continue;

            rx_accumulator[rx_count++] = evt->data.rx.buf[evt->data.rx.offset + i];
            if (rx_count == SIZEOF_SERIAL_PACKET)
            {
                serial_packet_t tmp = {.invalid = 0};
                memcpy(&tmp.start_byte, rx_accumulator, SIZEOF_SERIAL_PACKET);
                rx_count = 0;
                k_msgq_put(serial_msgq, &tmp, K_NO_WAIT);
            }
        }
        break;
    case UART_RX_BUF_REQUEST:
        // Switch to the other buffer
        rx_buf = (rx_buf == uart_buf1) ? uart_buf2 : uart_buf1;
        // Provide a new buffer for continuous reception
        uart_rx_buf_rsp(dev, rx_buf, SIZEOF_UART_RX_BUFFER);
        break;
    case UART_RX_STOPPED:
    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, SIZEOF_UART_RX_BUFFER, TIMEOUT_UART_RX_MS);
        break;
    default:
        break;
    }
}

void configure_uart_device(const struct device *uart, struct k_msgq *serial_msgq)
{
    if (!device_is_ready(uart))
    {
        LOG_ERR("UART device not ready.");
        return;
    }

    const struct uart_config uart_cfg = {
        .baudrate = 115200,
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
        .data_bits = UART_CFG_DATA_BITS_8,
        .flow_ctrl = UART_CFG_FLOW_CTRL_RTS_CTS};

    int err = uart_configure(uart, &uart_cfg);

    if (err == -ENOSYS)
    {
        LOG_ERR("UART configuration not supported.");
        return;
    }

    err = uart_callback_set(uart, uart_cb, serial_msgq);

    if (err)
    {
        LOG_ERR("Failed to set UART callback (err %d)", err);
        return;
    }

    uart_rx_enable(uart, rx_buf, SIZEOF_UART_RX_BUFFER, TIMEOUT_UART_RX_MS);
    LOG_INF("UART device configured.");
}
