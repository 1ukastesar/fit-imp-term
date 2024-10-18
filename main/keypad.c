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

#define KEYPAD_STORAGE "keypad"

nvs_handle_t keypad_nvs_handle;

void nvs_set_defaults() {
    char access_pin[] = "1234";
    char admin_pin[] = "00000000";
    ESP_ERROR_CHECK(nvs_open(KEYPAD_STORAGE, NVS_READWRITE, &keypad_nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "access_pin", access_pin));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "admin_pin", admin_pin));
    ESP_ERROR_CHECK(nvs_commit(keypad_nvs_handle));
    nvs_close(keypad_nvs_handle);
    ESP_LOGE(PROJ_NAME, "Defaults set:\n\tAccess PIN: %s\n\tAdmin PIN: %s", access_pin, admin_pin);
}

void nvs_configure() {
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
    err = nvs_open(KEYPAD_STORAGE, NVS_READONLY, &keypad_nvs_handle);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(PROJ_NAME, "Namespace not found, setting defaults");
        nvs_close(keypad_nvs_handle);
        nvs_set_defaults();
    } else {
        nvs_close(keypad_nvs_handle);
    }

    ESP_LOGI(PROJ_NAME, "NVS configured");
}

static esp_err_t check_pin(char * pin_to_check, bool * is_correct) {
    char pin_set[5] = {0};
    size_t len = sizeof(pin_set);
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE, NVS_READONLY, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
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

void keypad_keypress_handler(char key_pressed) {
    ESP_LOGI(PROJ_NAME, "key %c pressed", key_pressed);
    gpio_blink_nonblocking(STATUS_LED, 20);

    static char access_pin[5] = {0};
    static uint8_t pin_index = 0;

    switch(key_pressed) {
        case '#':
            ESP_LOGI(PROJ_NAME, "access pin: %s", access_pin);
            bool is_correct;
            ESP_ERROR_CHECK(check_pin(access_pin, &is_correct));
            memset(access_pin, 0, sizeof(access_pin));
            pin_index = 0;
            break;
        default:
            access_pin[pin_index++] = key_pressed;
            break;
    }

}
