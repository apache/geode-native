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

#include <geode/Execution.hpp>

#include "ExecutionImpl.hpp"

namespace apache {
namespace geode {
namespace client {

Execution::Execution() = default;

Execution::~Execution() noexcept = default;

Execution::Execution(Execution&& move) noexcept = default;

Execution& Execution::operator=(Execution&& move) noexcept = default;

Execution::Execution(std::unique_ptr<ExecutionImpl> impl)
    : impl_(std::move(impl)) {}

Execution Execution::withFilter(std::shared_ptr<CacheableVector> routingObj) {
  return impl_->withFilter(routingObj);
}

Execution Execution::withArgs(std::shared_ptr<Cacheable> args) {
  return impl_->withArgs(args);
}

Execution Execution::withCollector(std::shared_ptr<ResultCollector> rs) {
  return impl_->withCollector(rs);
}

std::shared_ptr<ResultCollector> Execution::execute(
    const std::string& func, std::chrono::milliseconds timeout) {
  return impl_->execute(func, timeout);
}

std::shared_ptr<ResultCollector> Execution::execute(
    const std::shared_ptr<CacheableVector>& routingObj,
    const std::shared_ptr<Cacheable>& args,
    const std::shared_ptr<ResultCollector>& rs, const std::string& func,
    std::chrono::milliseconds timeout) {
  return impl_->execute(routingObj, args, rs, func, timeout);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
