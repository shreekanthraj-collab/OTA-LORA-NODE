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

/* ============================================================
 * Firmware storage status
 * ============================================================ */

typedef enum
{
    FIRMWARE_STORAGE_OK = 0,
    FIRMWARE_STORAGE_NOT_INITIALIZED,
    FIRMWARE_STORAGE_NOT_AVAILABLE,
    FIRMWARE_STORAGE_INVALID,
    FIRMWARE_STORAGE_READ_ERROR
} firmware_storage_status_t;

/* ============================================================
 * Initialization
 * ============================================================ */

firmware_storage_status_t firmware_storage_init(void);

/* ============================================================
 * Firmware information
 * ============================================================ */

/**
 * @brief Check whether a firmware image is available.
 *
 * @return true if firmware is available, otherwise false.
 */
bool firmware_storage_is_available(void);

/**
 * @brief Get the firmware image size.
 *
 * @param size Output firmware size in bytes.
 *
 * @return FIRMWARE_STORAGE_OK on success.
 */
firmware_storage_status_t firmware_storage_get_size(
    size_t *size
);

/* ============================================================
 * Firmware read
 * ============================================================ */

/**
 * @brief Read firmware data from the storage source.
 *
 * @param offset Offset into firmware image.
 * @param buffer Destination buffer.
 * @param length Number of bytes to read.
 *
 * @return FIRMWARE_STORAGE_OK on success.
 */
firmware_storage_status_t firmware_storage_read(
    size_t offset,
    uint8_t *buffer,
    size_t length
);

#ifdef __cplusplus
}
#endif

#endif /* FIRMWARE_STORAGE_H */
