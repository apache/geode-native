#pragma once

#ifndef GEODE_REGIONCONFIG_H_
#define GEODE_REGIONCONFIG_H_

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

/**
 * @file
 */
// cacheRegion.h: interface for the cacheRegion class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <memory>
#include <string>

#include <geode/Properties.hpp>

namespace apache {
namespace geode {
namespace client {

class RegionConfig;

class RegionConfig {
 public:
  explicit RegionConfig(const std::string& capacity);

  uint64_t entries();
  void setConcurrency(const std::string& str);
  void setLru(const std::string& str);
  void setCaching(const std::string& str);
  uint8_t getConcurrency();
  uint64_t getLruEntriesLimit();
  bool getCaching();

 private:
  std::string m_capacity;
  std::string m_lruEntriesLimit;
  std::string m_concurrency;
  std::string m_caching;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REGIONCONFIG_H_
