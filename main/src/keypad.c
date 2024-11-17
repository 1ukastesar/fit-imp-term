/*
 * @file main/keypad.c
 *
 * @proj imp-term
 * @brief PIN checking and memory access related functions
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include "config.h"
#include "gpio.h"
#include "keypad.h"
#include "common.h"

#include <string.h>

#include <esp_log.h>
#include <esp_check.h>
#include <nvs.h>
#include <nvs_flash.h>

nvs_handle_t keypad_nvs_handle;

QueueHandle_t door_evt_queue;

enum DoorState {
    DOOR_OPEN,
    DOOR_CLOSE
};

enum DoorState door_state = DOOR_CLOSE;

void nvs_set_defaults()
{
    char access_pin[] = KEYPAD_DEFAULT_ACCESS_PIN;
    char admin_pin[] = KEYPAD_DEFAULT_ADMIN_PIN;
    ESP_ERROR_CHECK(nvs_open(KEYPAD_STORAGE_NAME, NVS_READWRITE, &keypad_nvs_handle));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "access_pin", access_pin));
    ESP_ERROR_CHECK(nvs_set_str(keypad_nvs_handle, "admin_pin", admin_pin));
    ESP_ERROR_CHECK(nvs_set_u16(keypad_nvs_handle, "door_duration", DOOR_OPEN_TIME_SEC));
    ESP_ERROR_CHECK(nvs_commit(keypad_nvs_handle));
    nvs_close(keypad_nvs_handle);
    ESP_LOGE(PROJ_NAME, "Defaults set:\n\tAccess PIN: %s\n\tAdmin PIN: %s\n\tDoor open duration: %u", access_pin, admin_pin, DOOR_OPEN_TIME_SEC);
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
    err = nvs_open(KEYPAD_STORAGE_NAME, NVS_READONLY, &keypad_nvs_handle);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        vTaskDelaySec(2); // Wait for serial monitor to connect
        ESP_LOGE(PROJ_NAME, "Storage not initialized, setting defaults");
        nvs_close(keypad_nvs_handle);
        nvs_set_defaults();
    } else {
        nvs_close(keypad_nvs_handle);
    }

    ESP_LOGI(PROJ_NAME, "NVS configured");
}

static esp_err_t check_pin(const char * pin_to_check, const char * pin_name, bool * is_correct)
{
    *is_correct = false;
    char pin_set[KEYPAD_PIN_MAX_LEN] = {0};
    size_t len = sizeof(pin_set);
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE_NAME, NVS_READONLY, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_get_str(keypad_nvs_handle, pin_name, pin_set, &len), "Error reading PIN from NVS", PROJ_NAME);

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

esp_err_t change_pin(const char * new_pin, const char * pin_name)
{
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE_NAME, NVS_READWRITE, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_set_str(keypad_nvs_handle, pin_name, new_pin), "Error writing PIN to NVS", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_commit(keypad_nvs_handle), "Error committing changes", PROJ_NAME);
    nvs_close(keypad_nvs_handle);
    ESP_LOGI(PROJ_NAME, "%s updated to %s", pin_name, new_pin);
    return ESP_OK;
}

esp_err_t update_door_duration(uint16_t duration)
{
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE_NAME, NVS_READWRITE, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_set_u16(keypad_nvs_handle, "door_duration", duration), "Error writing duration to NVS", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_commit(keypad_nvs_handle), "Error committing changes", PROJ_NAME);
    nvs_close(keypad_nvs_handle);
    ESP_LOGI(PROJ_NAME, "Door duration updated to %d seconds", duration);
    return ESP_OK;
}

esp_err_t read_door_duration(uint16_t * duration)
{
    size_t len = sizeof(*duration);
    ESP_RETURN_ON_ERROR(nvs_open(KEYPAD_STORAGE_NAME, NVS_READONLY, &keypad_nvs_handle), "Error opening handle", PROJ_NAME);
    ESP_RETURN_ON_ERROR(nvs_get_u16(keypad_nvs_handle, "door_duration", duration), "Error reading duration from NVS", PROJ_NAME);
    nvs_close(keypad_nvs_handle);
    return ESP_OK;
}

void keypad_clear_pin(char * pin, uint8_t * pin_index)
{
    if(strlen(pin) == 0) {
        ESP_LOGD(PROJ_NAME, "PIN already empty");
        return;
    }
    ESP_LOGD(PROJ_NAME, "Clearing sequence: %s", pin);
    memset(pin, 0, strlen(pin));
    ESP_LOGD(PROJ_NAME, "Sequence cleared, new length: %d", strlen(pin));
    *pin_index = 0;
}

/*
 * @brief Wait for a security delay after a failed attempt
 * @note This function will block the keypad for KEYPAD_SECURITY_DELAY_SEC seconds
 */
