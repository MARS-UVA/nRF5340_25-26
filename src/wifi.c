#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include "wifi.h"

LOG_MODULE_REGISTER(wifi, LOG_LEVEL_DBG);

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static struct net_mgmt_event_callback wifi_cb;

void wifi_event_handler(struct net_mgmt_event_callback *cb,
                        uint32_t mgmt_event,
                        struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD)
    {
        LOG_INF("IPv4 Address obtained");
        k_sem_give(&ipv4_address_obtained);
    }
}

void connect_to_wifi(struct net_if *iface)
{
    struct wifi_connect_req_params cnx_params = {0};

    cnx_params.ssid = SSID;
    cnx_params.ssid_length = strlen(SSID);
    cnx_params.psk = PSK;
    cnx_params.psk_length = strlen(PSK);
    cnx_params.security = WIFI_SECURITY_TYPE_PSK;
    cnx_params.channel = WIFI_CHANNEL_ANY;
    cnx_params.band = WIFI_FREQ_BAND_2_4_GHZ;
    cnx_params.mfp = WIFI_MFP_OPTIONAL;

    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params, sizeof(cnx_params));
    if (ret != 0)
    {
        LOG_ERR("WiFi Connection Request Failed: %d", ret);
    }
    else
    {
        LOG_INF("WiFi Connection Request Sent");
    }
}

void wifi_status(void)
{
    struct net_if *iface = net_if_get_default();

    struct wifi_iface_status status = {0};

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)))
    {
        LOG_ERR("WiFi Status Request Failed");
    }

    while (status.state < WIFI_STATE_ASSOCIATED)
    {
        LOG_INF("Current WiFi State: %d", status.state);
        k_msleep(1000);

        if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)))
        {
            LOG_ERR("WiFi Status Request Failed");
        }
    }

    if (status.state >= WIFI_STATE_ASSOCIATED)
    {
        LOG_INF("SSID: %-32s", status.ssid);
        LOG_INF("Band: %s", wifi_band_txt(status.band));
        LOG_INF("Channel: %d", status.channel);
        LOG_INF("Security: %s", wifi_security_txt(status.security));
        LOG_INF("RSSI: %d", status.rssi);
        k_sem_give(&wifi_connected);
    }
}

int open_udp_socket(const char *server_ip, uint16_t server_port)
{
    int sock = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        LOG_ERR("Failed to create socket: %d", sock);
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    zsock_inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Connect the socket
    int ret = zsock_connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        LOG_ERR("Socket connect failed: %d", ret);
        zsock_close(sock);
        return -1;
    }

    return sock;
}

int close_udp_socket(int sock)
{
    return zsock_close(sock);
}

int configure_wifi(void)
{
    net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&wifi_cb);

    struct net_if *iface;
    do
    {
        iface = net_if_get_default();
        k_msleep(50);
    } while (!iface || !net_if_flag_is_set(iface, NET_IF_UP));

    connect_to_wifi(iface);

    LOG_INF("Connecting to WiFi network...");

    wifi_status();

    k_sem_take(&wifi_connected, K_FOREVER);
    k_sem_take(&ipv4_address_obtained, K_FOREVER);

    LOG_INF("WiFi connected and IPv4 address obtained.");

    return 0;
}
