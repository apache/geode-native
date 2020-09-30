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

#ifndef GEODE_RESULTCOLLECTOR_H_
#define GEODE_RESULTCOLLECTOR_H_

#include <chrono>
#include <memory>

#include "CacheableBuiltins.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class ResultCollector ResultCollector.hpp
 * Defines the interface for a container that gathers results from function
 * execution.<br>
 * Geode provides a default implementation for ResultCollector.
 * Applications can choose to implement their own custom ResultCollector.
 * A custom ResultCollector facilitates result sorting or aggregation.
 * Aggregation
 * functions like sum, minimum, maximum and average can also be applied to the
 * result using
 * a custom ResultCollector.
 *  Example:
 *  <br>
 *  <pre>
 * auto rc = FunctionService::onRegion(region)
 *                                      .withArgs(args)
 *                                      .withFilter(keySet)
 *                                      .withCollector(new
 * MyCustomResultCollector())
 *                                      .execute(Function);
 *  //Application can do something else here before retrieving the result
 * auto functionResult = rc.getResult();
 * </pre>
 *
 * @see FunctionService
 */

class APACHE_GEODE_EXPORT ResultCollector {
  /**
   * @brief public methods
   */
 public:
  ResultCollector() = default;
  virtual ~ResultCollector() noexcept = default;

  /**
   * Returns the result of function execution, potentially blocking until all
   * the results are available.
   * If geode sendException is called then {@link ResultCollector.getResult}
   * will not
   * throw exception but will have exception {@link
   * UserFunctionExecutionException} as a part of results received.
   * @param timeout if result is not ready within this time,
   * exception will be thrown
   * @return the result
   * @throws FunctionException if result retrieval fails
   * @see UserFunctionExecutionException
   */
  virtual std::shared_ptr<CacheableVector> getResult(
      std::chrono::milliseconds timeout = DEFAULT_QUERY_RESPONSE_TIMEOUT) = 0;

  /**
   * Adds a single function execution result to the ResultCollector
   *
   * @param resultOfSingleExecution single function execution result to add
   * @since 5.8LA
   */
  virtual void addResult(
      const std::shared_ptr<Cacheable>& resultOfSingleExecution) = 0;

  /**
   * Geode will invoke this method when function execution has completed
   * and all results for the execution have been obtained and  added to the
   * ResultCollector}
   */
  virtual void endResults() = 0;

  /**
   * Geode will invoke this method before re-executing function (in case of
   * Function Execution HA) This is to clear the previous execution results from
   * the result collector
   * @since 6.5
   */
  virtual void clearResults() = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_RESULTCOLLECTOR_H_
