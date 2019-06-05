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

#include <atomic>
#include <memory>
#include <stdint.h>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {
/**
 * @brief This class encapsulates LRU specific properties for a LRUList node.
 */
class APACHE_GEODE_EXPORT LRUEntryProperties {
 public:
  LRUEntryProperties();

  void setRecentlyUsed();

  void clearRecentlyUsed();

  bool testRecentlyUsed() const;

  bool testEvicted() const;

  void setEvicted();

  void clearEvicted();

  const std::shared_ptr<void>& getPersistenceInfo() const;

  void setPersistenceInfo(const std::shared_ptr<void>& persistenceInfo);

 protected:
  // this constructor deliberately skips initializing any fields
   LRUEntryProperties(bool);

 private:
  std::atomic<uint32_t> m_bits;
  std::shared_ptr<void> m_persistenceInfo;
};
}  // namespace client
}  // namespace geode
}  // namespace apache
