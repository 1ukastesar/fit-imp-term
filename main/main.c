/*
 * @file main/main.c
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include "gpio.h"
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

void main_task()
{
    ESP_LOGI(PROJ_NAME, "Executing main task");
    // xTaskCreate(&led_heartbeat_task, "led_heartbeat", 2048, NULL, 5, NULL);
    xTaskCreate(&keypad_handler_task, "keypad_handler", 2048, NULL, 5, NULL);
}

void app_main(void)
{
    gpio_configure();
    main_task();
}
