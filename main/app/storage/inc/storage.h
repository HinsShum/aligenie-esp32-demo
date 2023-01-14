/**
 * @file app/storage/inc/storage.h
 *
 * Copyright (C) 2023
 *
 * storage.h is free software: you can redistribute it and/or modify
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
#ifndef __STORAGE_H
#define __STORAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
struct storage_wifi {
    uint8_t ssid[33];
    uint8_t password[65];
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern bool storage_init(void);
extern bool storage_restore(void);
extern bool storage_get(const char *key, void *value, uint32_t size);
extern bool storage_set(const char *key, void *value, uint32_t size);

#ifdef __cplusplus
}
#endif
#endif /* __STORAGE_H */
