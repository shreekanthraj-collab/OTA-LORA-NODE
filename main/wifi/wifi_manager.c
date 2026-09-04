/**
 * @file wifi_manager.c
 * @brief ORB DRIVE OTA module Wi-Fi station manager.
 *
 * The OTA module joins the Node-created Wi-Fi AP.
 *
 * Frozen Node interface:
 *
 * SSID       : lora-node
 * PASSWORD   : node@1234
 * CHANNEL    : 1
 * MAX CLIENT : 1
 * NODE IP    : 192.168.4.1
 */

#include "wifi_manager.h"

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_err.h"

#include "ota_hw_config.h"

static const char *TAG = "WIFI_MANAGER";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define WIFI_MAX_RETRY     10

static EventGroupHandle_t s_wifi_event_group;

static esp_netif_t *s_sta_netif = NULL;

static int s_retry_count = 0;

static bool s_initialized = false;

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
        if (event_id == WIFI_EVENT_STA_START)
        {
            ESP_LOGI(TAG, "Wi-Fi STA started");

            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            if (s_retry_count < WIFI_MAX_RETRY)
            {
                s_retry_count++;

                ESP_LOGW(
                    TAG,
                    "Node AP connection retry %d/%d",
                    s_retry_count,
                    WIFI_MAX_RETRY
                );

                esp_wifi_connect();
            }
            else
            {
                ESP_LOGE(
                    TAG,
                    "Failed to connect to Node AP"
                );

                xEventGroupSetBits(
                    s_wifi_event_group,
                    WIFI_FAIL_BIT
                );
            }
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event =
                (ip_event_got_ip_t *)event_data;

            ESP_LOGI(
                TAG,
                "Node connection established"
            );

            ESP_LOGI(
                TAG,
                "Assigned IP: " IPSTR,
                IP2STR(&event->ip_info.ip)
            );

            s_retry_count = 0;

            xEventGroupSetBits(
                s_wifi_event_group,
                WIFI_CONNECTED_BIT
            );
        }
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

    s_sta_netif = esp_netif_create_default_wifi_sta();

    if (s_sta_netif == NULL)
    {
        ESP_LOGE(
            TAG,
            "Failed to create Wi-Fi STA network interface"
        );

        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );

    s_wifi_event_group =
        xEventGroupCreate();

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

    s_initialized = true;

    ESP_LOGI(
        TAG,
        "Wi-Fi manager initialized"
    );
}

bool wifi_manager_scan_node(void)
{
    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi manager not initialized"
        );

        return false;
    }

    ESP_LOGI(
        TAG,
        "Scanning for Node AP: %s",
        OTA_WIFI_AP_SSID
    );

    wifi_scan_config_t scan_config = {
        .ssid = (uint8_t *)OTA_WIFI_AP_SSID,
        .bssid = NULL,
        .channel = OTA_WIFI_CHANNEL,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
        .home_chan_dwell_time = 30,
    };

    esp_err_t err =
        esp_wifi_set_mode(WIFI_MODE_STA);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to set STA mode: %s",
            esp_err_to_name(err)
        );

        return false;
    }

    err =
        esp_wifi_start();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(
            TAG,
            "Failed to start Wi-Fi: %s",
            esp_err_to_name(err)
        );

        return false;
    }

    err =
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

        return false;
    }

    uint16_t ap_count = 0;

    err =
        esp_wifi_scan_get_ap_num(
            &ap_count
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to get AP count"
        );

        return false;
    }

    ESP_LOGI(
        TAG,
        "Wi-Fi scan found %u AP(s)",
        ap_count
    );

    if (ap_count == 0)
    {
        ESP_LOGW(
            TAG,
            "Node AP not found"
        );

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
            "Failed to read scan results"
        );

        free(records);

        return false;
    }

    bool node_found = false;

    for (uint16_t i = 0; i < record_count; i++)
    {
        if (strcmp(
                (char *)records[i].ssid,
                OTA_WIFI_AP_SSID
            ) == 0)
        {
            ESP_LOGI(
                TAG,
                "NODE FOUND: %s",
                OTA_WIFI_AP_SSID
            );

            ESP_LOGI(
                TAG,
                "Channel: %d",
                records[i].primary
            );

            ESP_LOGI(
                TAG,
                "RSSI: %d dBm",
                records[i].rssi
            );

            node_found = true;

            break;
        }
    }

    free(records);

    return node_found;
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

    wifi_config_t wifi_config = {0};

    strncpy(
        (char *)wifi_config.sta.ssid,
        OTA_WIFI_AP_SSID,
        sizeof(wifi_config.sta.ssid) - 1
    );

    strncpy(
        (char *)wifi_config.sta.password,
        OTA_WIFI_AP_PASSWORD,
        sizeof(wifi_config.sta.password) - 1
    );

    wifi_config.sta.channel = OTA_WIFI_CHANNEL;

    wifi_config.sta.scan_method =
        WIFI_FAST_SCAN;

    wifi_config.sta.sort_method =
        WIFI_CONNECT_AP_BY_SIGNAL;

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    s_retry_count = 0;

    xEventGroupClearBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT |
        WIFI_FAIL_BIT
    );

    ESP_LOGI(
        TAG,
        "Connecting to Node AP: %s",
        OTA_WIFI_AP_SSID
    );

    esp_err_t err =
        esp_wifi_start();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi start failed: %s",
            esp_err_to_name(err)
        );

        return false;
    }

    err =
        esp_wifi_connect();

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi connect failed: %s",
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
            pdMS_TO_TICKS(15000)
        );

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(
            TAG,
            "NODE CONNECTED"
        );

        return true;
    }

    ESP_LOGE(
        TAG,
        "NODE CONNECTION FAILED"
    );

    return false;
}

bool wifi_manager_is_connected(void)
{
    if (!s_initialized)
    {
        return false;
    }

    EventBits_t bits =
        xEventGroupGetBits(
            s_wifi_event_group
        );

    return (bits & WIFI_CONNECTED_BIT) != 0;
}
