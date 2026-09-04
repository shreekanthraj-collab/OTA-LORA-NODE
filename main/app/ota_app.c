/**
 * @file ota_app.c
 * @brief ORB DRIVE OTA module application controller.
 */

#include "ota_app.h"

#include "esp_log.h"

#include "buttons.h"
#include "buzzer.h"
#include "oled.h"
#include "ota_manager.h"
#include "wifi_manager.h"
#include "firmware_storage.h"

static const char *TAG = "OTA_APP";

static void ota_app_show_failure(
    const char *reason)
{
    ESP_LOGE(
        TAG,
        "Application failure: %s",
        reason
    );

    oled_show_failure(reason);
    buzzer_error();
}

static void ota_app_upload_progress(
    size_t bytes_sent,
    size_t total_bytes,
    void *context)
{
    (void)context;

    if (total_bytes == 0U)
    {
        return;
    }

    unsigned int percent =
        (unsigned int)(
            (bytes_sent * 100U) / total_bytes
        );

    if (percent > 100U)
    {
        percent = 100U;
    }

    ESP_LOGI(
        TAG,
        "OTA upload progress: %u%% (%u/%u)",
        percent,
        (unsigned int)bytes_sent,
        (unsigned int)total_bytes
    );
}

void ota_app_start(void)
{
    ESP_LOGI(
        TAG,
        "ORB DRIVE OTA application starting"
    );

    /*
     * Hardware initialization.
     */
    oled_init();
    buttons_init();
    buzzer_init();

    oled_clear();
    oled_show_startup();

    /*
     * OTA transport initialization.
     */
    ota_manager_init();

    /*
     * Wi-Fi station initialization.
     */
    wifi_manager_init();

    /*
     * Search for the frozen Node AP.
     */
    if (!wifi_manager_find_node())
    {
        ota_app_show_failure(
            "NODE NOT FOUND"
        );

        return;
    }

    oled_show_node_found();

    /*
     * Connect to the Node.
     */
    if (!wifi_manager_connect())
    {
        ota_app_show_failure(
            "WIFI CONNECT FAILED"
        );

        return;
    }

    /*
     * Confirm the Node is reachable.
     */
    if (!wifi_manager_node_reachable())
    {
        ota_app_show_failure(
            "NODE NOT REACHABLE"
        );

        return;
    }

    oled_show_node_connected();

    /*
     * Firmware storage initialization.
     */
    if (!firmware_storage_init())
    {
        ota_app_show_failure(
            "STORAGE INIT FAILED"
        );

        return;
    }

    /*
     * Check whether the embedded Node firmware
     * is available.
     */
    if (!firmware_storage_available())
    {
        ota_app_show_failure(
            "FIRMWARE NOT FOUND"
        );

        return;
    }

    /*
     * Obtain the embedded Node firmware image.
     */
    firmware_image_t image;

    if (!firmware_storage_get_image(&image))
    {
        ota_app_show_failure(
            "FIRMWARE LOAD FAILED"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "Node firmware image loaded: %u bytes",
        (unsigned int)image.size
    );

    /*
     * Upload the firmware to the Node.
     */
    if (!ota_manager_upload(
            image.data,
            image.size,
            ota_app_upload_progress,
            NULL))
    {
        firmware_storage_release(&image);

        ota_app_show_failure(
            "OTA UPLOAD FAILED"
        );

        return;
    }

    /*
     * Embedded image does not require dynamic
     * memory cleanup, but release the descriptor
     * through the storage interface.
     */
    firmware_storage_release(&image);

    buzzer_success();

    ESP_LOGI(
        TAG,
        "Node firmware upload completed"
    );

    ESP_LOGI(
        TAG,
        "OTA module ready"
    );
}