/*
 * @file gpio.c
 *
 * @proj imp-term
 * @brief GPIO related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#ifndef IMP_TERM_GPIO_H
#define IMP_TERM_GPIO_H

#include <inttypes.h>
#include <stdnoreturn.h>

#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

// CONFIGURABLE OPTIONS

// GPIO port number definitions
#define STATUS_LED GPIO_NUM_2 // Onboard LED GPIO pin

// CONVENIENCE DEFINITIONS

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define E_KEYPAD_NO_KEY_FOUND ((uint8_t) -1)


void gpio_blink_blocking(const uint8_t gpio_num, const uint16_t duration);
void gpio_blink_nonblocking(const uint8_t gpio_num, const uint16_t duration);
void gpio_configure();
uint8_t keyboard_lookup_key();
noreturn void check_keyboard_task();

#endif // IMP_TERM_GPIO_H
