/**
 * @file oled.c
 * @brief ORB DRIVE OTA module OLED display driver.
 *
 * NOTE:
 * This is the initial display abstraction.
 * The physical OLED controller and I2C configuration
 * will be implemented after hardware verification.
 */

#include "oled.h"

#include "esp_log.h"

static const char *TAG = "OLED";

void oled_init(void)
{
    ESP_LOGI(TAG, "OLED initialization");
}

void oled_clear(void)
{
    ESP_LOGI(TAG, "OLED clear");
}

void oled_show_startup(void)
{
    ESP_LOGI(TAG, "ORB DRIVE OTA");
    ESP_LOGI(TAG, "Connecting...");
}

void oled_show_node_found(void)
{
    ESP_LOGI(TAG, "NODE FOUND");
    ESP_LOGI(TAG, "lora-node");
}

void oled_show_node_connected(void)
{
    ESP_LOGI(TAG, "NODE CONNECTED");
    ESP_LOGI(TAG, "192.168.4.1");
}

void oled_show_upload_progress(
    size_t bytes_sent,
    size_t total_bytes)
{
    if (total_bytes == 0U)
    {
        return;
    }

    uint32_t percentage =
        (uint32_t)(((uint64_t)bytes_sent * 100U) /
                   total_bytes);

    if (percentage > 100U)
    {
        percentage = 100U;
    }

    ESP_LOGI(
        TAG,
        "OTA UPDATE: %lu%% (%u / %u bytes)",
        (unsigned long)percentage,
        (unsigned int)bytes_sent,
        (unsigned int)total_bytes
    );
}

void oled_show_upload_complete(void)
{
    ESP_LOGI(TAG, "OTA UPDATE");
    ESP_LOGI(TAG, "UPLOAD COMPLETE");
    ESP_LOGI(TAG, "100%%");
}

void oled_show_failure(const char *reason)
{
    ESP_LOGE(TAG, "OTA UPDATE");
    ESP_LOGE(
        TAG,
        "FAILED: %s",
        reason != NULL ? reason : "Unknown error"
    );
}
