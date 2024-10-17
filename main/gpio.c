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

static int gpio_keypad_conn_map[] = {
    GPIO_NUM_5,     // = pin 1
    GPIO_NUM_23,    // = pin 2
    GPIO_NUM_26,    // = pin 3
    GPIO_NUM_25,    // = pin 4
    GPIO_NUM_17,    // = pin 5
    GPIO_NUM_16,    // = pin 6
    GPIO_NUM_27     // = pin 7
};

char * gpio_pad_map[][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
};

// Rows and columns are not in order
static int gpio_keypad_pin_cols[] = {2, 0, 4};
static int gpio_keypad_pin_rows[] = {1, 6, 5, 3};

#define map_keypad_col_to_gpio_pin(col) gpio_keypad_conn_map[gpio_keypad_pin_cols[col]]
#define map_keypad_row_to_gpio_pin(row) gpio_keypad_conn_map[gpio_keypad_pin_rows[row]]

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

    // Keypad
    gpio_config_t col_conf = {};
    col_conf.intr_type = GPIO_INTR_DISABLE;
    col_conf.mode = GPIO_MODE_OUTPUT;
    for(uint8_t col = 0; col < array_len(gpio_keypad_pin_cols); col++) {
        uint8_t col_pin = map_keypad_col_to_gpio_pin(col);
        col_conf.pin_bit_mask |= (1ULL << col_pin);
    }

    gpio_config_t row_conf = {};
    row_conf.intr_type = GPIO_INTR_POSEDGE;
    row_conf.mode = GPIO_MODE_INPUT;
    row_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    for(uint8_t row = 0; row < array_len(gpio_keypad_pin_rows); row++) {
        uint8_t row_pin = map_keypad_row_to_gpio_pin(row);
        row_conf.pin_bit_mask |= (1ULL << row_pin);
    }

    ESP_ERROR_CHECK(gpio_config(&col_conf));
    ESP_ERROR_CHECK(gpio_config(&row_conf));

    ESP_LOGI(PROJ_NAME, "GPIO pins configured");
}

uint8_t keyboard_lookup_key() {
    for(uint8_t col = 0; col < array_len(gpio_keypad_pin_cols); col++) {
        uint8_t col_pin = map_keypad_col_to_gpio_pin(col);
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_LOW));
    }

    for(uint8_t col = 0; col < array_len(gpio_keypad_pin_cols); col++) {
        uint8_t col_pin = map_keypad_col_to_gpio_pin(col);
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_HIGH));

        for(uint8_t row = 0; row < array_len(gpio_keypad_pin_rows); row++) {
            uint8_t row_pin = map_keypad_row_to_gpio_pin(row);
            if(gpio_get_level(row_pin) == GPIO_HIGH) {
                return gpio_pad_map[row][col][0];
            }
        }
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_LOW));
    }

    return E_KEYPAD_NO_KEY_FOUND;
}
