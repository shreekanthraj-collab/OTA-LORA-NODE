/**
 * @file ota_app.c
 * @brief ORB DRIVE OTA module application controller.
 */

#include "ota_app.h"

#include "esp_log.h"

static const char *TAG = "OTA_APP";

void ota_app_start(void)
{
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "       ORB DRIVE OTA");
    ESP_LOGI(TAG, "       OTA MODULE");
    ESP_LOGI(TAG, "================================");

    ESP_LOGI(TAG, "Application started");
}
