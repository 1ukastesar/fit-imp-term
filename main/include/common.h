/*
 * @file main/common.h
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_COMMON_H
#define IMP_TERM_COMMON_H


// CONFIGURABLE OPTIONS

#define PROJ_NAME "imp-term"
#define GATT_TAG "nimble-gatt"

// CONVENIENCE DEFINITIONS

#define ms_in_s 1000
#define seconds(seconds) (seconds * ms_in_s)

#define vTaskDelayMSec(milis) vTaskDelay(milis              / portTICK_PERIOD_MS)
#define vTaskDelaySec(milis)  vTaskDelay(milis * ms_in_s    / portTICK_PERIOD_MS)

#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))

/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* Includes */
/* STD APIs */
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* ESP APIs */
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

/* FreeRTOS APIs */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* NimBLE stack APIs */
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include <freertos/FreeRTOS.h>

#endif // IMP_TERM_COMMON_H
