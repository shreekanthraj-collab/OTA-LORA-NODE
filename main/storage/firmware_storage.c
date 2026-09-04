/**
 * @file firmware_storage.c
 * @brief ORB DRIVE OTA module firmware storage manager.
 */

#include "firmware_storage.h"

#include "esp_log.h"

static const char *TAG = "FW_STORAGE";

void firmware_storage_init(void)
{
    ESP_LOGI(TAG, "Firmware storage manager initialized");
}
