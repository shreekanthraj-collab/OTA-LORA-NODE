/**
 * @file firmware_storage.c
 * @brief ORB DRIVE OTA module firmware storage manager.
 *
 * Hardware source is intentionally not selected yet.
 */

#include "firmware_storage.h"

#include "esp_log.h"

static const char *TAG = "FW_STORAGE";

static bool s_initialized = false;

firmware_storage_status_t firmware_storage_init(void)
{
    /*
     * The physical firmware source has not yet been frozen.
     *
     * This layer therefore performs no hardware access.
     */
    s_initialized = true;

    ESP_LOGI(
        TAG,
        "Firmware storage manager initialized"
    );

    return FIRMWARE_STORAGE_OK;
}

bool firmware_storage_is_available(void)
{
    /*
     * No physical firmware source is implemented yet.
     */
    return false;
}

firmware_storage_status_t firmware_storage_get_size(
    size_t *size)
{
    if (!s_initialized)
    {
        return FIRMWARE_STORAGE_NOT_INITIALIZED;
    }

    if (size == NULL)
    {
        return FIRMWARE_STORAGE_INVALID;
    }

    *size = 0U;

    return FIRMWARE_STORAGE_NOT_AVAILABLE;
}

firmware_storage_status_t firmware_storage_read(
    size_t offset,
    uint8_t *buffer,
    size_t length)
{
    (void)offset;

    if (!s_initialized)
    {
        return FIRMWARE_STORAGE_NOT_INITIALIZED;
    }

    if ((buffer == NULL) && (length > 0U))
    {
        return FIRMWARE_STORAGE_INVALID;
    }

    /*
     * No physical firmware source is implemented yet.
     */
    return FIRMWARE_STORAGE_NOT_AVAILABLE;
}
