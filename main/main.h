/*
 * @file main/main.h
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_MAIN_H
#define IMP_TERM_MAIN_H

#include <stdint.h>
#include <stdnoreturn.h>

#include <esp_log.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

// CONFIGURABLE OPTIONS

#define PROJ_NAME "imp-term"


// CONVENIENCE DEFINITIONS

#define ms_in_s 1000
#define seconds(seconds) (seconds * ms_in_s)

#define vTaskDelayMSec(milis) vTaskDelay(milis              / portTICK_PERIOD_MS)
#define vTaskDelaySec(milis)  vTaskDelay(milis * ms_in_s    / portTICK_PERIOD_MS)

#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))


#endif // IMP_TERM_MAIN_H
