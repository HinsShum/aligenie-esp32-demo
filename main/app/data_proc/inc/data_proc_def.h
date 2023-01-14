/**
 * @file main/app/data_proc/inc/data_proc_def.h
 *
 * Copyright (C) 2023
 *
 * data_proc_def.h is free software: you can redistribute it and/or modify
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
#ifndef __DATA_PROC_DEF_H
#define __DATA_PROC_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_proc.h"
#include "storage.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/* storage data proc structure definition
 */
typedef struct dp_storage *dp_storage_t;
struct dp_storage {
    enum {
        DP_STORAGE_TYPE_WIFI                            /*<< support sub pull and notify */
    } type;
    union {
        struct storage_wifi wifi;
    } param;
};

/* wifi sta data proc structure definition
 */
typedef struct dp_wifi_sta *dp_wifi_sta_t;
struct dp_wifi_sta {
    enum {
        DP_WIFI_STA_TYPE_NETWORK,                       /*<< support pub publish and sub pull */
        DP_WIFI_STA_TYPE_MAC,                           /*<< support sub pull and notify */
        DP_WIFI_STA_TYPE_DISCONNECT,                    /*<< only support notify */
        DP_WIFI_STA_TYPE_CONNECT                        /*<< only support notify */
    } type;
    union {
        struct wifi_sta_network {
            enum {
                WIFI_STA_DISCONNECTED,
                WIFI_STA_CONNECTED
            } state;
            struct {
                uint32_t ip;
                uint32_t gw;
                uint32_t netmask;
            } ipv4;
        } network;
        uint8_t mac[6];
        struct {
            char *ssid;
            char *passwd;
        } connect;
    } param;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __DATA_PROC_DEF_H */
