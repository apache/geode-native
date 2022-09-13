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

#include <geode/DefaultResultCollector.hpp>

namespace apache {
namespace geode {
namespace client {

DefaultResultCollector::DefaultResultCollector()
    : resultList(CacheableVector::create()), ready(false) {}

DefaultResultCollector::~DefaultResultCollector() noexcept {}
std::shared_ptr<CacheableVector> DefaultResultCollector::getResult(
    std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lk(readyMutex);
  if (readyCondition.wait_for(lk, timeout, [this] { return ready; })) {
    return resultList;
  }

  throw FunctionException(
      "Result is not ready, endResults callback is called before invoking "
      "getResult() method");
}

void DefaultResultCollector::addResult(
    const std::shared_ptr<Cacheable>& result) {
  resultList->push_back(result);
}

void DefaultResultCollector::endResults() {
  {
    std::lock_guard<std::mutex> lk(readyMutex);
    ready = true;
  }
  readyCondition.notify_all();
}

void DefaultResultCollector::clearResults() { resultList->clear(); }

}  // namespace client
}  // namespace geode
}  // namespace apache
