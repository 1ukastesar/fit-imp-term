/*
 * @file main/gpio.c
 *
 * @proj imp-term
 * @brief GPIO related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include <esp_log.h>
#include <esp_check.h>

#include <soc/gpio_reg.h>

#include "config.h"
#include "gpio.h"
#include "keypad.h"
#include "main.h"

static char * gpio_pad_map[][3] =
{
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
};

static int gpio_keypad_pin_cols[] = GPIO_KEYPAD_PIN_COLS;
static int gpio_keypad_pin_rows[] = GPIO_KEYPAD_PIN_ROWS;

static int gpio_keypad_col_mask;
static int gpio_keypad_row_mask;

static volatile uint32_t *gpio_w1ts_reg = (volatile uint32_t *) GPIO_OUT_W1TS_REG;
static volatile uint32_t *gpio_w1tc_reg = (volatile uint32_t *) GPIO_OUT_W1TC_REG;

QueueHandle_t gpio_evt_queue;

void gpio_blink_blocking(const uint8_t gpio_num, const uint16_t duration)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_HIGH));
    vTaskDelayMSec(duration);
    ESP_ERROR_CHECK(gpio_set_level(gpio_num, GPIO_LOW));
}

/*
 * @brief Task to blink a GPIO pin in a non-blocking manner
 * @param param GPIO pin number and duration
 * @note uint32_t param = ((uint16_t) duration << 8 | (uint8_t) gpio_num)
*/
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

void gpio_blink_twice_blocking(const uint32_t gpio_num)
{
    gpio_blink_blocking(gpio_num, seconds(0.05));
    vTaskDelaySec(0.1);
    gpio_blink_blocking(gpio_num, seconds(0.05));
}

/*
 * @brief Task to blink a GPIO pin twice in a non-blocking manner
 * @param param GPIO pin number
*/
static noreturn void gpio_blink_twice_task(void * param)
{
    uint32_t gpio_num = (uint32_t) param;
    gpio_blink_twice_blocking(gpio_num);
    vTaskDelete(NULL); // Delete self
    while(1); // Wait for deletion
}

void gpio_blink_twice_nonblocking(const uint32_t gpio_num)
{
    xTaskCreate(&gpio_blink_twice_task, "gpio_blink_success", 1024, (void*) gpio_num, 5, NULL);
}

/*
 * @brief GPIO interrupt handler for keypad
*/
static void IRAM_ATTR gpio_keypad_interrupt(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void gpio_configure()
{
    ESP_LOGI(PROJ_NAME, "Configuring GPIO pins");

    // Onboard status LED
    ESP_ERROR_CHECK(gpio_reset_pin(STATUS_LED));
    ESP_ERROR_CHECK(gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT));

    // Door open LED
    ESP_ERROR_CHECK(gpio_reset_pin(DOOR_OPEN_LED));
    ESP_ERROR_CHECK(gpio_set_direction(DOOR_OPEN_LED, GPIO_MODE_OUTPUT));

    // Door closed LED
    ESP_ERROR_CHECK(gpio_reset_pin(DOOR_CLOSED_LED));
    ESP_ERROR_CHECK(gpio_set_direction(DOOR_CLOSED_LED, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(DOOR_CLOSED_LED, GPIO_HIGH));

    // Keypad
    gpio_config_t col_conf = {};
    col_conf.intr_type = GPIO_INTR_DISABLE;
    col_conf.mode = GPIO_MODE_OUTPUT;
    for(uint8_t col = 0; col < array_len(gpio_keypad_pin_cols); col++) {
        uint8_t col_pin = map_keypad_col_to_gpio_pin(col);
        gpio_keypad_col_mask |= (1 << col_pin);
        gpio_set_level(col_pin, GPIO_HIGH);
    }

    col_conf.pin_bit_mask = gpio_keypad_col_mask;

    gpio_config_t row_conf = {};
    row_conf.intr_type = GPIO_INTR_POSEDGE;
    row_conf.mode = GPIO_MODE_INPUT;
    row_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    for(uint8_t row = 0; row < array_len(gpio_keypad_pin_rows); row++) {
        uint8_t row_pin = map_keypad_row_to_gpio_pin(row);
        gpio_keypad_row_mask |= (1 << row_pin);
    }

    row_conf.pin_bit_mask = gpio_keypad_row_mask;

    ESP_ERROR_CHECK(gpio_config(&col_conf));
    ESP_ERROR_CHECK(gpio_config(&row_conf));

    gpio_evt_queue = xQueueCreate(3, sizeof(uint32_t));
    if(gpio_evt_queue == NULL) {
        ESP_LOGE(PROJ_NAME, "Failed to create GPIO event queue");
        abort();
    }

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));
    for(uint8_t row = 0; row < array_len(gpio_keypad_pin_rows); row++) {
        uint32_t row_pin = map_keypad_row_to_gpio_pin(row);
        ESP_ERROR_CHECK(gpio_isr_handler_add(row_pin, &gpio_keypad_interrupt, (void*) row_pin));
    }

    ESP_LOGI(PROJ_NAME, "GPIO pins configured");
}

uint8_t gpio_keypad_key_lookup(uint32_t io_num)
{
    uint8_t key = E_KEYPAD_NO_KEY_FOUND;
    uint8_t row;

    // First find out which row was pressed from GPIO number
    // TODO rewrite this using reverse lookup table
    for(uint8_t row_i = 0; row_i < array_len(gpio_keypad_pin_rows); row_i++) {
        if(io_num == map_keypad_row_to_gpio_pin(row_i))
            row = row_i;
    }

    // Quickly set all columns to LOW
    *gpio_w1tc_reg = gpio_keypad_col_mask;

    // Iterate over columns setting each to HIGH and checking if the row goes HIGH
    for(uint8_t col = 0; col < array_len(gpio_keypad_pin_cols); col++) {
        uint8_t col_pin = map_keypad_col_to_gpio_pin(col);
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_HIGH));
        if(gpio_get_level(io_num) == GPIO_HIGH) {
            key = gpio_pad_map[row][col][0];
            break;
        }
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_LOW));
    }

    *gpio_w1ts_reg = gpio_keypad_col_mask; // Restore original state (all columns HIGH)
    return key;
}
