/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifndef APACHE_GEODE_C_CACHE_FACTORY_H
#define APACHE_GEODE_C_CACHE_FACTORY_H

#include "geode/internal/geode_base.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct apache_geode_client_s;
typedef struct apache_geode_client_s apache_geode_client_t;

struct apache_geode_cache_factory_s;
typedef struct apache_geode_cache_factory_s apache_geode_cache_factory_t;

struct apache_geode_cache_s;
typedef struct apache_geode_cache_s apache_geode_cache_t;

struct apache_geode_properties_s;
typedef struct apache_geode_properties_s apache_geode_properties_t;

APACHE_GEODE_C_EXPORT apache_geode_cache_factory_t*
apache_geode_CreateCacheFactory(apache_geode_client_t *);

APACHE_GEODE_C_EXPORT apache_geode_cache_t* apache_geode_CacheFactory_CreateCache(
    apache_geode_cache_factory_t* factory);

APACHE_GEODE_C_EXPORT const char* apache_geode_CacheFactory_GetVersion(
    apache_geode_cache_factory_t* factory);

APACHE_GEODE_C_EXPORT const char* apache_geode_CacheFactory_GetProductDescription(
    apache_geode_cache_factory_t* factory);

APACHE_GEODE_C_EXPORT void apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(
    apache_geode_cache_factory_t* factory, bool pdxIgnoreUnreadFields);

APACHE_GEODE_C_EXPORT void apache_geode_CacheFactory_SetAuthInitialize(
    apache_geode_cache_factory_t* factory,
    void (*getCredentials)(apache_geode_properties_t*), void (*close)());

APACHE_GEODE_C_EXPORT void apache_geode_CacheFactory_SetPdxReadSerialized(
    apache_geode_cache_factory_t* factory, bool pdxReadSerialized);

APACHE_GEODE_C_EXPORT void apache_geode_CacheFactory_SetProperty(
    apache_geode_cache_factory_t* factory, const char* key, const char* value);

APACHE_GEODE_C_EXPORT void apache_geode_DestroyCacheFactory(
    apache_geode_cache_factory_t* factory);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // APACHE_GEODE_C_CACHE_FACTORY_H
