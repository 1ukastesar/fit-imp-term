/*
 * @proj imp-term
 *
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


// CONFIGURABLE OPTIONS

#define PROJ_NAME "imp-term"
#define TAG_INIT "-init"

// GPIO port number definitions
#define STATUS_LED GPIO_NUM_2 // Onboard LED GPIO pin


// CONVENIENCE DEFINITIONS

#define GPIO_LOW 0
#define GPIO_HIGH 1

#define ms_in_s 1000
#define seconds(seconds) (seconds * ms_in_s)

#define vTaskDelayMSec(milis) vTaskDelay(milis              / portTICK_PERIOD_MS)
#define vTaskDelaySec(milis)  vTaskDelay(milis * ms_in_s    / portTICK_PERIOD_MS)

#define array_len(arr) (sizeof(arr) / sizeof(arr[0]))

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

static noreturn void gpio_blink_task(void * param) {
    // struct blink_opt_t * opt = (struct blink_opt_t *) param;
    uint32_t num_duration = (uint32_t) param;
    gpio_blink_blocking(num_duration & 0xFF, num_duration >> 8);
    vTaskDelete(NULL); // Delete self
    while(1); // Wait for deletion
}

void gpio_blink_nonblocking(const uint8_t gpio_num, const uint16_t duration) {
    uint32_t num_duration = duration << 8 | gpio_num;
    xTaskCreate(&gpio_blink_task, "gpio_blink_nonblocking", 1024, (void*) num_duration, 5, NULL);
}

static void gpio_configure()
{
    ESP_LOGI(PROJ_NAME TAG_INIT, "Configuring GPIO pins");
    // Onboard status LED
    ESP_ERROR_CHECK(gpio_reset_pin(STATUS_LED));
    ESP_ERROR_CHECK(gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT));
}

noreturn void main_task()
{
    ESP_LOGI(PROJ_NAME TAG_INIT, "Executing main task");
    // Main task loop
    while(1) {
        // For now, just blink every second
        gpio_blink_blocking(STATUS_LED, seconds(0.1));
        vTaskDelayMSec(seconds(0.9));
    }
}

void app_main(void)
{
    gpio_configure();
    main_task();
}
