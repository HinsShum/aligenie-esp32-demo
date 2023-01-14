/**
 * @file app/storage/storage.c
 *
 * Copyright (C) 2023
 *
 * storage.c is free software: you can redistribute it and/or modify
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
#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "options.h"
#include <string.h>

/*---------- macro ----------*/
#define TAG                                         "Storage"

/*---------- type define ----------*/
struct storage {
    struct storage_wifi wifi;
};

struct storage_default_node {
    const char *key;
    void *data;
    uint32_t size;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static SemaphoreHandle_t mutex;
static nvs_handle_t nvs_storage;
/* storage cache */
static struct storage storage_default;
static struct storage storage_cache;
/* storage default */
static struct storage_default_node default_nodes[] = {
    {"wifi", &storage_default.wifi, sizeof(storage_default.wifi)}
};

/*---------- function ----------*/
static inline void _lock(void)
{
    if(mutex) {
        xSemaphoreTake(mutex, portMAX_DELAY);
    }
}

static inline void _unlock(void)
{
    if(mutex) {
        xSemaphoreGive(mutex);
    }
}

static inline void _nvs_init(void)
{
    esp_err_t res = ESP_FAIL;

    res = nvs_flash_init();
    if(res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

static inline void _storage_set_default(struct storage *db)
{
    /* clear wifi informations */
    memset(db->wifi.ssid, 0, ARRAY_SIZE(db->wifi.ssid));
    memset(db->wifi.password, 0, ARRAY_SIZE(db->wifi.password));
}

static bool _storage_init(const char *name, nvs_handle_t *nvs)
{
    bool err = true;

    _lock();
    nvs_open(name, NVS_READWRITE, nvs);
    if(ESP_OK != nvs_open(name, NVS_READWRITE, nvs)) {
        xlog_tag_error(TAG, "Open storage nvs(%s) failure\n", name);
        err = false;
    }
    _unlock();

    return err;
}

static void *__get_storage_default_node(const char *key, const struct storage_default_node *def,
        const uint16_t size)
{
    void *retval = NULL;

    for(uint16_t i = 0; i < size; ++i) {
        if(strncmp(def[i].key, key, strlen(key)) == 0 &&
                strlen(def[i].key) == strlen(key)) {
            retval = (void *)&def[i];
            break;
        }
    }

    return retval;
}

static inline void *__get_cache_member(const struct storage *def, const struct storage *cache,
        const void *member)
{
    return ((uint8_t *)member - (uint8_t *)def + (uint8_t *)cache);
}

static void __get_cache(nvs_handle_t nvs, const char *name,
        const struct storage_default_node *def, const uint16_t size)
{
    nvs_iterator_t iter = NULL;
    nvs_entry_info_t info = {0};
    struct storage_default_node *n = NULL;
    void *m = NULL;
    size_t bytes = 0;

    nvs_entry_find(NVS_DEFAULT_PART_NAME, name, NVS_TYPE_BLOB, &iter);
    while(iter) {
        nvs_entry_info(iter, &info);
        n = __get_storage_default_node(info.key, def, size);
        if(n) {
            m = __get_cache_member(&storage_default, &storage_cache, n->data);
            bytes = (size_t)n->size;
            nvs_get_blob(nvs, n->key, m, &bytes);
        }
        nvs_entry_next(&iter);
    }
}

static inline void _get_cache(void)
{
    __get_cache(nvs_storage, "storage", default_nodes, ARRAY_SIZE(default_nodes));
    /* print cache context */
    xlog_tag_message(TAG, "Storage:\n");
    xlog_tag_message(TAG, "%-4sWiFi ssid: %s\n", "", storage_cache.wifi.ssid);
    xlog_tag_message(TAG, "%-4sWiFi password: %s\n", "", storage_cache.wifi.password);
}

bool storage_init(void)
{
    bool err = false;

    do {
        _nvs_init();
        _storage_set_default(&storage_default);
        _storage_set_default(&storage_cache);
        mutex = xSemaphoreCreateMutex();
        assert(mutex);
        if(true != (err = _storage_init("storage", &nvs_storage))) {
            break;
        }
        /* read cache */
        _get_cache();
        xlog_tag_info(TAG, "Initialize successfully\n");
    } while(0);

    return err;
}

bool storage_restore(void)
{
    bool err = false;

    _lock();
    do {
        if(!nvs_storage) {
            break;
        }
        if(ESP_OK != nvs_erase_all(nvs_storage)) {
            break;
        }
        for(uint16_t i = 0; i < ARRAY_SIZE(default_nodes); ++i) {
            nvs_set_blob(nvs_storage, default_nodes[i].key, default_nodes[i].data, default_nodes[i].size);
        }
        if(ESP_OK != nvs_commit(nvs_storage)) {
            break;
        }
        _get_cache();
        err = true;
    } while(0);
    _unlock();

    return err;
}

bool storage_get(const char *key, void *value, uint32_t size)
{
    bool err = false;
    struct storage_default_node *n = NULL;
    void *m = NULL;

    _lock();
    do {
        assert(key);
        assert(value);
        assert(size);
        n = __get_storage_default_node(key, default_nodes, ARRAY_SIZE(default_nodes));
        if(!n) {
            break;
        }
        m = __get_cache_member(&storage_default, &storage_cache, n->data);
        if(size > n->size) {
            size = n->size;
        }
        memcpy(value, m, size);
        err = true;
    } while(0);
    _unlock();

    return err;
}

bool storage_set(const char *key, void *value, uint32_t size)
{
    bool err = false;
    struct storage_default_node *n = NULL;
    void *m = NULL;

    _lock();
    do {
        assert(key);
        assert(value);
        assert(size);
        n = __get_storage_default_node(key, default_nodes, ARRAY_SIZE(default_nodes));
        if(!n) {
            break;
        }
        m = __get_cache_member(&storage_default, &storage_cache, n->data);
        if(size > n->size) {
            break;
        }
        nvs_set_blob(nvs_storage, key, value, size);
        if(ESP_OK != nvs_commit(nvs_storage)) {
            break;
        }
        /* update cache */
        memcpy(m, value, size);
        err = true;
    } while(0);
    _unlock();

    return err;
}
