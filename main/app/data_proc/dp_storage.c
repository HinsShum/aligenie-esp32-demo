/**
 * @file app/data_proc/dp_storage.c
 *
 * Copyright (C) 2023
 *
 * dp_storage.c is free software: you can redistribute it and/or modify
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
#include "storage.h"

/*---------- macro ----------*/
#define TAG                                         "DPStorage"

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t __act_evt_sub_pull_wifi(dp_storage_t dp);
static int32_t _act_evt_sub_pull(struct account_event_param *param);
static int32_t __act_evt_notify_wifi(dp_storage_t dp);
static int32_t _act_evt_notify(struct account_event_param *param);

/*---------- variable ----------*/
static struct protocol_callback evt_sub_pull_cbs[] = {
    {DP_STORAGE_TYPE_WIFI, __act_evt_sub_pull_wifi}
};
static struct protocol_callback evt_notify_cbs[] = {
    {DP_STORAGE_TYPE_WIFI, __act_evt_notify_wifi}
};
static struct protocol_callback evt_cbs[] = {
    {ACCOUNT_EVENT_SUB_PULL, _act_evt_sub_pull},
    {ACCOUNT_EVENT_NOTIFY, _act_evt_notify}
};

/*---------- function ----------*/
static int32_t __act_evt_notify_wifi(dp_storage_t dp)
{
    enum account_error err = ACCOUNT_ERR_NONE;

    if(true != storage_set("wifi", &dp->param.wifi, sizeof(dp->param.wifi))) {
        err = ACCOUNT_ERR_UNKNOW;
    }

    return err;
}

static int32_t _act_evt_notify(struct account_event_param *param)
{
    enum account_error err = ACCOUNT_ERR_PARAM_ERROR;
    dp_storage_t dp = (dp_storage_t)param->data;
    int32_t (*cb)(dp_storage_t) = NULL;

    do {
        assert(dp);
        if(sizeof(*dp) != param->size) {
            xlog_tag_error(TAG, "Account(%s) sub pull event expect %d size not %d size\n",
                    param->recv->id, sizeof(*dp), param->size);
            err = ACCOUNT_ERR_SIZE_MISMATCH;
            break;
        }
        cb = protocol_callback_find(dp->type, evt_notify_cbs, ARRAY_SIZE(evt_notify_cbs));
        if(!cb) {
            err = ACCOUNT_ERR_UNSUPPORTED_REQUEST;
            break;
        }
        err = cb(dp);
    } while(0);

    return err;
}

static int32_t __act_evt_sub_pull_wifi(dp_storage_t dp)
{
    enum account_error err = ACCOUNT_ERR_NONE;

    if(true != storage_get("wifi", &dp->param.wifi, sizeof(dp->param.wifi))) {
        err = ACCOUNT_ERR_UNKNOW;
    }

    return err;
}

static int32_t _act_evt_sub_pull(struct account_event_param *param)
{
    enum account_error err = ACCOUNT_ERR_PARAM_ERROR;
    dp_storage_t dp = (dp_storage_t)param->data;
    int32_t (*cb)(dp_storage_t) = NULL;

    do {
        assert(dp);
        if(sizeof(*dp) != param->size) {
            xlog_tag_error(TAG, "Account(%s) sub pull event expect %d size not %d size\n",
                    param->recv->id, sizeof(*dp), param->size);
            err = ACCOUNT_ERR_SIZE_MISMATCH;
            break;
        }
        cb = protocol_callback_find(dp->type, evt_sub_pull_cbs, ARRAY_SIZE(evt_sub_pull_cbs));
        if(!cb) {
            err = ACCOUNT_ERR_UNSUPPORTED_REQUEST;
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
        cb = protocol_callback_find(evt, evt_cbs, ARRAY_SIZE(evt_cbs));
        if(!cb) {
            break;
        }
        err = cb(param);
    } while(0);

    return err;
}

DATA_PROC_INIT_DEF(storage)
{
    /* initialize storage */
    storage_init();
    /* register account event callback */
    account->ops.set_event_cb(account, on_event);
    /* say hi */
    xlog_tag_info(TAG, "Initialize successfully\n");
}
