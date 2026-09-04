/**
 * @file buttons.c
 * @brief ORB DRIVE OTA module button driver.
 */

#include "buttons.h"

#include "driver/gpio.h"

#include "ota_hw_config.h"

void buttons_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask =
            (1ULL << OTA_GPIO_BTN_SELECT) |
            (1ULL << OTA_GPIO_BTN_UP) |
            (1ULL << OTA_GPIO_BTN_DOWN),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&config);
}

bool button_select_pressed(void)
{
    return gpio_get_level(OTA_GPIO_BTN_SELECT) == 0;
}

bool button_up_pressed(void)
{
    return gpio_get_level(OTA_GPIO_BTN_UP) == 0;
}

bool button_down_pressed(void)
{
    return gpio_get_level(OTA_GPIO_BTN_DOWN) == 0;
}
