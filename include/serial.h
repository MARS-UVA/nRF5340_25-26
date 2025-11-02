#include <stdint.h>

#ifndef SERIAL_H_
#define SERIAL_H_

typedef struct serial_packet
{
    int8_t invalid;
    uint8_t header;
    uint8_t front_left_wheel;
    uint8_t back_left_wheel;
    uint8_t front_right_wheel;
    uint8_t back_right_wheel;
    uint8_t drum;
    uint8_t actuator;
} serial_packet_t;

#endif /* SERIAL_H_ */
