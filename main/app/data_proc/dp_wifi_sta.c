/**
 * @file main/app/data_proc/dp_wifi_sta.c
 *
 * Copyright (C) 2023
 *
 * dp_wifi_sta.c is free software: you can redistribute it and/or modify
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
#include "data_proc_def.h"
#include "wifi_sta_manager.h"
#include "esp_netif.h"
#include "options.h"
#include <string.h>

/*---------- macro ----------*/
#define TAG                                         "DPWiFiSTA"

/*---------- type define ----------*/
enum {
    EVT_BITS_WIFI_STA_READY = BIT(0),
    EVT_BITS_WIFI_STA_CONNECTED = BIT(1)
} evt_bits_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void __wifi_event_ready(void *data);
static void __wifi_event_disconnected(void *data);
static void _wifi_event_got_ip(void *data);
static void __wifi_event_scan(void *data);
static void _wifi_event_got_ssid_passwd(void *data);
static int32_t _account_event_timer(struct account_event_param *param);
static int32_t __account_event_sub_pull_network(dp_wifi_sta_t dp);
static int32_t __account_event_sub_pull_mac(dp_wifi_sta_t dp);
static int32_t _account_event_sub_pull(struct account_event_param *param);
static int32_t __account_event_notify_mac(dp_wifi_sta_t dp);
static int32_t __account_event_notify_disconnect(dp_wifi_sta_t dp);
static int32_t __account_event_notify_connect(dp_wifi_sta_t dp);
static int32_t _account_event_notify(struct account_event_param *param);

/*---------- variable ----------*/
static EventGroupHandle_t evt_handle;
static esp_netif_t *netif;
static account_t act;
static struct protocol_callback wifi_evt_cbs[] = {
    {WIFI_STA_EVT_READY, __wifi_event_ready},
    {WIFI_STA_EVT_DISCONNECTED, __wifi_event_disconnected},
    {WIFI_STA_EVT_GOT_IP, _wifi_event_got_ip},
    {WIFI_STA_EVT_SCAN, __wifi_event_scan},
    {WIFI_STA_EVT_SSID_PSWD, _wifi_event_got_ssid_passwd}
};
static struct protocol_callback act_evt_sub_pull_cbs[] = {
    {DP_WIFI_STA_TYPE_NETWORK, __account_event_sub_pull_network},
    {DP_WIFI_STA_TYPE_MAC, __account_event_sub_pull_mac}
};
static struct protocol_callback act_evt_notify_cbs[] = {
    {DP_WIFI_STA_TYPE_MAC, __account_event_notify_mac},
    {DP_WIFI_STA_TYPE_DISCONNECT, __account_event_notify_disconnect},
    {DP_WIFI_STA_TYPE_CONNECT, __account_event_notify_connect}
};
static struct protocol_callback act_evt_cbs[] = {
    {ACCOUNT_EVENT_SUB_PULL, _account_event_sub_pull},
    {ACCOUNT_EVENT_NOTIFY, _account_event_notify},
    {ACCOUNT_EVENT_TIMER, _account_event_timer}
};

/*---------- function ----------*/
static void __wifi_event_ready(void *data)
{
    xlog_tag_info(TAG, "Start wifi sta manager\n");
    netif = (esp_netif_t *)data;
    if(true == wifi_sta_manager_start()) {
        xEventGroupSetBits(evt_handle, EVT_BITS_WIFI_STA_READY);
        /* start scan wifi ap */
        wifi_sta_manager_scan_start(NULL);
    }
}

static void __wifi_event_disconnected(void *data)
{
    struct wifi_sta_network publish = {0};

    xEventGroupClearBits(evt_handle, EVT_BITS_WIFI_STA_CONNECTED);
    /* publish wifi disconnected event to all subscribers */
    publish.state = WIFI_STA_DISCONNECTED;
    act->ops.commit(act, &publish, sizeof(publish));
    act->ops.publish(act);
}

static void _wifi_event_got_ip(void *data)
{
    wifi_sta_ipv4_t param = (wifi_sta_ipv4_t)data;
    struct wifi_sta_network publish = {0};

    xEventGroupSetBits(evt_handle, EVT_BITS_WIFI_STA_CONNECTED);
    /* stop scan */
    act->ops.set_timer_enable(act, false);
    /* publish wifi got ip event to all subscribers */
    publish.state = WIFI_STA_CONNECTED;
    publish.ipv4.ip = param->ip;
    publish.ipv4.gw = param->gw;
    publish.ipv4.netmask = param->netmask;
    act->ops.commit(act, &publish, sizeof(publish));
    act->ops.publish(act);
}

