/**
 * @file firmware_storage.c
 * @brief ORB DRIVE OTA module firmware storage manager.
 */

#include "firmware_storage.h"

#include "esp_log.h"

static const char *TAG = "FW_STORAGE";

static bool s_initialized = false;

bool firmware_storage_init(void)
{
    if (s_initialized)
    {
        return true;
    }

    s_initialized = true;

    ESP_LOGI(
        TAG,
        "Firmware storage manager initialized"
    );

    return true;
}

bool firmware_storage_available(void)
{
    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "Firmware storage not initialized"
        );

        return false;
    }

    /*
     * Firmware source is not yet frozen.
     *
     * Do not report a firmware image as available
     * until the actual storage/source implementation
     * is defined.
     */
    return false;
}

bool firmware_storage_get_image(
    firmware_image_t *image)
{
    if (image == NULL)
    {
        ESP_LOGE(
            TAG,
            "Invalid firmware image descriptor"
        );

        return false;
    }

    image->data = NULL;
    image->size = 0U;

    if (!s_initialized)
    {
        ESP_LOGE(
            TAG,
            "Firmware storage not initialized"
        );

        return false;
    }

    if (!firmware_storage_available())
    {
        ESP_LOGW(
            TAG,
            "No firmware image available"
        );

        return false;
    }

    return true;
}

void firmware_storage_release(
    firmware_image_t *image)
{
    if (image == NULL)
    {
        return;
    }

    /*
     * Current implementation does not allocate
     * or own any firmware buffer.
     */
    image->data = NULL;
    image->size = 0U;
}
