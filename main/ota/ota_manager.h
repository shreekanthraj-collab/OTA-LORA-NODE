/**
 * @file ota_manager.h
 * @brief ORB DRIVE OTA module HTTP firmware upload manager.
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ota_progress_callback_t)(
    size_t bytes_sent,
    size_t total_bytes,
    void *context
);

/**
 * @brief Initialize the OTA manager.
 */
void ota_manager_init(void);

/**
 * @brief Upload a firmware image to the Node OTA endpoint.
 *
 * @param firmware_data Firmware image buffer.
 * @param firmware_size Firmware image size in bytes.
 * @param progress_callback Optional callback receiving actual bytes sent.
 * @param context User context passed to the callback.
 *
 * @return true when the Node accepts the OTA upload successfully.
 */
bool ota_manager_upload(
    const uint8_t *firmware_data,
    size_t firmware_size,
    ota_progress_callback_t progress_callback,
    void *context
);

#ifdef __cplusplus
}
#endif

#endif /* OTA_MANAGER_H */
