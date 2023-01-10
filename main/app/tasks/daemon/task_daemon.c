/**
 * @file main/app/tasks/daemon/task_daemon.c
 *
 * Copyright (C) 2023
 *
 * task_daemon.c is free software: you can redistribute it and/or modify
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
#include "options.h"
#include "nvs_flash.h"

/*---------- macro ----------*/
#define TAG                                         "Daemon"

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static SemaphoreHandle_t xlog_buf_mutex;
static SemaphoreHandle_t xlog_console_mutex;

/*---------- function ----------*/
static void __xlog_buf_lock(void)
{
    if(xlog_buf_mutex) {
        xSemaphoreTake(xlog_buf_mutex, portMAX_DELAY);
    }
}

static void __xlog_buf_unlock(void)
{
    if(xlog_buf_mutex) {
        xSemaphoreGive(xlog_buf_mutex);
    }
}

static bool __xlog_acquire_console(void)
{
    if(xlog_console_mutex) {
        xSemaphoreTake(xlog_console_mutex, portMAX_DELAY);
    }

    return true;
}

static void __xlog_release_console(void)
{
    if(xlog_console_mutex) {
        xSemaphoreGive(xlog_console_mutex);
    }
}

static void __xlog_print(const char *str, uint32_t length)
{
    for(uint32_t i = 0; i < length; ++i) {
        printf("%c", str[i]);
    }
}

static inline void __xlog_init(void)
{
    xlog_ops_t ops = {0};

    xlog_buf_mutex = xSemaphoreCreateMutex();
    assert(xlog_buf_mutex);
    xlog_console_mutex = xSemaphoreCreateMutex();
    assert(xlog_console_mutex);
    /* set xlog ops */
    ops.lock = __xlog_buf_lock;
    ops.unlock = __xlog_buf_unlock;
    ops.acquire_console = __xlog_acquire_console;
    ops.release_console = __xlog_release_console;
    ops.print = __xlog_print;
    xlog_init(&ops);
}

static void _init(void)
{
    /* initialize xlog */
    __xlog_init();
    /* say hi */
    xlog_tag_info(TAG, "Initialize successfully\n");
}

void app_main(void)
{
    _init();
    for(;;) {
        __delay_ms(5000);
        xlog_tag_info(TAG, "Hello, AliGenie\n");
    }
}
