/**
 * @file components/wifi_sta_manager/wifi_sta_manager.c
 *
 * Copyright (C) 2023
 *
 * wifi_sta_manager.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "wifi_sta_manager.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "options.h"
#include <string.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
typedef enum {
    EVT_BITS_AIRKISS_RUNNING = BIT(0),
    EVT_BITS_NETWORK_CONNECTED = BIT(1),
    EVT_BITS_ACTIVE_DISCONNECT = BIT(2)
} evt_bits_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
TimerHandle_t airkiss_timer;
EventGroupHandle_t evt_handle;
static esp_netif_t *netif;
static wifi_sta_manager_event_callback_t wifi_evt_cb;
static char wifi_ssid[32];
static char wifi_password[64];

/*---------- function ----------*/
static inline bool __is_airkiss_running(void)
{
    return !!(xEventGroupGetBits(evt_handle) & EVT_BITS_AIRKISS_RUNNING);
}

static inline void __airkiss_stop(void)
{
    if(xEventGroupGetBits(evt_handle) & EVT_BITS_AIRKISS_RUNNING) {
        xEventGroupClearBits(evt_handle, EVT_BITS_AIRKISS_RUNNING);
        esp_smartconfig_stop();
    }
    if(xTimerIsTimerActive(airkiss_timer)) {
        xTimerStop(airkiss_timer, portMAX_DELAY);
    }
}

static inline void __airkiss_start(void)
{
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();

    if(__is_airkiss_running()) {
        __airkiss_stop();
    }
    xEventGroupSetBits(evt_handle, EVT_BITS_AIRKISS_RUNNING);
    esp_smartconfig_set_type(SC_TYPE_AIRKISS);
    esp_smartconfig_start(&cfg);
    xTimerStart(airkiss_timer, portMAX_DELAY);
}

static void __default_wifi_evt_cb(wifi_sta_evt_t evt, void *data)
{
}

static bool __wifi_sta_config(const char ssid[32], const char password[64])
{
    bool err = false;
    wifi_config_t cfg = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    do {
        if(ssid == NULL) {
            if(strlen(wifi_ssid) == 0) {
                /* no wifi ssid information stored */
                break;
            }
            memcpy(cfg.sta.ssid, wifi_ssid, ARRAY_SIZE(cfg.sta.ssid));
            memcpy(cfg.sta.password, wifi_password, ARRAY_SIZE(cfg.sta.password));
        } else {
            memcpy(wifi_ssid, ssid, ARRAY_SIZE(wifi_ssid));
            memcpy(cfg.sta.ssid, ssid, ARRAY_SIZE(cfg.sta.ssid));
            if(password) {
                memcpy(wifi_password, password, ARRAY_SIZE(wifi_password));
                memcpy(cfg.sta.password, password, ARRAY_SIZE(cfg.sta.password));
            } else {
                memset(wifi_password, 0, ARRAY_SIZE(wifi_password));
            }
        }
        esp_wifi_set_config(WIFI_IF_STA, &cfg);
        xEventGroupClearBits(evt_handle, EVT_BITS_ACTIVE_DISCONNECT);
        if(ESP_OK != esp_wifi_connect()) {
            break;
        }
        err = true;
    } while(0);

    return err;
}

static void __wifi_sta_scan_done(void)
{
    struct wifi_sta_scan scan = {0};

    do {
        esp_wifi_scan_get_ap_num(&scan.nums);
        if(!scan.nums) {
            break;
        }
        scan.ap_list = __malloc(scan.nums * sizeof(*scan.ap_list));
        if(!scan.ap_list) {
            break;
        }
        if(esp_wifi_scan_get_ap_records(&scan.nums, scan.ap_list) == ESP_OK) {
            wifi_evt_cb(WIFI_STA_EVT_SCAN, &scan);
        }
        __free(scan.ap_list);
    } while(0);
}

static void __wifi_on_event(void *args, esp_event_base_t base, int32_t evt, void *data)
{
    switch(evt) {
        case WIFI_EVENT_STA_START:
            wifi_evt_cb(WIFI_STA_EVT_START, NULL);
            break;
        case WIFI_EVENT_STA_STOP:
            wifi_evt_cb(WIFI_STA_EVT_STOP, NULL);
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(evt_handle, EVT_BITS_NETWORK_CONNECTED);
            if(__is_airkiss_running()) {
                break;
            }
            if(xEventGroupGetBits(evt_handle) & EVT_BITS_ACTIVE_DISCONNECT) {
                wifi_evt_cb(WIFI_STA_EVT_DISCONNECTED, NULL);
                break;
            }
            /* automatic re-connect */
            __wifi_sta_config(NULL, NULL);
            break;
        case WIFI_EVENT_STA_CONNECTED:
            xEventGroupSetBits(evt_handle, EVT_BITS_NETWORK_CONNECTED);
            xEventGroupClearBits(evt_handle, EVT_BITS_ACTIVE_DISCONNECT);
            wifi_evt_cb(WIFI_STA_EVT_CONNECTED, NULL);
            break;
        case WIFI_EVENT_SCAN_DONE:
            __wifi_sta_scan_done();
            break;
        default:
            break;
    }
}