static void _wifi_event_got_ssid_passwd(void *data)
{
    wifi_sta_ssid_pswd_t param = (wifi_sta_ssid_pswd_t)data;
    struct dp_storage dp = {0};

    do {
        dp.type = DP_STORAGE_TYPE_WIFI;
        act->ops.pull(act, "storage", &dp, sizeof(dp));
        if(memcmp(dp.param.wifi.ssid, param->ssid, ARRAY_SIZE(param->ssid)) == 0 &&
                memcmp(dp.param.wifi.password, param->pswd, ARRAY_SIZE(param->pswd)) == 0) {
            /* ssid and password not change */
            break;
        }
        /* store new wifi ssid and password */
        memcpy(dp.param.wifi.ssid, param->ssid, ARRAY_SIZE(param->ssid));
        memcpy(dp.param.wifi.password, param->pswd, ARRAY_SIZE(param->pswd));
        act->ops.notify(act, "storage", &dp, sizeof(dp));
    } while(0);
}

static void __wifi_event_scan(void *data)
{
    wifi_sta_scan_t param = (wifi_sta_scan_t)data;
    struct dp_storage dp_storage = {0};
    bool connect = false;

    dp_storage.type = DP_STORAGE_TYPE_WIFI;
    act->ops.pull(act, "storage", &dp_storage, sizeof(dp_storage));
    for(uint32_t i = 0; i < param->nums; ++i) {
        xlog_tag_info(TAG, "[%s][rssi=%d]\n", param->ap_list[i].ssid, param->ap_list[i].rssi);
        if(strcmp((char *)dp_storage.param.wifi.ssid, (char *)param->ap_list[i].ssid) == 0 &&
                strlen((char *)dp_storage.param.wifi.ssid) == strlen((char *)param->ap_list[i].ssid)) {
            connect = true;
        }
    }
    if(connect) {
        wifi_sta_manager_connect((char *)dp_storage.param.wifi.ssid, (char *)dp_storage.param.wifi.password);
    }
}

static void _wifi_on_event(wifi_sta_evt_t ev, void *data)
{
    void (*cb)(void *) = NULL;

    cb = protocol_callback_find(ev, wifi_evt_cbs, ARRAY_SIZE(wifi_evt_cbs));
    if(cb) {
        cb(data);
    }
}

static int32_t _account_event_timer(struct account_event_param *param)
{
    if(xEventGroupGetBits(evt_handle) & EVT_BITS_WIFI_STA_READY) {
        /* start scan wifi ap */
        wifi_sta_manager_scan_start(NULL);
    }

    return ACCOUNT_ERR_NONE;
}

static int32_t __account_event_sub_pull_network(dp_wifi_sta_t dp)
{
    esp_netif_ip_info_t ipv4 = {0};
    enum account_error err = ACCOUNT_ERR_UNKNOW;

    if(netif) {
        if(xEventGroupGetBits(evt_handle) & EVT_BITS_WIFI_STA_CONNECTED) {
            dp->param.network.state = WIFI_STA_CONNECTED;
            esp_netif_get_ip_info(netif, &ipv4);
            dp->param.network.ipv4.ip = ntohl(ipv4.ip.addr);
            dp->param.network.ipv4.gw = ntohl(ipv4.gw.addr);
            dp->param.network.ipv4.netmask = ntohl(ipv4.netmask.addr);
        } else {
            dp->param.network.state = WIFI_STA_DISCONNECTED;
        }
        err = ACCOUNT_ERR_NONE;
    }

    return err;
}

static int32_t __account_event_sub_pull_mac(dp_wifi_sta_t dp)
{
    enum account_error err = ACCOUNT_ERR_UNKNOW;

    if(netif) {
        esp_netif_get_mac(netif, dp->param.mac);
        err = ACCOUNT_ERR_NONE;
    }

    return err;
}

static int32_t _account_event_sub_pull(struct account_event_param *param)
{
    enum account_error err = ACCOUNT_ERR_PARAM_ERROR;
    dp_wifi_sta_t dp = (dp_wifi_sta_t)param->data;
    int32_t (*cb)(dp_wifi_sta_t) = NULL;

    do {
        assert(dp);
        if(sizeof(*dp) != param->size) {
            xlog_tag_error(TAG, "Account(%s) sub pull event expect %d size not %d size\n",
                    param->recv->id, sizeof(*dp), param->size);
            err = ACCOUNT_ERR_SIZE_MISMATCH;
            break;
        }
        cb = protocol_callback_find(dp->type, act_evt_sub_pull_cbs, ARRAY_SIZE(act_evt_sub_pull_cbs));
        if(!cb) {
            break;
        }
        err = cb(dp);
    } while(0);

    return err;
}

