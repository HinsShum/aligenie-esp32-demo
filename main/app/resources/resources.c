/**
 * @file main/app/resources/resources.c
 *
 * Copyright (C) 2023
 *
 * resources.c is free software: you can redistribute it and/or modify
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
#include "resources.h"
#include "resource_manager.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static resource_manager_base_t resources;

/*---------- function ----------*/
void resources_init(void)
{
    resources_deinit();
    resources = resource_manager_create();
}

void resources_deinit(void)
{
    if(resources) {
        resource_manager_destroy(resources);
        resources = NULL;
    }
}

void *resources_get(const char *name)
{
    void *handle = NULL;

    if(resources) {
        handle = resources->get_resource_careful(resources, name);
    }

    return handle;
}

bool resources_set(const char *name, void *handle)
{
    bool retval = false;

    if(resources) {
        retval = resources->add_resource(resources, name, handle);
    }

    return retval;
}
