/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"
#include "config.h"
#include "gpio.h"
#include "keypad.h"

/* Private function declarations */
static int access_pin_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);

/* Automation IO service */
static const ble_uuid16_t auto_io_svc_uuid = BLE_UUID16_INIT(0x1815);

/* Access PIN characteristics */
static uint16_t access_pin_chr_val_handle;
static const ble_uuid128_t access_pin_chr_uuid =
    BLE_UUID128_INIT(0x21, 0xc9, 0xa1, 0x6b, 0x7f, 0x9b, 0xd4, 0xbe, 0x5e, 0x42,
                     0x62, 0x5b, 0xdc, 0x36, 0x60, 0xbf);

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /* Automation IO service */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &auto_io_svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){/* Access PIN characteristic */
                                        {.uuid = &access_pin_chr_uuid.u,
                                         .access_cb = access_pin_chr_access,
                                         .flags = BLE_GATT_CHR_F_WRITE,
                                         .val_handle = &access_pin_chr_val_handle},
                                        {0}},
    },

    {
        0, /* No more services. */
    },
};

static int access_pin_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
    /* Handle access events */
    /* Note: Access PIN characteristic is write only */
    switch (ctxt->op) {

    /* Write characteristic event */
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(GATT_TAG, "characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(GATT_TAG,
                     "characteristic write by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == access_pin_chr_val_handle) {
            /* Check if door is open */
            if(!door_is_open()) {
                return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
            }
            /* Verify access buffer length */
            if (ctxt->om->om_len >= KEYPAD_PIN_MIN_LEN && ctxt->om->om_len <= KEYPAD_PIN_MAX_LEN) {
                /* Update access PIN */
                char pin[KEYPAD_PIN_MAX_LEN + 1] = {0};
                memcpy(pin, ctxt->om->om_data, sizeof(pin));
                pin[ctxt->om->om_len] = '\0';
                ESP_ERROR_CHECK(write_pin((const char *) pin, "access_pin"));
                return ESP_OK;
            } else {
                goto error;
            }
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(GATT_TAG,
             "unexpected access operation to access pin characteristic, opcode: %d",
             ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(GATT_TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(GATT_TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(GATT_TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
