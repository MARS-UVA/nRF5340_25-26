#include "serial.h"

#ifndef CONTROL_H_
#define CONTROL_H_

#define CONTROL_TIMEOUT_MS 50

// Define CAN IDs of each motor/actuator
#define FRONT_LEFT_WHEEL_ID 36
#define BACK_LEFT_WHEEL_ID 37
#define FRONT_RIGHT_WHEEL_ID 38
#define BACK_RIGHT_WHEEL_ID 13
#define BUCKET_DRUM_ID 25
#define BUCKET_DRUM_LEFT_ID 60
#define LEFT_ACTUATOR_ID 0
#define RIGHT_ACTUATOR_ID 1

// Define PDP IDs of each motor
#define FRONT_LEFT_WHEEL_PDP_ID 12
#define BACK_LEFT_WHEEL_PDP_ID 13
#define FRONT_RIGHT_WHEEL_PDP_ID 3
#define BACK_RIGHT_WHEEL_PDP_ID 1
#define BUCKET_DRUM_PDP_ID 0
#define BUCKET_DRUM_LEFT_PDP_ID 2
#define LEFT_ACTUATOR_PDP_ID 15
#define RIGHT_ACTUATOR_PDP_ID 14

#define CONTROL_THREAD_PRIORITY 5
#define DIRECT_ACTUATOR_CONTROL 1

void direct_control(serial_packet_t *pkt);
void initialize_talons(const struct device *can_dev);
bool talons_initialized(void);

#endif /* CONTROL_H_ */
