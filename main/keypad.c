/*
 * @file main/keypad.c
 *
 * @proj imp-term
 * @brief PIN checking and memory access related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include "gpio.h"
#include "main.h"
#include "keypad.h"

#include <string.h>

#include <esp_log.h>
#include <esp_check.h>

nvs_handle_t keypad_nvs_handle;

void nvs_set_defaults()
{
    char access_pin[] = "1234";
    char admin_pin[] = "00000000";
    ESP_ERROR_CHECK(nvs_open(KEYPAD_STORAGE_NS, NVS_READWRITE, &keypad_nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "access_pin", access_pin));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "admin_pin", admin_pin));
    ESP_ERROR_CHECK(nvs_commit(keypad_nvs_handle));
    nvs_close(keypad_nvs_handle);
    ESP_LOGE(PROJ_NAME, "Defaults set:\n\tAccess PIN: %s\n\tAdmin PIN: %s", access_pin, admin_pin);
}

void nvs_configure()
{
    ESP_LOGI(PROJ_NAME, "Configuring NVS");
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(PROJ_NAME, "Storage truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Set defaults if storage is empty
    err = nvs_open(KEYPAD_STORAGE_NS, NVS_READONLY, &keypad_nvs_handle);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(PROJ_NAME, "Storage not initialized, setting defaults");
        nvs_close(keypad_nvs_handle);
        nvs_set_defaults();
    } else {
        nvs_close(keypad_nvs_handle);
    }

    ESP_LOGI(PROJ_NAME, "NVS configured");
}

static esp_err_t check_pin(char * pin_to_check, bool * is_correct)
{
    char pin_set[5] = {0};
    size_t len = sizeof(pin_set);
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE_NS, NVS_READONLY, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_get_str(keypad_nvs_handle, "access_pin", pin_set, &len), "Error reading PIN from NVS", PROJ_NAME);

    if(strcmp(pin_to_check, pin_set) == 0) {
        ESP_LOGI(PROJ_NAME, "PIN correct");
        *is_correct = true;
    } else {
        ESP_LOGI(PROJ_NAME, "PIN incorrect");
        *is_correct = false;
    }

    nvs_close(keypad_nvs_handle);
    return ESP_OK;
}

void keypad_clear_pin(char * pin, uint8_t * pin_index)
{
    if(strlen(pin) == 0) {
        ESP_LOGD(PROJ_NAME, "PIN already empty");
        return;
    }
    ESP_LOGI(PROJ_NAME, "Clearing sequence: %s", pin);
    memset(pin, 0, strlen(pin));
    ESP_LOGD(PROJ_NAME, "Sequence cleared, new length: %d", strlen(pin));
    *pin_index = 0;
}

void keypad_keypress_handler(char key_pressed)
{
    ESP_LOGI(PROJ_NAME, "Key %c pressed", key_pressed);
    gpio_blink_nonblocking(STATUS_LED, 20);

    static char access_pin[KEYPAD_PIN_MAX_LEN] = {0};
    static uint8_t pin_index = 0;
    static enum {
        PIN_ENTER,
        PIN_CHANGE_REQUEST
    } pin_state = PIN_ENTER;

    switch(key_pressed) {
        case KEYPAD_PIN_SUBMIT_KEY:
            ESP_LOGI(PROJ_NAME, "Requested submit");
            ESP_LOGI(PROJ_NAME, "Validating pin: %s", access_pin);
            bool is_correct;
            ESP_ERROR_CHECK(check_pin(access_pin, &is_correct));
            keypad_clear_pin(access_pin, &pin_index);
            break;
        case KEYPAD_PIN_CHANGE_KEY:
            ESP_LOGI(PROJ_NAME, "Requested pin change");
            pin_state = PIN_CHANGE_REQUEST;
            keypad_clear_pin(access_pin, &pin_index);
            break;
        default:
            access_pin[pin_index++] = key_pressed;
            break;
    }

    if(pin_index >= sizeof(access_pin)) {
        ESP_LOGE(PROJ_NAME, "PIN too long, resetting");
        keypad_clear_pin(access_pin, &pin_index);
    }
}

noreturn void keypad_handler_task()
{
    uint8_t key;
    uint32_t io_num;

    while(1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // ESP_LOGI(PROJ_NAME, "GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if((key = gpio_keypad_key_lookup(io_num)) != E_KEYPAD_NO_KEY_FOUND) { // A key was pressed
                keypad_keypress_handler(key);
            }
        }
        xQueueReset(gpio_evt_queue);
    }
}
