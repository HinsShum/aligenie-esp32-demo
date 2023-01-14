/**
 * @file main/app/data_proc/inc/data_proc.h
 *
 * Copyright (C) 2023
 *
 * data_proc.h is free software: you can redistribute it and/or modify
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
#ifndef __DATA_PROC_H
#define __DATA_PROC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "account.h"

/*---------- macro ----------*/
#define DATA_PROC_INIT_DEF(name)                void _data_proc_##name##_init(account_t account)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void data_proc_init(void);
extern data_center_t data_proc_get_center(void);

#ifdef __cplusplus
}
#endif
#endif /* __DATA_PROC_H */
