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

#ifndef GEODE_CPPCACHELIBRARY_H_
#define GEODE_CPPCACHELIBRARY_H_

#include <string>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

// initialize GEODE runtime if it has not already been initialized.
class APACHE_GEODE_EXPORT CppCacheLibrary {
 public:
  /**
   * Call to this to trigger initialization.
   */
  static void initLib(void);

  /**
   * Call to this to trigger cleanup.  initLib and closeLib calls must be in
   * pairs.
   */
  static void closeLib(void);

  /**
   * Returns the directory where the library/DLL resides
   */
  static const std::string& getProductLibDir();

  static const std::string& getProductDir();

 private:
  static std::string initProductLibDir();

  static std::string initProductDir();
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CPPCACHELIBRARY_H_
