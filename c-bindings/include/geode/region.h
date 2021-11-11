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

#ifndef APACHE_GEODE_C_REGION_H
#define APACHE_GEODE_C_REGION_H

#include "geode/internal/geode_base.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct apache_geode_region_s;
typedef struct apache_geode_region_s apache_geode_region_t;

APACHE_GEODE_C_EXPORT void apache_geode_DestroyRegion(
    apache_geode_region_t* region);

APACHE_GEODE_C_EXPORT void apache_geode_Region_PutString(
    apache_geode_region_t* region, const char* key, const char* value);

APACHE_GEODE_C_EXPORT void apache_geode_Region_PutByteArray(
    apache_geode_region_t* region, const char* key, const char* value, size_t size);

APACHE_GEODE_C_EXPORT const char* apache_geode_Region_GetString(
    apache_geode_region_t* region, const char* key);

APACHE_GEODE_C_EXPORT void apache_geode_Region_GetByteArray(
    apache_geode_region_t* region, const char* key, char** value, size_t* size);

APACHE_GEODE_C_EXPORT void apache_geode_Region_Remove(
    apache_geode_region_t* region, const char* key);

APACHE_GEODE_C_EXPORT bool apache_geode_Region_ContainsValueForKey(
    apache_geode_region_t* region, const char* key);
    
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // APACHE_GEODE_C_REGION_H
