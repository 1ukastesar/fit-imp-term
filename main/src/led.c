/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "led.h"
#include "common.h"
#include "config.h"

/* Private variables */
static uint8_t led_state;

/* Public functions */
uint8_t get_led_state(void) { return led_state; }

void led_on(void) { gpio_set_level(STATUS_LED, true); }

void led_off(void) { gpio_set_level(STATUS_LED, false); }

void led_init(void) {
    ESP_LOGI(TAG, "example configured to blink gpio led!");
    gpio_reset_pin(STATUS_LED);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT);
}
