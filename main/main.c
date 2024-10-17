/*
 * @file main.c
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include <inttypes.h>
#include <stdio.h>
#include <stdnoreturn.h>

#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#include "gpio.h"
#include "main.h"


noreturn void led_heartbeat_task() {
    // Blink every second indefinitely
    ESP_LOGI(PROJ_NAME, "Heartbeat blink started");
    while(1) {
        if(gpio_get_level(STATUS_LED) == GPIO_LOW) // Not currently used by other tasks
            gpio_blink_blocking(STATUS_LED, seconds(0.1));
        vTaskDelaySec(0.9);
    }
}

noreturn void check_keyboard_task() {
    while(1) {
        uint8_t key;
        if((key = keyboard_lookup_key()) != E_KEYPAD_NO_KEY_FOUND) { // A key was pressed
            ESP_LOGI(PROJ_NAME, "key %i was pressed", key);
            // gpio_blink_nonblocking(STATUS_LED, seconds(2));
        }
        vTaskDelayMSec(20);
    }
}

void main_task()
{
    ESP_LOGI(PROJ_NAME, "Executing main task");
    xTaskCreate(&led_heartbeat_task, "led_heartbeat_task", 2048, NULL, 5, NULL);
    xTaskCreate(&check_keyboard_task, "check_keyboard_task", 2048, NULL, 5, NULL);
}

void app_main(void)
{
    gpio_configure();
    main_task();
}
