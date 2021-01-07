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

#ifndef GEODE_NORESULT_H_
#define GEODE_NORESULT_H_

#include <geode/ExceptionTypes.hpp>
#include <geode/ResultCollector.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

/**
 * A Special ResultCollector implementation. Functions having
 * {@link Function#hasResult()} false, this ResultCollector will be returned.
 * <br>
 * Calling getResult on this NoResult will throw
 * {@link FunctionException}
 *
 *
 */
class NoResult final : public ResultCollector {
 public:
  NoResult() = default;
  ~NoResult() final = default;

  void addResult(const std::shared_ptr<Cacheable>&) final {
    throw UnsupportedOperationException("can not add to NoResult");
  }

  void endResults() final {
    throw UnsupportedOperationException("can not close on NoResult");
  }

  inline std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds) final {
    throw FunctionExecutionException(
        "Cannot return any result, as Function.hasResult() is false");
  }

  inline void clearResults() final {
    throw UnsupportedOperationException("can not clear results on NoResult");
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_NORESULT_H_
