/**
 * @file ota_manager.c
 * @brief ORB DRIVE OTA module firmware upload manager.
 *
 * Frozen Node OTA interface:
 *
 * Node IP : 192.168.4.1
 * Endpoint: POST /ota
 *
 * Firmware is transferred over the Node Wi-Fi AP.
 */

#include "ota_manager.h"

#include "esp_log.h"

#include "ota_hw_config.h"

static const char *TAG = "OTA_MANAGER";

void ota_manager_init(void)
{
    ESP_LOGI(TAG, "OTA manager initialized");

    ESP_LOGI(
        TAG,
        "Node OTA endpoint: http://%s%s",
        OTA_NODE_IP,
        OTA_NODE_OTA_PATH
    );
}

bool ota_manager_start_upload(
    const char *firmware_path,
    size_t firmware_size)
{
    if (firmware_path == NULL)
    {
        ESP_LOGE(TAG, "Firmware path is NULL");
        return false;
    }

    if (firmware_size == 0U)
    {
        ESP_LOGE(TAG, "Firmware size is zero");
        return false;
    }

    /*
     * HTTP POST implementation will be added after
     * Wi-Fi connectivity and firmware source are verified.
     */

    ESP_LOGI(
        TAG,
        "Firmware selected: %s",
        firmware_path
    );

    ESP_LOGI(
        TAG,
        "Firmware size: %u bytes",
        (unsigned int)firmware_size
    );

    return false;
}
