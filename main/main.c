/*
 * @file main/main.c
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "config.h"
#include "gpio.h"
#include "keypad.h"
#include "main.h"

noreturn void led_heartbeat_task() {
    // Blink every second indefinitely
    ESP_LOGI(PROJ_NAME, "Heartbeat blink started");
    while(1) {
        if(gpio_get_level(STATUS_LED) == GPIO_LOW) { // Not currently used by other tasks
            gpio_blink_blocking(STATUS_LED, seconds(0.1));
            vTaskDelaySec(0.1);
            gpio_blink_blocking(STATUS_LED, seconds(0.1));
        }
        vTaskDelaySec(0.7);
    }
}

void app_main(void)
{
    // Initialization
    gpio_configure();
    nvs_configure();

    ESP_LOGI(PROJ_NAME, "Initialization complete");
    ESP_LOGI(PROJ_NAME, "Starting tasks...");

    // Create long-running tasks
    // Uncomment to enable heartbeat
    if(xTaskCreate(&led_heartbeat_task, "led_heartbeat", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create heartbeat task");
        abort();
    }
    if(xTaskCreate(&keypad_handler_task, "keypad_handler", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create keypad handler task");
        abort();
    }

    if(xTaskCreate(&door_handler_task, "door_handler", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create door handler task");
        abort();
    }
}
