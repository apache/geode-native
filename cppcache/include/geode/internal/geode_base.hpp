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

// TODO shared_ptr - Consider making con/destructors public.
/*
 * Allows std::shared_ptr to access protected constructors and destructors.
 */
#if defined(__clang__)
#if defined(__apple_build_version__) && __apple_build_version__ >= 9020039
#define _GEODE_FRIEND_STD_SHARED_PTR(_T) \
  friend std::__1::__compressed_pair_elem<_T, 1, false>;
#else
#define _GEODE_FRIEND_STD_SHARED_PTR(_T)                               \
  friend std::__libcpp_compressed_pair_imp<std::allocator<_T>, _T, 1>; \
  friend std::__shared_ptr_emplace<_T, std::allocator<_T> >;           \
  friend std::default_delete<_T>;
#endif
#elif defined(__GNUC__) || defined(__SUNPRO_CC)
#define _GEODE_FRIEND_STD_SHARED_PTR(_T) friend __gnu_cxx::new_allocator<_T>;
#elif defined(_MSC_VER)
#if defined(_MANAGED)
#define _GEODE_FRIEND_STD_SHARED_PTR(_T) \
  friend std::_Ref_count_obj<_T>;        \
  friend std::_Ref_count<_T>;            \
  friend std::_Ptr_base<_T>;             \
  friend std::default_delete<_T>;        \
  friend std::shared_ptr<_T>;
#else
#define _GEODE_FRIEND_STD_SHARED_PTR(_T) friend std::_Ref_count_obj<_T>;
#endif
#else
#define _GEODE_FRIEND_STD_SHARED_PTR(_T)
#endif

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
