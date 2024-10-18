/*
 * @file main/gpio.c
 *
 * @proj imp-term
 * @brief GPIO related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include <esp_log.h>

#include <soc/gpio_reg.h>

#include "gpio.h"
#include "main.h"

char * gpio_pad_map[][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"*", "0", "#"}
};

// Rows and columns are not in order
static int gpio_keypad_pin_cols[] = {GPIO_NUM_26, GPIO_NUM_5, GPIO_NUM_17};
static int gpio_keypad_pin_rows[] = {GPIO_NUM_23, GPIO_NUM_27, GPIO_NUM_16, GPIO_NUM_25};

#define map_keypad_col_to_gpio_pin(col) gpio_keypad_pin_cols[col]
#define map_keypad_row_to_gpio_pin(row) gpio_keypad_pin_rows[row]

static int gpio_keypad_col_mask = 0;
static int gpio_keypad_row_mask = 0;

static volatile uint32_t *gpio_w1ts_reg = (volatile uint32_t *) GPIO_OUT_W1TS_REG;
static volatile uint32_t *gpio_w1tc_reg = (volatile uint32_t *) GPIO_OUT_W1TC_REG;

QueueHandle_t gpio_evt_queue = NULL;

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

static void IRAM_ATTR gpio_keypad_handler(void* arg)
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

    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));
    for(uint8_t row = 0; row < array_len(gpio_keypad_pin_rows); row++) {
        uint32_t row_pin = map_keypad_row_to_gpio_pin(row);
        ESP_ERROR_CHECK(gpio_isr_handler_add(row_pin, &gpio_keypad_handler, (void*) row_pin));
    }

    ESP_LOGI(PROJ_NAME, "GPIO pins configured");
}

uint8_t keypad_key_lookup(uint32_t io_num) {
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
            goto end;
        }
        ESP_ERROR_CHECK(gpio_set_level(col_pin, GPIO_LOW));
    }

    end:
    *gpio_w1ts_reg = gpio_keypad_col_mask; // Restore original state (all columns HIGH)
    return key;
}

noreturn void keypad_handler_task()
{
    uint8_t key;
    uint32_t io_num;

    while(1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if((key = keypad_key_lookup(io_num)) != E_KEYPAD_NO_KEY_FOUND) { // A key was pressed
                ESP_LOGI(PROJ_NAME, "key %c pressed", key);
                gpio_blink_nonblocking(STATUS_LED, 20);
            }
        }
        xQueueReset(gpio_evt_queue);
    }
}
