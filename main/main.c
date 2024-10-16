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


static void gpio_blink_blocking(const uint8_t gpio_num, const uint32_t duration)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_HIGH));
    vTaskDelayMSec(duration);
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_LOW));
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
