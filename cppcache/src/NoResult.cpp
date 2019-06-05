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

#include "NoResult.hpp"

namespace apache {
namespace geode {
namespace client {
NoResult::NoResult() {}

NoResult::~NoResult() {}

void NoResult::addResult(const std::shared_ptr<Cacheable>&) {
throw UnsupportedOperationException("can not add to NoResult");
}

void NoResult::endResults() {
throw UnsupportedOperationException("can not close on NoResult");
}

std::shared_ptr<CacheableVector> NoResult::getResult(
    std::chrono::milliseconds) {
throw FunctionExecutionException(
    "Cannot return any result, as Function.hasResult() is false");
}

void NoResult::clearResults() {
throw UnsupportedOperationException("can not clear results on NoResult");
}

}  // namespace client
}  // namespace geode
}  // namespace apache
