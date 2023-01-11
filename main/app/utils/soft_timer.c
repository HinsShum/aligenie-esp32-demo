/**
 * @file app/utils/soft_timer.c
 *
 * Copyright (C) 2022
 *
 * soft_timer.c is free software: you can redistribute it and/or modify
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
#include "soft_timer.h"

/*---------- macro ----------*/
#define TAG                                 "SoftTimer"

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
timer_handle_t soft_timer_create(const char *name, soft_timer_mode_t mode, uint32_t period, void *user_data, timer_cb_t cb)
{
    timer_handle_t tcb = NULL;

    do {
        if(!period) {
            xlog_tag_error(TAG, "SoftTimer's period can not be zero\n");
            break;
        }
        if(mode > SFTIM_MODE_REPEAT) {
            xlog_tag_error(TAG, "SoftTimer's mode para format error\n");
            break;
        }
        tcb = (timer_handle_t)xTimerCreate(name, __ms2ticks(period),
                (UBaseType_t)mode, user_data, (TimerCallbackFunction_t)cb);
    } while(0);

    return tcb;
}

void soft_timer_destroy(timer_handle_t tcb)
{
    assert(tcb);
    xTimerDelete((TimerHandle_t)tcb, portMAX_DELAY);
}

void soft_timer_start(timer_handle_t tcb)
{
    assert(tcb);
    xTimerStart((TimerHandle_t)tcb, portMAX_DELAY);
}

void soft_timer_restart(timer_handle_t tcb)
{
    soft_timer_start(tcb);
}

void soft_timer_stop(timer_handle_t tcb)
{
    assert(tcb);
    xTimerStop((TimerHandle_t)tcb, portMAX_DELAY);
}

void soft_timer_change_period(timer_handle_t tcb, uint32_t period)
{
    assert(tcb);
    assert(period);
    xTimerChangePeriod((timer_handle_t)tcb, __ms2ticks(period), portMAX_DELAY);
}

void soft_timer_set_reload_mode(timer_handle_t tcb, soft_timer_mode_t mode)
{
    assert(tcb);
    vTimerSetReloadMode((TimerHandle_t)tcb, (UBaseType_t)mode);
}

bool soft_timer_is_active(timer_handle_t tcb)
{
    assert(tcb);

    return !!xTimerIsTimerActive((TimerHandle_t)tcb);
}

const char *soft_timer_get_name(timer_handle_t tcb)
{
    assert(tcb);

    return pcTimerGetName((TimerHandle_t)tcb);
}

soft_timer_mode_t soft_timer_get_reload_mode(timer_handle_t tcb)
{
    assert(tcb);

    return (soft_timer_mode_t)uxTimerGetReloadMode((TimerHandle_t)tcb);
}

uint32_t soft_timer_get_period(timer_handle_t tcb)
{
    assert(tcb);

    return __ticks2ms(xTimerGetPeriod((TimerHandle_t)tcb));
}

void *soft_timer_get_user_data(timer_handle_t tcb)
{
    assert(tcb);

    return pvTimerGetTimerID((TimerHandle_t)tcb);
}
