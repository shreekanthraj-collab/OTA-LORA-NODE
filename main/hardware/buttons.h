/**
 * @file buttons.h
 * @brief ORB DRIVE OTA module button interface.
 */

#ifndef OTA_BUTTONS_H
#define OTA_BUTTONS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void buttons_init(void);

bool button_select_pressed(void);

bool button_up_pressed(void);

bool button_down_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_BUTTONS_H */
