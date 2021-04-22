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

#ifndef GEODE_FWKLIB_FWKLOG_H_
#define GEODE_FWKLIB_FWKLOG_H_

// ----------------------------------------------------------------------------

#include <ace/ACE.h>
#include <ace/OS.h>
#include <ace/Task.h>

#include <geode/internal/geode_base.hpp>

#include "FwkException.hpp"

// kludge to compile on bar
#define _CPP_CMATH 1
#include <sstream>

// ----------------------------------------------------------------------------

namespace apache {
namespace geode {
namespace client {
namespace testframework {

const char* strnrchr(const char* str, const char tok, int32_t cnt);
const char* dirAndFile(const char* str);
void plog(const char* l, const char* s, const char* filename, int32_t lineno);
const char* getNodeName();

/* Macro for logging */
#ifdef DEBUG

#define FWKDEBUG(x)                                      \
  do {                                                   \
    std::ostringstream os;                               \
    os << x;                                             \
    plog("Debug", os.str().c_str(), __FILE__, __LINE__); \
  } while (0)

#else

#define FWKDEBUG(x)

#endif

#define FWKINFO(x)                                                       \
  do {                                                                   \
    std::ostringstream os;                                               \
    os << x;                                                             \
    apache::geode::client::testframework::plog("Info", os.str().c_str(), \
                                               __FILE__, __LINE__);      \
  } while (0)
#define FWKWARN(x)                                                       \
  do {                                                                   \
    std::ostringstream os;                                               \
    os << x;                                                             \
    apache::geode::client::testframework::plog("Warn", os.str().c_str(), \
                                               __FILE__, __LINE__);      \
  } while (0)
#define FWKERROR(x)                                                       \
  do {                                                                    \
    std::ostringstream os;                                                \
    os << x;                                                              \
    apache::geode::client::testframework::plog("Error", os.str().c_str(), \
                                               __FILE__, __LINE__);       \
  } while (0)
#define FWKSEVERE(x)                                                       \
  do {                                                                     \
    std::ostringstream os;                                                 \
    os << x;                                                               \
    apache::geode::client::testframework::plog("Severe", os.str().c_str(), \
                                               __FILE__, __LINE__);        \
  } while (0)
#define FWKEXCEPTION(x)                                              \
  do {                                                               \
    std::ostringstream os;                                           \
    os << x << " In file: " << __FILE__ << " at line: " << __LINE__; \
    throw apache::geode::client::testframework::FwkException(        \
        os.str().c_str());                                           \
  } while (0)

}  // namespace  testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FWKLIB_FWKLOG_H_
