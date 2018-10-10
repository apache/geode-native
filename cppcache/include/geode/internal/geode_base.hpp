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

#ifndef GEODE_BASE_H_
#define GEODE_BASE_H_

#include "apache-geode_export.h"

/**@namespace geode This namespace contains all the Geode
 * C++ API classes, enumerations and globals.
 */

/**@namespace geode_statistics This namespace contains all the Geode
 * C++ statistics API classes.
 */

/**@namespace apache::geode::client::TypeHelper This namespace contains type
 * traits helper
 * structs/classes to determine type information at compile time
 * using typename. Useful for templates in particular.
 */

/**
 * @file
 *
 *  Definitions of types and functions supported in the Geode C++ interface
 */

/** Allocates x and throws OutOfMemoryException if it fails */
#define _GEODE_NEW(v, stmt)                                             \
  {                                                                     \
    try {                                                               \
      v = new stmt;                                                     \
    } catch (const std::bad_alloc &) {                                  \
      throw apache::geode::client::OutOfMemoryException(                \
          "Out of Memory while executing \"" #v " = new " #stmt ";\""); \
    }                                                                   \
  }

/** Deletes x only if it exists */
#define _GEODE_SAFE_DELETE(x) \
  {                           \
    delete x;                 \
    x = nullptr;              \
  }

/** Deletes array x only if it exists */
#define _GEODE_SAFE_DELETE_ARRAY(x) \
  {                                 \
    delete[] x;                     \
    x = nullptr;                    \
  }

#include <chrono>
#include <string>

namespace apache {
namespace geode {
namespace client {

constexpr static std::chrono::milliseconds DEFAULT_QUERY_RESPONSE_TIMEOUT =
    std::chrono::seconds{15};

static const std::string EMPTY_STRING{};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_BASE_H_
