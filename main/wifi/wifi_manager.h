/**
 * @file wifi_manager.h
 * @brief ORB DRIVE OTA module Wi-Fi station interface.
 */

#ifndef OTA_WIFI_MANAGER_H
#define OTA_WIFI_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void wifi_manager_init(void);

bool wifi_manager_scan_node(void);

bool wifi_manager_connect(void);

bool wifi_manager_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_WIFI_MANAGER_H */