static int32_t __account_event_notify_mac(dp_wifi_sta_t dp)
{
    enum account_error err = ACCOUNT_ERR_UNKNOW;

    if(netif) {
        esp_netif_set_mac(netif, dp->param.mac);
        err = ACCOUNT_ERR_NONE;
    }

    return err;
}

static int32_t __account_event_notify_disconnect(dp_wifi_sta_t dp)
{
    enum account_error err = ACCOUNT_ERR_UNKNOW;

    if(netif) {
        wifi_sta_manager_disconnect();
        err = ACCOUNT_ERR_NONE;
    }

    return err;
}

static int32_t __account_event_notify_connect(dp_wifi_sta_t dp)
{
    enum account_error err = ACCOUNT_ERR_UNKNOW;
    struct dp_storage dp_storage = {0};

    do {
        if(!netif) {
            break;
        }
        if(!dp->param.connect.ssid) {
            break;
        }
        dp_storage.type = DP_STORAGE_TYPE_WIFI;
        act->ops.pull(act, "storage", &dp_storage, sizeof(dp_storage));
        if(strcmp((char *)dp_storage.param.wifi.ssid, dp->param.connect.ssid) != 0 ||
                strlen((char *)dp_storage.param.wifi.ssid) != strlen(dp->param.connect.ssid) ||
                (dp->param.connect.passwd == NULL && strlen((char *)dp_storage.param.wifi.password) != 0) ||
                (dp->param.connect.passwd != NULL &&
                (strcmp((char *)dp_storage.param.wifi.password, dp->param.connect.passwd) != 0 ||
                strlen((char *)dp_storage.param.wifi.password) != strlen(dp->param.connect.passwd)))) {
            /* store new wifi ssid and password */
            strlcpy((char *)dp_storage.param.wifi.ssid, dp->param.connect.ssid,
                    ARRAY_SIZE(dp_storage.param.wifi.ssid));
            if(dp->param.connect.passwd == NULL) {
                memset(dp_storage.param.wifi.password, 0, ARRAY_SIZE(dp_storage.param.wifi.password));
            } else {
                strlcpy((char *)dp_storage.param.wifi.password, dp->param.connect.passwd,
                        ARRAY_SIZE(dp_storage.param.wifi.password));
            }
            act->ops.notify(act, "storage", &dp_storage, sizeof(dp_storage));
        }
        /* start scan wifi ap */
        wifi_sta_manager_scan_start(NULL);
        act->ops.set_timer_enable(act, true);
        err = ACCOUNT_ERR_NONE;
    } while(0);

    return err;
}

static int32_t _account_event_notify(struct account_event_param *param)
{
    enum account_error err = ACCOUNT_ERR_PARAM_ERROR;
    dp_wifi_sta_t dp = (dp_wifi_sta_t)param->data;
    int32_t (*cb)(dp_wifi_sta_t) = NULL;

    do {
        assert(dp);
        if(sizeof(*dp) != param->size) {
            xlog_tag_error(TAG, "Account(%s) notify event expet %d size not %d size\n",
                    param->recv->id, sizeof(*dp), param->size);
            err = ACCOUNT_ERR_SIZE_MISMATCH;
            break;
        }
        cb = protocol_callback_find(dp->type, act_evt_notify_cbs, ARRAY_SIZE(act_evt_notify_cbs));
        if(!cb) {
            break;
        }
        err = cb(dp);
    } while(0);

    return err;
}

static int32_t on_event(account_t account, struct account_event_param *param)
{
    enum account_error err = ACCOUNT_ERR_UNSUPPORTED_REQUEST;
    enum account_event evt = param->event;
    int32_t (*cb)(struct account_event_param *) = NULL;

    do {
        cb = protocol_callback_find(evt, act_evt_cbs, ARRAY_SIZE(act_evt_cbs));
        if(!cb) {
            break;
        }
        err = cb(param);
    } while(0);

    return err;
}

DATA_PROC_INIT_DEF(wifi_sta)
{
    /* create event group handle */
    evt_handle = xEventGroupCreate();
    assert(evt_handle);
    /* initialize wifi manager */
    wifi_sta_manager_init(_wifi_on_event);
    /* subscribe other accounts */
    account->ops.subscribe(account, "storage");
    /* register account event callback */
    account->ops.set_event_cb(account, on_event);
    /* enable account timer */
    account->ops.set_timer_period(account, __ms2ticks(30000));
    account->ops.set_timer_enable(account, true);
    act = account;
    /* say hi */
    xlog_tag_info(TAG, "Initialize successfully\n");
}
