/**
 * @file main/app/data_proc/data_proc.c
 *
 * Copyright (C) 2023
 *
 * data_proc.c is free software: you can redistribute it and/or modify
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
#include "data_proc.h"
#include "data_proc_def.h"
#include "data_center.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static struct data_center data_center;
#define DATA_PROC_DEF(name, buffer_size)            static struct account act_##name
#include "data_proc_list.inc"
#undef DATA_PROC_DEF

/*---------- function ----------*/
void data_proc_init(void)
{
    data_center_init(&data_center, "center");
#define DATA_PROC_DEF(name, buffer_size)                                \
        account_create(&act_##name, #name, &data_center, buffer_size, NULL)
#include "data_proc_list.inc"
#undef DATA_PROC_DEF

#define DATA_PROC_DEF(name, buffer_size)                                \
        do {                                                            \
            extern void _data_proc_##name##_init(account_t account);    \
            _data_proc_##name##_init(&act_##name);                      \
        } while(0)
#include "data_proc_list.inc"
#undef DATA_PROC_DEF
}

data_center_t data_proc_get_center(void)
{
    return &data_center;
}
