#ifndef WIFI_H_
#define WIFI_H_

#include <stdint.h>

#define WIFI_THREAD_PRIORITY 7

// All WiFi configurations
#define SSID "Team_02"
#define PSK "marsuva!"
#define SERVER_IP "192.168.0.104"
#define SERVER_PORT 8080

int configure_wifi();
int open_udp_socket(const char *server_ip, uint16_t server_port);
int close_udp_socket(int sock);

#endif /* WIFI_H_ */
