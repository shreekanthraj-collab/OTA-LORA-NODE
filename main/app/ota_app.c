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

    ESP_LOGI(
        TAG,
        "OTA module ready"
    );

    /*
     * Firmware acquisition is intentionally not performed here.
     *
     * firmware_storage will provide the firmware image once
     * the actual firmware source is frozen.
     */
}
