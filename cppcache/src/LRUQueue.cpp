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

#include "LRUQueue.hpp"

#include <mutex>

#include "LRUEntryProperties.hpp"
#include "MapEntryImpl.hpp"

namespace apache {
namespace geode {
namespace client {

LRUQueue::~LRUQueue() { clear(); }

void LRUQueue::push(const std::shared_ptr<MapEntryImpl>& entry) {
  auto end = container_.end();
  auto& properties = entry->getLRUProperties();

  std::unique_lock<mutex> lock{mutex_};
  container_.push_back(entry);
  properties.iterator(--end);
}

LRUQueue::type LRUQueue::pop() {
  auto end = container_.end();
  std::unique_lock<mutex> lock{mutex_};

  if (container_.empty()) {
    return {};
  }

  auto result = container_.front();
  result->getLRUProperties().iterator(end);
  container_.pop_front();
  return result;
}

void LRUQueue::remove(const type& entry) {
  auto end = container_.end();
  auto& properties = entry->getLRUProperties();
  std::unique_lock<mutex> lock{mutex_};

  auto iter = properties.iterator();
  if (iter != end) {
    container_.erase(iter);
    properties.iterator(end);
  }
}

void LRUQueue::move_to_end(const type& entry) {
  auto end = container_.end();
  auto& properties = entry->getLRUProperties();

  std::unique_lock<mutex> lock{mutex_};
  container_.splice(end, container_, properties.iterator());
  properties.iterator(--end);
}

void LRUQueue::clear() {
  auto end = container_.end();
  std::unique_lock<mutex> lock{mutex_};
  while (!container_.empty()) {
    auto iter = container_.begin();
    auto& properties = (*iter)->getLRUProperties();
    properties.iterator(end);
    container_.erase(iter);
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
