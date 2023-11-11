#include <FreeRTOS.h>
#include <lwip/tcp.h>
#include <pico/cyw43_arch.h>
#include <sys/cdefs.h>
#include <task.h>

#include "kernel.h"
#include "logging.h"

static char mac[13];
static char default_headers[256];
static uint8_t default_headers_size;

static char content_length_header[64];
static char http_response_first_line[64];

static char strIpAddress[16] = {0};
static ip_addr_t api_addr;

extern uint32_t blinkDelay;

static void httpSend(struct tcp_pcb *tpcb, struct HttpRequest *request) {
    size_t payload_size;

    tcp_write(tpcb, request->request_line, strlen(request->request_line), TCP_WRITE_FLAG_MORE);
    if (request->payload != NULL) {
        payload_size = strlen(request->payload);
        snprintf(content_length_header, 64, "Content-Length: %d\r\n", payload_size);
        tcp_write(tpcb, content_length_header, strlen(content_length_header), TCP_WRITE_FLAG_MORE);
        tcp_write(tpcb, default_headers, default_headers_size, TCP_WRITE_FLAG_MORE);
        tcp_write(tpcb, request->payload, payload_size, 0);
    } else {
        tcp_write(tpcb, default_headers, default_headers_size, 0);
    }
    tcp_output(tpcb);
}

static void httpErrHandler(void *arg, err_t err) { LOG_ERROR("tcp error %d", err); }

static err_t httpRecvHandler(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    int index = 0;
    char *payload = p->payload;
    while (*payload != '\r') {
        http_response_first_line[index] = *payload;
        payload++;
        index++;
    }
    http_response_first_line[index] = '\0';
    LOG_INFO("%s", http_response_first_line);

    pbuf_free(p);
    return tcp_close(tpcb);
}

static err_t httpConnectHandler(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (arg != NULL) {
        struct HttpRequest *request = (struct HttpRequest *)arg;
        httpSend(tpcb, request);
    } else {
        LOG_ERROR("no args for tcp connection, aborting");
        tcp_close(tpcb);
    }

    return 0;
}

void vNetifStatusCallback(struct netif *netif) {
    ip4addr_ntoa_r(&netif->ip_addr, strIpAddress, 16);
    LOG_INFO("ip addr is %s", strIpAddress);
    LOG_INFO("using api server %s:%d", API_IPV4, API_PORT);
}

void vWifiTask(__unused void *pvParameters) {
    IP4_ADDR(&api_addr, API_IPV4_B1, API_IPV4_B2, API_IPV4_B3, API_IPV4_B4);
    sprintf(
        mac, "%02x%02x%02x%02x%02x%02x", cyw43_state.mac[0], cyw43_state.mac[1], cyw43_state.mac[2],
        cyw43_state.mac[3], cyw43_state.mac[4], cyw43_state.mac[5]
    );
    snprintf(
        default_headers, 256,
        "Host: %s:%d\r\n"
        "Accept: application/json\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: Basic %s:%s\r\n"
        "Connection: close\r\n\r\n",
        API_IPV4, API_PORT, mac, mac
    );
    default_headers_size = strlen(default_headers);
    while (1) {
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        LOG_INFO("connecting to %s", WIFI_SSID);
        LOG_DEBUG("using PSK %s and timeout %d ms", WIFI_PSK, WIFI_TIMEOUT);
        if (cyw43_arch_wifi_connect_timeout_ms(
                WIFI_SSID, WIFI_PSK, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT
            )) {
            LOG_WARNING("failed to connect");
            blinkDelay = 300;
        } else {
            LOG_INFO("connected");
            blinkDelay = 1980;
        }
    }
}

void vHttpRequestTask(__unused void *pvParameters) {
    while (1) {
        struct HttpRequest *request = (struct HttpRequest *)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        LOG_INFO("%s\x1B[A", request->request_line);
        struct tcp_pcb *connection = tcp_new();
        tcp_arg(connection, request);
        tcp_err(connection, httpErrHandler);
        tcp_recv(connection, httpRecvHandler);

        if (tcp_connect(connection, &api_addr, API_PORT, httpConnectHandler) != ERR_OK) {
            LOG_ERROR("tcp_connect error");
        }
    }
}
