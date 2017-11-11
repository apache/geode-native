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

#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "geode_globals.hpp"
#include "geode_types.hpp"

#include "VectorT.hpp"
#include "CacheableBuiltins.hpp"
#include "ResultCollector.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class CPPCACHE_EXPORT DefaultResultCollector : public ResultCollector {
 public:
  DefaultResultCollector();
  virtual ~DefaultResultCollector() noexcept;

  virtual CacheableVectorPtr getResult(
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  virtual void addResult(const CacheablePtr& resultOfSingleExecution) override;

  virtual void endResults() override;

  virtual void clearResults() override;

 private:
  CacheableVectorPtr resultList;
  bool ready;
  std::condition_variable readyCondition;
  std::mutex readyMutex;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_DEFAULTRESULTCOLLECTOR_H_
