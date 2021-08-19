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

#ifndef APACHE_GEODE_C_REGION_FACTORY_H
#define APACHE_GEODE_C_REGION_FACTORY_H

#include "geode/internal/geode_base.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct apache_geode_cache_s;
typedef struct apache_geode_cache_s apache_geode_cache_t;

struct apache_geode_region_factory_s;
typedef struct apache_geode_region_factory_s apache_geode_region_factory_t;

struct apache_geode_region_s;
typedef struct apache_geode_region_s apache_geode_region_t;

APACHE_GEODE_C_EXPORT void apache_geode_DestroyRegionFactory(
    apache_geode_region_factory_t* regionFactory);

APACHE_GEODE_C_EXPORT void apache_geode_RegionFactory_SetPoolName(
    apache_geode_region_factory_t* regionFactory, const char* poolName);

APACHE_GEODE_C_EXPORT apache_geode_region_t*
apache_geode_RegionFactory_CreateRegion(
    apache_geode_region_factory_t* regionFactory, const char* regionName);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // APACHE_GEODE_C_REGION_FACTORY_H
