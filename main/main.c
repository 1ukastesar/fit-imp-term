/*
 * @file main/main.c
 *
 * @proj imp-term
 * @brief Access terminal with ESP32 - IMP semestral project
 * @author Lukas Tesar <xtesar43@stud.fit.vut.cz>
 * @year 2024
*/

#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "config.h"
#include "gpio.h"
#include "keypad.h"
#include "main.h"
#include "common.h"
#include "gap.h"
#include "gatt_svc.h"
#include "heart_rate.h"
#include "led.h"

/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/* Library function declarations */
void ble_store_config_init(void);

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);

/* Private functions */
/*
 *  Stack event callback functions
 *      - on_stack_reset is called when host resets BLE stack due to errors
 *      - on_stack_sync is called when host has synced with controller
 */
static void on_stack_reset(int reason) {
    /* On reset, print reset reason to console */
    ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void) {
    /* On stack sync, do advertising initialization */
    adv_init();
}

static void nimble_host_config_init(void) {
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Store host configuration */
    ble_store_config_init();
}

static void nimble_host_task(void *param) {
    /* Task entry log */
    ESP_LOGI(TAG, "nimble host task has been started!");

    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

static void heart_rate_task(void *param) {
    /* Task entry log */
    ESP_LOGI(TAG, "heart rate task has been started!");

    /* Loop forever */
    while (1) {
        /* Update heart rate value every 1 second */
        update_heart_rate();
        ESP_LOGI(TAG, "heart rate updated to %d", get_heart_rate());

        /* Send heart rate indication if enabled */
        send_heart_rate_indication();

        /* Sleep */
        vTaskDelay(HEART_RATE_TASK_PERIOD);
    }

    /* Clean up at exit */
    vTaskDelete(NULL);
}

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

void app_main(void)
{
    // Initialization
    gpio_configure();
    nvs_configure();

    ESP_LOGI(PROJ_NAME, "Initialization complete");
    ESP_LOGI(PROJ_NAME, "Starting tasks...");

    // Create long-running tasks
    // Uncomment to enable heartbeat
    if(xTaskCreate(&led_heartbeat_task, "led_heartbeat", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create heartbeat task");
        abort();
    }
    if(xTaskCreate(&keypad_handler_task, "keypad_handler", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create keypad handler task");
        abort();
    }

    if(xTaskCreate(&door_handler_task, "door_handler", 2048, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE(PROJ_NAME, "Failed to create door handler task");
        abort();
    }

    int rc;
    esp_err_t ret;

    /* NimBLE stack initialization */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ",
                 ret);
        return;
    }

    /* GAP service initialization */
    rc = gap_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
        return;
    }

    /* GATT server initialization */
    rc = gatt_svc_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GATT server, error code: %d", rc);
        return;
    }

    /* NimBLE host configuration initialization */
    nimble_host_config_init();

    /* Start NimBLE host task thread and return */
    xTaskCreate(nimble_host_task, "NimBLE Host", 4*1024, NULL, 5, NULL);
    xTaskCreate(heart_rate_task, "Heart Rate", 4*1024, NULL, 5, NULL);
}