static void __ip_on_event(void *args, esp_event_base_t base, int32_t evt, void *data)
{
    struct wifi_sta_ipv4 param = {0};
    esp_netif_ip_info_t *ipinfo = &((ip_event_got_ip_t *)data)->ip_info;

    switch(evt) {
        case IP_EVENT_STA_GOT_IP:
            param.ip = ntohl(ipinfo->ip.addr);
            param.gw = ntohl(ipinfo->gw.addr);
            param.netmask = ntohl(ipinfo->netmask.addr);
            wifi_evt_cb(WIFI_STA_EVT_GOT_IP, &param);
            break;
        case IP_EVENT_STA_LOST_IP:
            wifi_evt_cb(WIFI_STA_EVT_LOST_IP, NULL);
            break;
        default:
            break;
    }
}

static void __airkiss_got_ssid_pswd(smartconfig_event_got_ssid_pswd_t *data)
{
    struct wifi_sta_ssid_pswd param = {0};

    memcpy(param.ssid, data->ssid, ARRAY_SIZE(param.ssid));
    memcpy(param.pswd, data->password, ARRAY_SIZE(param.pswd));
    wifi_evt_cb(WIFI_STA_EVT_SSID_PSWD, &param);
    wifi_sta_manager_connect((const char *)data->ssid, (const char *)data->password);
}

static void __scan_on_event(void *args, esp_event_base_t base, int32_t evt, void *data)
{
    switch(evt) {
        case SC_EVENT_SCAN_DONE:
            break;
        case SC_EVENT_FOUND_CHANNEL:
            if(__is_airkiss_running()) {
                xTimerStart(airkiss_timer, portMAX_DELAY);
            }
            break;
        case SC_EVENT_SEND_ACK_DONE:
            __airkiss_stop();
            break;
        case SC_EVENT_GOT_SSID_PSWD:
            __airkiss_got_ssid_pswd((smartconfig_event_got_ssid_pswd_t *)data);
            break;
        default:
            break;
    }
}

static void _airkiss_timer_expired(TimerHandle_t timer)
{
    __airkiss_stop();
}

bool wifi_sta_manager_init(wifi_sta_manager_event_callback_t cb)
{
    bool err = false;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    evt_handle = xEventGroupCreate();
    assert(evt_handle);
    xEventGroupSetBits(evt_handle, EVT_BITS_ACTIVE_DISCONNECT);
    airkiss_timer = xTimerCreate("Airkiss", __ms2ticks(60000), pdFALSE, NULL, _airkiss_timer_expired);
    assert(airkiss_timer);
    /* create esp event loop */
    esp_event_loop_create_default();
    /* register wifi event handle */
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, __wifi_on_event, NULL);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, __ip_on_event, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, __scan_on_event, NULL);
    wifi_evt_cb = (cb ? cb : __default_wifi_evt_cb);
    /* configure wifi */
    do {
        if(ESP_OK != esp_netif_init()) {
            break;
        }
        netif = esp_netif_create_default_wifi_sta();
        if(!netif) {
            break;
        }
        if(ESP_OK != esp_wifi_init(&cfg)) {
            break;
        }
        if(ESP_OK != esp_wifi_set_mode(WIFI_MODE_STA)) {
            break;
        }
        err = true;
        wifi_evt_cb(WIFI_STA_EVT_READY, (void *)netif);
    } while(0);

    return err;
}

bool wifi_sta_manager_start(void)
{
    return (ESP_OK == esp_wifi_start());
}

bool wifi_sta_manager_stop(void)
{
    return (ESP_OK == esp_wifi_stop());
}

bool wifi_sta_manager_connect(const char ssid[32], const char password[64])
{
    bool err = false;

    do {
        if(!evt_handle) {
            break;
        }
        xEventGroupClearBits(evt_handle, EVT_BITS_ACTIVE_DISCONNECT);
        if(xEventGroupGetBits(evt_handle) & EVT_BITS_NETWORK_CONNECTED) {
            if(ESP_OK != esp_wifi_disconnect()) {
                break;
            }
            xEventGroupClearBits(evt_handle, EVT_BITS_NETWORK_CONNECTED);
        }
        err = __wifi_sta_config(ssid, password);
    } while(0);

    return err;
}

bool wifi_sta_manager_disconnect(void)
{
    bool err = false;

    do {
        if(!evt_handle) {
            break;
        }
        xEventGroupSetBits(evt_handle, EVT_BITS_ACTIVE_DISCONNECT);
        if(!(xEventGroupGetBits(evt_handle) & EVT_BITS_NETWORK_CONNECTED)) {
            err = true;
            break;
        }
        if(ESP_OK != esp_wifi_disconnect()) {
            break;
        }
        xEventGroupClearBits(evt_handle, EVT_BITS_NETWORK_CONNECTED);
        err = true;
    } while(0);

    return err;
}

bool wifi_sta_manager_airkiss_start(void)
{
    bool err = false;

    do {
        if(!evt_handle) {
            break;
        }
        if(xEventGroupGetBits(evt_handle) & EVT_BITS_NETWORK_CONNECTED) {
            xEventGroupClearBits(evt_handle, EVT_BITS_NETWORK_CONNECTED);
            if(ESP_OK != esp_wifi_disconnect()) {
                break;
            }
        }
        __airkiss_start();
        err = true;
    } while(0);

    return err;
}

void wifi_sta_manager_airkiss_stop(void)
{
    __airkiss_stop();
}

bool wifi_sta_manager_scan_start(const char *ssid)
{
    wifi_scan_config_t cfg = {
        .ssid = (uint8_t *)ssid
    };

    return (ESP_OK == esp_wifi_scan_start(&cfg, false));
}

bool wifi_sta_manager_scan_stop(void)
{
    return (ESP_OK == esp_wifi_scan_stop());
}
