#include <stdint.h>

#ifndef SERIAL_H_
#define SERIAL_H_

#define SIZEOF_UART_RX_BUFFER MAX(SIZEOF_SERIAL_PACKET * 2, 32)
#define TIMEOUT_UART_RX_MS 10

// Size of serial packet excluding the invalid byte
#define SIZEOF_SERIAL_PACKET sizeof(serial_packet_t) - sizeof(uint8_t)
#define SERIAL_START_BYTE 0xFF

typedef struct __attribute__((packed))
{
    int8_t invalid;
    uint8_t start_byte;
    uint8_t header_byte;
    uint8_t front_left_wheel;
    uint8_t back_left_wheel;
    uint8_t front_right_wheel;
    uint8_t back_right_wheel;
    uint8_t drum;
    uint8_t actuator;
} serial_packet_t;

void configure_uart_device(const struct device *uart, struct k_msgq *serial_msgq);

#endif /* SERIAL_H_ */
