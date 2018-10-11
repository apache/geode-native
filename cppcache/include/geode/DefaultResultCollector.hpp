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

#ifndef GEODE_DEFAULTRESULTCOLLECTOR_H_
#define GEODE_DEFAULTRESULTCOLLECTOR_H_

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "CacheableBuiltins.hpp"
#include "ResultCollector.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class APACHE_GEODE_EXPORT DefaultResultCollector : public ResultCollector {
 public:
  DefaultResultCollector();
  ~DefaultResultCollector() noexcept override;

  std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  void addResult(
      const std::shared_ptr<Cacheable>& resultOfSingleExecution) override;

  void endResults() override;

  void clearResults() override;

 private:
  std::shared_ptr<CacheableVector> resultList;
  bool ready;
  std::condition_variable readyCondition;
  std::mutex readyMutex;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DEFAULTRESULTCOLLECTOR_H_
