/*
 * @file main/config.h
 *
 * @proj imp-term
 * @brief Project config
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_CONFIG_H
#define IMP_TERM_CONFIG_H

#include <driver/gpio.h>

// CONFIGURABLE OPTIONS

#define DOOR_OPEN_FOR_SEC 10 // Time in seconds the door stay open

#define KEYPAD_DEFAULT_ACCESS_PIN "1234"
#define KEYPAD_DEFAULT_ADMIN_PIN "00000000"

#define KEYPAD_PIN_MIN_LEN 4
#define KEYPAD_PIN_MAX_LEN 10
#define KEYPAD_PIN_SUBMIT_KEY '#'
#define KEYPAD_PIN_CHANGE_KEY '*'

// GPIO port number definitions
#define STATUS_LED      GPIO_NUM_2  // Onboard LED GPIO pin
#define DOOR_OPEN_LED   GPIO_NUM_19 // Door open LED GPIO pin
#define DOOR_CLOSED_LED GPIO_NUM_18 // Door closed LED GPIO pin

// Rows and columns are not in order
#define GPIO_KEYPAD_PIN_COLS {GPIO_NUM_26, GPIO_NUM_5, GPIO_NUM_17}
#define GPIO_KEYPAD_PIN_ROWS {GPIO_NUM_23, GPIO_NUM_27, GPIO_NUM_16, GPIO_NUM_25}

#define KEYPAD_STORAGE_NS "keypad"
#define ESP_INTR_FLAG_DEFAULT 0 // Default interrupt flags

#endif // IMP_TERM_CONFIG_H
