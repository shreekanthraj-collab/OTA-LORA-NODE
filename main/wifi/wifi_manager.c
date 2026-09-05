/**
 * @file wifi_manager.c
 * @brief ORB DRIVE OTA module Wi-Fi station manager.
 */

#include "wifi_manager.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "lwip/sockets.h"

#include "ota_hw_config.h"

static const char *TAG = "WIFI_MANAGER";

#define WIFI_CONNECTED_BIT    BIT0
#define WIFI_FAIL_BIT         BIT1

#define WIFI_MAX_RETRY        5U
#define WIFI_SCAN_TIMEOUT_MS  5000

static EventGroupHandle_t s_wifi_event_group = NULL;

static esp_netif_t *s_sta_netif = NULL;

static uint8_t s_retry_count = 0U;

static bool s_initialized = false;
static bool s_node_found = false;
static bool s_connected = false;

static char s_local_ip[16] = "0.0.0.0";

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {            case WIFI_EVENT_STA_START:
                ESP_LOGI(
                    TAG,
                    "Wi-Fi station started"
                );

                /* Connection is started explicitly by wifi_manager_connect(). */
                break;
break;

            case WIFI_EVENT_STA_DISCONNECTED:
                s_connected = false;

                if (s_retry_count < WIFI_MAX_RETRY)
                {
                    s_retry_count++;

                    ESP_LOGW(
                        TAG,
                        "Wi-Fi disconnected, retry %u/%u",
                        (unsigned int)s_retry_count,
                        (unsigned int)WIFI_MAX_RETRY
                    );

                    esp_wifi_connect();
                }
                else
                {
                    xEventGroupSetBits(
                        s_wifi_event_group,
                        WIFI_FAIL_BIT
                    );
                }

                break;

            default:
                break;
        }
    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        snprintf(
            s_local_ip,
            sizeof(s_local_ip),
            IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        s_retry_count = 0U;
        s_connected = true;

        ESP_LOGI(
            TAG,
            "Connected, local IP: %s",
            s_local_ip
        );

        xEventGroupSetBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}

void wifi_manager_init(void)
{
    if (s_initialized)
    {
        return;
    }

    ESP_ERROR_CHECK(
        esp_netif_init()
    );

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );

    s_sta_netif =
        esp_netif_create_default_wifi_sta();

    if (s_sta_netif == NULL)
    {
        ESP_LOGE(
            TAG,
            "Failed to create STA network interface"
        );

        return;
    }

    wifi_init_config_t wifi_config =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&wifi_config)
    );

    s_wifi_event_group =
        xEventGroupCreate();

    if (s_wifi_event_group == NULL)
    {
        ESP_LOGE(
            TAG,
            "Failed to create Wi-Fi event group"
        );

        return;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );

    s_initialized = true;

    ESP_LOGI(
        TAG,
        "Wi-Fi manager initialized"
    );
}

bool wifi_manager_find_node(void)
{
    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi manager not initialized"
        );

        return false;
    }

    wifi_scan_config_t scan_config = {
        .ssid = (uint8_t *)OTA_WIFI_AP_SSID,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    ESP_LOGI(
        TAG,
        "Scanning for Node AP: %s",
        OTA_WIFI_AP_SSID
    );

    esp_err_t err =
        esp_wifi_scan_start(
            &scan_config,
            true
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi scan failed: %s",
            esp_err_to_name(err)
        );

        s_node_found = false;

        return false;
    }

    uint16_t ap_count = 0U;

    err =
        esp_wifi_scan_get_ap_num(
            &ap_count
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to get scan count: %s",
            esp_err_to_name(err)
        );

        s_node_found = false;

        return false;
    }

    if (ap_count == 0U)
    {
        ESP_LOGW(
            TAG,
            "No Wi-Fi networks found"
        );

        s_node_found = false;

        return false;
    }

    wifi_ap_record_t *records =
        calloc(
            ap_count,
            sizeof(wifi_ap_record_t)
        );

    if (records == NULL)
    {
        ESP_LOGE(
            TAG,
            "Unable to allocate scan records"
        );

        s_node_found = false;

        return false;
    }

    uint16_t record_count = ap_count;

    err =
        esp_wifi_scan_get_ap_records(
            &record_count,
            records
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to read scan records: %s",
            esp_err_to_name(err)
        );

        free(records);

        s_node_found = false;

        return false;
    }

    s_node_found = false;

    for (uint16_t i = 0U; i < record_count; i++)
    {
        if (strcmp(
                (const char *)records[i].ssid,
                OTA_WIFI_AP_SSID) == 0)
        {
            ESP_LOGI(
                TAG,
                "Node AP found: %s",
                records[i].ssid
            );

            ESP_LOGI(
                TAG,
                "Channel: %u RSSI: %d",
                (unsigned int)records[i].primary,
                (int)records[i].rssi
            );

            if (records[i].primary == OTA_WIFI_CHANNEL)
            {
                s_node_found = true;

                ESP_LOGI(
                    TAG,
                    "Node AP channel verified: %u",
                    (unsigned int)OTA_WIFI_CHANNEL
                );

                break;
            }

            ESP_LOGW(
                TAG,
                "SSID found but channel mismatch"
            );
        }
    }

    free(records);

    return s_node_found;
}

