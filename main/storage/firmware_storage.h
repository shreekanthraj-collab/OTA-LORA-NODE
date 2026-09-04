/**
 * @file firmware_storage.h
 * @brief ORB DRIVE OTA module firmware storage interface.
 */

#ifndef FIRMWARE_STORAGE_H
#define FIRMWARE_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Firmware image descriptor.
 */
typedef struct
{
    const uint8_t *data;
    size_t size;
} firmware_image_t;

/**
 * @brief Initialize firmware storage.
 *
 * @return true when the storage layer is ready.
 */
bool firmware_storage_init(void);

/**
 * @brief Check whether a firmware image is available.
 *
 * @return true when a valid firmware image is available.
 */
bool firmware_storage_available(void);

/**
 * @brief Get the firmware image.
 *
 * @param image Output firmware image descriptor.
 *
 * @return true when a valid firmware image is available.
 */
bool firmware_storage_get_image(
    firmware_image_t *image
);

/**
 * @brief Release the firmware image.
 *
 * This allows the storage implementation to release
 * any resources associated with the image.
 *
 * @param image Firmware image descriptor.
 */
void firmware_storage_release(
    firmware_image_t *image
);

#ifdef __cplusplus
}
#endif

#endif /* FIRMWARE_STORAGE_H */