void wait_security_delay()
{
    gpio_set_level(DOOR_CLOSED_LED, GPIO_LOW);
    vTaskDelaySec(0.1);
    gpio_blink_twice_blocking(DOOR_CLOSED_LED);
    vTaskDelaySec(0.1);
    vTaskDelaySec(KEYPAD_SECURITY_DELAY_SEC);
    gpio_set_level(DOOR_CLOSED_LED, GPIO_HIGH);
}

void keypad_keypress_handler(char key_pressed)
{
    ESP_LOGI(PROJ_NAME, "Key %c pressed", key_pressed);

    static char pin[KEYPAD_PIN_MAX_LEN] = {0};
    static uint8_t pin_index = 0;

    static enum {
        PIN_AUTH,
        PIN_CHANGE_AUTH,
        PIN_CHANGE_ENTER_NEW,
        PIN_CHANGE_CONFIRM
    } pin_state = PIN_AUTH;

    enum {
        NONE,
        SUCCESS,
        FAIL
    } error_state = NONE;

    bool is_correct = false;

    if(door_state == DOOR_OPEN) {
        // Immediately close the door
        ESP_LOGI(PROJ_NAME, "Requested immediate door close");
        enum DoorState evt = DOOR_CLOSE;
        xQueueSend(door_evt_queue, &evt, portMAX_DELAY);
        return;
    }

    switch(key_pressed) {
        case KEYPAD_PIN_SUBMIT_KEY:
            ESP_LOGI(PROJ_NAME, "Requested submit");
            switch(pin_state) {
                case PIN_AUTH:
                    ESP_LOGI(PROJ_NAME, "Checking access PIN");
                    ESP_ERROR_CHECK(check_pin(pin, "access_pin", &is_correct));
                    if(is_correct) {
                        ESP_LOGI(PROJ_NAME, "Access granted");
                        enum DoorState evt = DOOR_OPEN;
                        xQueueSend(door_evt_queue, (void*) &evt, portMAX_DELAY);
                    } else {
                        ESP_LOGI(PROJ_NAME, "Access denied");
                        error_state = FAIL;
                    }
                    break;

                case PIN_CHANGE_AUTH:
                    ESP_LOGI(PROJ_NAME, "Checking admin PIN");
                    ESP_ERROR_CHECK(check_pin(pin, "admin_pin", &is_correct));
                    if(is_correct) {
                        ESP_LOGI(PROJ_NAME, "Admin access granted");
                        ESP_LOGI(PROJ_NAME, "Enter new PIN");
                        pin_state = PIN_CHANGE_ENTER_NEW;
                        gpio_set_level(DOOR_CLOSED_LED, GPIO_LOW);
                        error_state = SUCCESS;
                    } else {
                        ESP_LOGI(PROJ_NAME, "Admin access denied");
                        pin_state = PIN_AUTH; // Return to normal state
                        error_state = FAIL;
                    }
                    break;

                case PIN_CHANGE_ENTER_NEW:
                    if(strlen(pin) < KEYPAD_PIN_MIN_LEN) {
                        ESP_LOGI(PROJ_NAME, "PIN too short (minimum %u), try again", KEYPAD_PIN_MIN_LEN);
                        error_state = FAIL;
                        break;
                    }
                    ESP_ERROR_CHECK(change_pin(pin, "new_pin"));
                    ESP_LOGI(PROJ_NAME, "Confirm new PIN");
                    pin_state = PIN_CHANGE_CONFIRM;
                    error_state = SUCCESS;
                    break;

                case PIN_CHANGE_CONFIRM:
                    ESP_ERROR_CHECK(check_pin(pin, "new_pin", &is_correct));
                    if(is_correct) {
                        ESP_LOGI(PROJ_NAME, "PIN change confirmed");
                        ESP_ERROR_CHECK(change_pin(pin, "access_pin"));
                        pin_state = PIN_AUTH;
                        gpio_set_level(DOOR_CLOSED_LED, GPIO_HIGH);
                        error_state = SUCCESS;
                    } else {
                        ESP_LOGI(PROJ_NAME, "PINs do not match, try again");
                        pin_state = PIN_CHANGE_ENTER_NEW;
                        error_state = FAIL;
                    }
                    break;
            }
            break;

        case KEYPAD_PIN_CHANGE_KEY:
            gpio_blink_nonblocking(DOOR_OPEN_LED, 20);
            ESP_LOGI(PROJ_NAME, "Requested pin change");
            ESP_LOGI(PROJ_NAME, "Enter admin PIN");
            pin_state = PIN_CHANGE_AUTH;
            break;

        default:
            gpio_blink_nonblocking(DOOR_OPEN_LED, 20);
            pin[pin_index++] = key_pressed;
            if(pin_index >= sizeof(pin)) {
                ESP_LOGE(PROJ_NAME, "PIN too long, resetting");
                error_state = FAIL;
                break;
            }
            return;
    }

    if(error_state == SUCCESS) {
        gpio_blink_twice_nonblocking(DOOR_OPEN_LED);
    } else if(error_state == FAIL) {
        wait_security_delay();
    }
    keypad_clear_pin(pin, &pin_index);
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

bool is_door_open()
{
    return door_state == DOOR_OPEN;
}

void door_open()
{
    ESP_LOGI(PROJ_NAME, "Opening door");
    gpio_set_level(DOOR_CLOSED_LED, GPIO_LOW);
    gpio_set_level(DOOR_OPEN_LED, GPIO_HIGH);
}

void door_close()
{
    gpio_set_level(DOOR_OPEN_LED, GPIO_LOW);
    gpio_set_level(DOOR_CLOSED_LED, GPIO_HIGH);
}

noreturn void door_open_for_defined_time_task()
{
    door_open();
    uint16_t duration;
    ESP_ERROR_CHECK(read_door_duration(&duration));
    vTaskDelaySec(duration); // Leave open for DOOR_OPEN_TIME_SEC seconds
    ESP_LOGI(PROJ_NAME, "Closing door");
    door_state = DOOR_CLOSE;
    door_close();
    vTaskDelete(NULL); // Delete self
    while(1); // Wait for deletion
}

noreturn void door_handler_task()
{
    door_evt_queue = xQueueCreate(1, sizeof(enum DoorState));
    if(door_evt_queue == NULL) {
        ESP_LOGE(PROJ_NAME, "Failed to create door event queue");
        abort();
    }

    enum DoorState evt;
    TaskHandle_t door_open_task_handle = NULL;

    while(1) {
        if(xQueueReceive(door_evt_queue, &evt, portMAX_DELAY)) {
            switch(evt) {
                case DOOR_OPEN:
                    if(door_state == DOOR_CLOSE) {
                        xTaskCreate(&door_open_for_defined_time_task, "door_open", 2048, NULL, tskIDLE_PRIORITY, &door_open_task_handle);
                        door_state = DOOR_OPEN;
                    } else {
                        ESP_LOGE(PROJ_NAME, "Door already open");
                    }
                    break;
                case DOOR_CLOSE:
                    if(door_state == DOOR_OPEN) {
                        ESP_LOGI(PROJ_NAME, "Closing door prematurely");
                        // Prevent race condition when both tasks reach deletion state
                        static portMUX_TYPE task_delete_spinlock = portMUX_INITIALIZER_UNLOCKED;
                        taskENTER_CRITICAL(&task_delete_spinlock);
                        if(eTaskGetState(door_open_task_handle) != eDeleted) {
                            vTaskDelete(door_open_task_handle);
                        }
                        taskEXIT_CRITICAL(&task_delete_spinlock);
                        door_close();
                    } else {
                        ESP_LOGE(PROJ_NAME, "Door already closed");
                    }
                    door_state = DOOR_CLOSE;
                    break;
            }
        }
    }
}
