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

#ifndef GEODE_PDXUNREADDATA_H_
#define GEODE_PDXUNREADDATA_H_

#include <vector>

#include <geode/PdxUnreadFields.hpp>

#include "ExpiryTaskManager.hpp"

namespace apache {
namespace geode {
namespace client {

class PdxType;
class PdxReaderImpl;
class PdxWriterImpl;

class PdxUnreadData : public PdxUnreadFields {
 private:
  using time_point_t = std::chrono::steady_clock::time_point;

 public:
  PdxUnreadData();
  PdxUnreadData(std::shared_ptr<PdxType> pdxType, std::vector<int32_t> indexes,
                std::vector<std::vector<uint8_t>> data);

  ~PdxUnreadData() noexcept override = default;

  void taskId(ExpiryTask::id_t id) { expiryTaskId_ = id; }

  ExpiryTask::id_t taskId() { return expiryTaskId_; }

  time_point_t expiresAt() const { return expiresAt_; }
  void expiresAt(const time_point_t& tp) { expiresAt_ = tp; }

  inline std::vector<uint8_t> getPreservedData(int32_t idx) {
    return data_[idx];
  }

  void write(PdxWriterImpl& writer);

 private:
  std::shared_ptr<PdxType> pdxType_;

  std::vector<int32_t> indexes_;
  std::vector<std::vector<uint8_t>> data_;

  ExpiryTask::id_t expiryTaskId_;
  time_point_t expiresAt_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXUNREADDATA_H_
