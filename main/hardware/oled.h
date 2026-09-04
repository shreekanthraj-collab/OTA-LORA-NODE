/**
 * @file oled.h
 * @brief ORB DRIVE OTA module OLED display interface.
 */

#ifndef OTA_OLED_H
#define OTA_OLED_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void oled_init(void);

void oled_clear(void);

void oled_show_startup(void);

void oled_show_node_found(void);

void oled_show_node_connected(void);

void oled_show_upload_progress(
    size_t bytes_sent,
    size_t total_bytes
);

void oled_show_upload_complete(void);

void oled_show_failure(const char *reason);

#ifdef __cplusplus
}
#endif

#endif /* OTA_OLED_H */
