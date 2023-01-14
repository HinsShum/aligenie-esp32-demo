/**
 * @file components/wifi_sta_manager/inc/wifi_sta_manager.h
 *
 * Copyright (C) 2023
 *
 * wifi_sta_manager.h is free software: you can redistribute it and/or modify
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
#ifndef __WIFI_STA_MANAGER_H
#define __WIFI_STA_MANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_wifi_types.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
typedef enum {
    WIFI_STA_EVT_NONE,              /*<< no wifi sta event */
    WIFI_STA_EVT_READY,             /*<< wifi sta init ok */
    WIFI_STA_EVT_DISCONNECTED,      /*<< wifi link down */
    WIFI_STA_EVT_CONNECTED,         /*<< wifi link up */
    WIFI_STA_EVT_START,             /*<< start wifi manager */
    WIFI_STA_EVT_STOP,              /*<< start wifi manager */
    WIFI_STA_EVT_GOT_IP,            /*<< got ipv4 */
    WIFI_STA_EVT_LOST_IP,           /*<< lost ipv4 */
    WIFI_STA_EVT_SCAN,              /*<< scan wifi ap */
    WIFI_STA_EVT_SSID_PSWD
} wifi_sta_evt_t;

typedef struct wifi_sta_scan *wifi_sta_scan_t;
struct wifi_sta_scan {
    uint16_t nums;
    wifi_ap_record_t *ap_list;
};

typedef struct wifi_sta_ipv4 *wifi_sta_ipv4_t;
struct wifi_sta_ipv4 {
    uint32_t ip;
    uint32_t gw;
    uint32_t netmask;
};

typedef struct wifi_sta_ssid_pswd *wifi_sta_ssid_pswd_t;
struct wifi_sta_ssid_pswd {
    char ssid[32];
    char pswd[64];
};

typedef void (*wifi_sta_manager_event_callback_t)(wifi_sta_evt_t ev, void *data);

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern bool wifi_sta_manager_init(wifi_sta_manager_event_callback_t cb);
extern bool wifi_sta_manager_start(void);
extern bool wifi_sta_manager_stop(void);
extern bool wifi_sta_manager_connect(const char ssid[32], const char password[64]);
extern bool wifi_sta_manager_disconnect(void);
extern bool wifi_sta_manager_airkiss_start(void);
extern void wifi_sta_manager_airkiss_stop(void);
extern bool wifi_sta_manager_scan_start(const char *ssid);
extern bool wifi_sta_manager_scan_stop(void);

#ifdef __cplusplus
}
#endif
#endif /* __WIFI_STA_MANAGER_H */
