/**
 * @file wifi_manager.h
 * @brief ORB DRIVE OTA module Wi-Fi client interface.
 */

#ifndef OTA_WIFI_MANAGER_H
#define OTA_WIFI_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Wi-Fi subsystem.
 *
 * Initializes the ESP32-S3 Wi-Fi station interface.
 */
void wifi_manager_init(void);

/**
 * @brief Scan for the frozen Orb Drive Node AP.
 *
 * @return true when "lora-node" is detected.
 */
bool wifi_manager_find_node(void);

/**
 * @brief Connect to the Orb Drive Node AP.
 *
 * @return true when Wi-Fi connection is established.
 */
bool wifi_manager_connect(void);

/**
 * @brief Check whether the OTA module is connected to Wi-Fi.
 *
 * @return true when connected.
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Get the IP address assigned to the OTA module.
 *
 * @return Pointer to a static IP address string.
 */
const char *wifi_manager_get_local_ip(void);

/**
 * @brief Check communication with the Node.
 *
 * Verifies that 192.168.4.1 can be reached.
 *
 * @return true when the Node is reachable.
 */
bool wifi_manager_node_reachable(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_WIFI_MANAGER_H */
