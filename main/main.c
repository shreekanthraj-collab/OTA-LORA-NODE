/**
 * @file main.c
 * @brief ORB DRIVE OTA module application entry point.
 */

#include "app/ota_app.h"

#include "hardware/buzzer.h"
#include "hardware/buttons.h"
#include "hardware/oled.h"

#include "wifi/wifi_manager.h"

#include "ota/ota_manager.h"

#include "storage/firmware_storage.h"

void app_main(void)
{
    /*
     * Hardware initialization
     */
    oled_init();

    buzzer_init();

    buttons_init();

    /*
     * System managers
     */
    wifi_manager_init();

    ota_manager_init();

    firmware_storage_init();

    /*
     * Application startup
     */
    ota_app_start();

    oled_show_startup();
}
