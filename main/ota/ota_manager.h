/**
 * @file ota_manager.h
 * @brief ORB DRIVE OTA module firmware upload interface.
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void ota_manager_init(void);

bool ota_manager_start_upload(
    const char *firmware_path,
    size_t firmware_size
);

#ifdef __cplusplus
}
#endif

#endif /* OTA_MANAGER_H */
