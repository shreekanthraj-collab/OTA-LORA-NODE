/**
 * @file buzzer.c
 * @brief ORB DRIVE OTA module buzzer driver.
 */

#include "buzzer.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ota_hw_config.h"

void buzzer_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << OTA_GPIO_BUZZER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&config);

    gpio_set_level(OTA_GPIO_BUZZER, 0);
}

void buzzer_short_beep(void)
{
    gpio_set_level(OTA_GPIO_BUZZER, 1);

    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(OTA_GPIO_BUZZER, 0);
}

void buzzer_long_beep(void)
{
    gpio_set_level(OTA_GPIO_BUZZER, 1);

    vTaskDelay(pdMS_TO_TICKS(500));

    gpio_set_level(OTA_GPIO_BUZZER, 0);
}

void buzzer_success(void)
{
    buzzer_short_beep();

    vTaskDelay(pdMS_TO_TICKS(100));

    buzzer_short_beep();
}

void buzzer_error(void)
{
    buzzer_long_beep();
}
