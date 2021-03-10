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

#ifndef GEODE_LRUENTRYPROPERTIES_H_
#define GEODE_LRUENTRYPROPERTIES_H_

#include <list>
#include <memory>

namespace apache {
namespace geode {
namespace client {

class MapEntryImpl;

/**
 * @brief This class encapsulates LRU specific properties for a LRUList node.
 */
class LRUEntryProperties {
 public:
  using list_iterator = std::list<std::shared_ptr<MapEntryImpl>>::iterator;

 public:
  inline LRUEntryProperties() : persistence_info_(nullptr) {}

  inline const std::shared_ptr<void>& persistence_info() const {
    return persistence_info_;
  }

  inline void persistence_info(const std::shared_ptr<void>& persistenceInfo) {
    persistence_info_ = persistenceInfo;
  }

  void iterator(list_iterator iter) { iter_ = iter; }

  list_iterator iterator() const { return iter_; }

 protected:
  // this constructor deliberately skips initializing any fields
  inline explicit LRUEntryProperties(bool) {}

 private:
  std::shared_ptr<void> persistence_info_;
  list_iterator iter_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUENTRYPROPERTIES_H_
