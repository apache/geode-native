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

#ifndef GEODE_TOMBSTONEENTRY_H_
#define GEODE_TOMBSTONEENTRY_H_

#include <chrono>
#include <memory>

#include "ExpiryTask.hpp"
#include "MapEntry.hpp"

namespace apache {
namespace geode {
namespace client {

class TombstoneEntry {
 public:
  explicit TombstoneEntry(std::shared_ptr<MapEntryImpl> entry)
      : entry_(std::move(entry)), task_id_(ExpiryTask::invalid()) {}

  std::shared_ptr<MapEntryImpl> entry() { return entry_; }
  ExpiryTask::id_t task_id() { return task_id_; }
  void task_id(ExpiryTask::id_t task_id) { task_id_ = task_id; }

  void invalidate() { valid_ = false; }
  bool valid() const { return valid_; }

 protected:
  std::shared_ptr<MapEntryImpl> entry_;
  ExpiryTask::id_t task_id_;
  bool valid_{true};
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TOMBSTONEENTRY_H_
