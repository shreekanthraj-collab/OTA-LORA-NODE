/**
 * @file ota_hw_config.h
 * @brief ORB DRIVE OTA module hardware and frozen interface configuration.
 */

#ifndef OTA_HW_CONFIG_H
#define OTA_HW_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * MCU / FLASH
 * ============================================================ */

#define OTA_MCU_ESP32S3

#define OTA_FLASH_SIZE_MB        16U

/* ============================================================
 * OLED
 *
 * Provisional GPIO assignment.
 * Hardware pinout must be verified against the actual PCB.
 * ============================================================ */

#define OTA_GPIO_OLED_SDA        8
#define OTA_GPIO_OLED_SCL        9

/* ============================================================
 * BUZZER
 * ============================================================ */

#define OTA_GPIO_BUZZER          21

/* ============================================================
 * BUTTONS
 * ============================================================ */

#define OTA_GPIO_BTN_SELECT      0
#define OTA_GPIO_BTN_UP          1
#define OTA_GPIO_BTN_DOWN        2

/* ============================================================
 * STATUS LED
 * ============================================================ */

#define OTA_GPIO_STATUS_LED      48

/* ============================================================
 * RESET
 *
 * Hardware reset button is connected to ESP32-S3 EN.
 * ============================================================ */

/* ============================================================
 * NODE WI-FI AP
 * ============================================================ */

#define OTA_WIFI_AP_SSID         "lora-node"

#define OTA_WIFI_AP_PASSWORD     "node@1234"

#define OTA_WIFI_CHANNEL         1

#define OTA_WIFI_MAX_CLIENTS     1

/* ============================================================
 * NODE OTA SERVER
 * ============================================================ */

#define OTA_NODE_IP              "192.168.4.1"

#define OTA_NODE_OTA_PATH        "/ota"

#ifdef __cplusplus
}
#endif

#endif /* OTA_HW_CONFIG_H */
