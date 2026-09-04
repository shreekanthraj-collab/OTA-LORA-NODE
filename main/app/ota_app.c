/**
 * @file ota_app.c
 * @brief ORB DRIVE OTA module application controller.
 */

#include "ota_app.h"

#include "esp_log.h"

#include "buttons.h"
#include "buzzer.h"
#include "firmware_storage.h"
#include "oled.h"
#include "ota_manager.h"
#include "wifi_manager.h"

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
     * Check whether a firmware image is available.
     */
    if (!firmware_storage_available())
    {
        ota_app_show_failure(
            "FIRMWARE NOT FOUND"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "Firmware image available"
    );

    ESP_LOGI(
        TAG,
        "OTA module ready"
    );

    /*
     * Firmware acquisition and OTA upload
     * will be integrated after the firmware
     * source is frozen.
     */
}