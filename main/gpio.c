/*
 * @file gpio.c
 *
 * @proj imp-term
 * @brief GPIO related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include "esp_log.h"
#include "sdkconfig.h"

#include "gpio.h"
#include "main.h"

static int gpio_keypad_pin_map[] = {
    GPIO_NUM_5,     // = pin 1
    GPIO_NUM_23,    // = pin 2
    GPIO_NUM_26,    // = pin 3
    GPIO_NUM_25,    // = pin 4
    GPIO_NUM_17,    // = pin 5
    GPIO_NUM_16,    // = pin 6
    GPIO_NUM_27     // = pin 7
};

void gpio_blink_blocking(const uint8_t gpio_num, const uint16_t duration)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_HIGH));
    vTaskDelayMSec(duration);
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_LOW));
}

static noreturn void gpio_blink_task(void * param)
{
    uint32_t num_duration = (uint32_t) param;
    gpio_blink_blocking(num_duration & 0xFF, num_duration >> 8);
    vTaskDelete(NULL); // Delete self
    while(1); // Wait for deletion
}

void gpio_blink_nonblocking(const uint8_t gpio_num, const uint16_t duration)
{
    uint32_t num_duration = duration << 8 | gpio_num;
    xTaskCreate(&gpio_blink_task, "gpio_blink_nonblocking", 1024, (void*) num_duration, 5, NULL);
}

void gpio_configure()
{
    ESP_LOGI(PROJ_NAME, "Configuring GPIO pins");
    // Onboard status LED
    ESP_ERROR_CHECK(gpio_reset_pin(STATUS_LED));
    ESP_ERROR_CHECK(gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT));

    for(uint8_t i = 0; i < array_len(gpio_keypad_pin_map); i++) {
        ESP_ERROR_CHECK(gpio_reset_pin(gpio_keypad_pin_map[i]));
        ESP_ERROR_CHECK(gpio_set_direction(gpio_keypad_pin_map[i], GPIO_MODE_INPUT_OUTPUT));
    }
}

uint8_t keyboard_lookup_key() {
    ESP_ERROR_CHECK(gpio_set_direction(gpio_keypad_pin_map[2], GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(gpio_keypad_pin_map[2], GPIO_HIGH));
    if(gpio_get_level(gpio_keypad_pin_map[3]) == GPIO_HIGH) {
        return gpio_keypad_pin_map[3];
    }

    return E_KEYPAD_NO_KEY_FOUND;
}
