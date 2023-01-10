/**
 * @file main/conifg/options.h
 *
 * Copyright (C) 2023
 *
 * options.h is free software: you can redistribute it and/or modify
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
#ifndef __BOARD_OPTIONS_H
#define __BOARD_OPTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
/* freertos */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
/* misc */
#include "xlog.h"
#include "misc.h"
#include "esp_err.h"
/* standard */
#include <unistd.h>

/*---------- macro ----------*/
/* system api interface start with "__"
 */
#define __delay_ms(ms)                              (vTaskDelay(pdMS_TO_TICKS(ms)))
#define __delay_us(us)                              (usleep(us))
#define __get_ticks()                               (xTaskGetTickCount())
#define __get_ticks_from_isr()                      (xTaskGetTickCountFromISR())
#define __reset_system()                            (esp_restart())
#define __enter_critical()                          (taskENTER_CRITICAL())
#define __enter_critical_from_isr()                 (taskENTER_CRITICAL_FROM_ISR())
#define __exit_critical()                           (taskEXIT_CRITICAL())
#define __exit_critical_from_isr()                  (taskEXIT_CRITICAL_FROM_ISR())
#define __malloc(size)                              (pvPortMalloc(size))
#define __free(ptr)                                 (vPortFree(ptr))
#define __ms2ticks(ms)                              (pdMS_TO_TICKS(ms))
#define __ticks2ms(ticks)                           ((ticks * (TickType_t)1000U) / (TickType_t)configTICK_RATE_HZ)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __BOARD_OPTIONS_H */
