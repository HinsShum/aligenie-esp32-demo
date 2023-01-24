/**
 * @file app/tasks/cmdline/task_cmdline.c
 *
 * Copyright (C) 2023
 *
 * task_cmdline.c is free software: you can redistribute it and/or modify
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
#include "tasks_def.h"
#include "inc/cmdline_def.h"
#include "options.h"
#include "esp_console.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static void *cmd_init_tables[] = {
    cmd_uname_init,
    cmd_iperf_init
};

/*---------- function ----------*/
static void _task(void *param)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_cfg = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

    /* set prompt */
    repl_cfg.prompt = "esp32>";
    esp_console_new_repl_uart(&uart_cfg, &repl_cfg, &repl);
    /* register console command */
    for(uint32_t i = 0; i < ARRAY_SIZE(cmd_init_tables); ++i) {
        if(cmd_init_tables[i]) {
            ((void (*)(void))cmd_init_tables[i])();
        }
    }
    /* start console */
    esp_console_start_repl(repl);
    /* delete self */
    vTaskDelete(NULL);
}

void task_cmdline_create(void)
{
    xTaskCreatePinnedToCore(_task,
            "CMDLine",
            CONFIG_TASK_CMDLINE_STACKSIZE,
            NULL,
            CONFIG_TASK_CMDLINE_PRIO,
            NULL,
            tskNO_AFFINITY);
}
