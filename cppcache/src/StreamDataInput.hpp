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

#ifndef GEODE_STREAMDATAINPUT_H_
#define GEODE_STREAMDATAINPUT_H_

#include <chrono>

#include "Connector.hpp"
#include "geode/DataInput.hpp"

namespace apache {
namespace geode {
namespace client {

class Connector;

/**
 * Provides the same functionality as its parent class but
 * data is retrieved, instead of from a passed buffer,
 * from a socket connection.
 */
class APACHE_GEODE_EXPORT StreamDataInput : public DataInput {
 public:
  StreamDataInput(std::chrono::milliseconds timeout,
                  std::unique_ptr<Connector> connector, const CacheImpl* cache,
                  Pool* pool);

  ~StreamDataInput() override;

 protected:
  void _checkBufferSize(size_t size, int32_t /* line */) override {
    readDataIfNotAvailable(size);
  }

  void readDataIfNotAvailable(size_t size);

 private:
  std::unique_ptr<Connector> m_connector;
  std::chrono::microseconds m_remainingTimeBeforeTimeout;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STREAMDATAINPUT_H_
