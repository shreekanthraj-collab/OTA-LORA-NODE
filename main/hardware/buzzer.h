/**
 * @file buzzer.h
 * @brief ORB DRIVE OTA module buzzer interface.
 */

#ifndef OTA_BUZZER_H
#define OTA_BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);

void buzzer_short_beep(void);

void buzzer_long_beep(void);

void buzzer_success(void);

void buzzer_error(void);

#ifdef __cplusplus
}
#endif

#endif /* OTA_BUZZER_H */