bool wifi_manager_connect(void)
{
    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi manager not initialized"
        );

        return false;
    }

    if (!s_node_found)
    {
        ESP_LOGW(
            TAG,
            "Node AP has not been found"
        );

        return false;
    }

    wifi_config_t config = {0};

    strncpy(
        (char *)config.sta.ssid,
        OTA_WIFI_AP_SSID,
        sizeof(config.sta.ssid) - 1U
    );

    strncpy(
        (char *)config.sta.password,
        OTA_WIFI_AP_PASSWORD,
        sizeof(config.sta.password) - 1U
    );

    config.sta.scan_method =
        WIFI_ALL_CHANNEL_SCAN;

    config.sta.sort_method =
        WIFI_CONNECT_AP_BY_SIGNAL;

    s_retry_count = 0U;
    s_connected = false;

    xEventGroupClearBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT |
        WIFI_FAIL_BIT
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &config
        )
    );

    ESP_LOGI(
        TAG,
        "Connecting to Node AP: %s",
        OTA_WIFI_AP_SSID
    );

    esp_err_t err =
        esp_wifi_connect();

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_wifi_connect failed: %s",
            esp_err_to_name(err)
        );

        return false;
    }

    EventBits_t bits =
        xEventGroupWaitBits(
            s_wifi_event_group,
            WIFI_CONNECTED_BIT |
            WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(
                WIFI_SCAN_TIMEOUT_MS
            )
        );

    if ((bits & WIFI_CONNECTED_BIT) != 0)
    {
        ESP_LOGI(
            TAG,
            "Node Wi-Fi connection established"
        );

        return true;
    }

    ESP_LOGE(
        TAG,
        "Node Wi-Fi connection failed"
    );

    return false;
}

bool wifi_manager_is_connected(void)
{
    return s_connected;
}

const char *wifi_manager_get_local_ip(void)
{
    return s_local_ip;
}

bool wifi_manager_node_reachable(void)
{
    if (!s_connected)
    {
        ESP_LOGW(
            TAG,
            "Node reachability check skipped: not connected"
        );

        return false;
    }

    struct sockaddr_in address;

    memset(
        &address,
        0,
        sizeof(address)
    );

    address.sin_family = AF_INET;
    address.sin_port = htons(80);

    if (inet_pton(
            AF_INET,
            OTA_NODE_IP,
            &address.sin_addr) != 1)
    {
        ESP_LOGE(
            TAG,
            "Invalid Node IP address"
        );

        return false;
    }

    int socket_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_IP
        );

    if (socket_fd < 0)
    {
        ESP_LOGE(
            TAG,
            "Unable to create TCP socket"
        );

        return false;
    }

    struct timeval timeout = {
        .tv_sec = 2,
        .tv_usec = 0
    };

    setsockopt(
        socket_fd,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout)
    );

    setsockopt(
        socket_fd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    );

    int result =
        connect(
            socket_fd,
            (struct sockaddr *)&address,
            sizeof(address)
        );

    close(socket_fd);

    if (result == 0)
    {
        ESP_LOGI(
            TAG,
            "Node reachable at %s",
            OTA_NODE_IP
        );

        return true;
    }

    ESP_LOGW(
        TAG,
        "Node TCP connection failed"
    );

    return false;
}


