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

#ifndef GEODE_UTIL_CONCURRENT_BINARY_SEMAPHORE_H_
#define GEODE_UTIL_CONCURRENT_BINARY_SEMAPHORE_H_

#include <condition_variable>
#include <mutex>

namespace apache {
namespace geode {
namespace client {
class binary_semaphore {
 public:
  explicit binary_semaphore(bool released);

  void release();
  void acquire();
  bool try_acquire_for(const std::chrono::milliseconds& period);

 protected:
  bool released_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif /* GEODE_UTIL_CONCURRENT_BINARY_SEMAPHORE_H_ */
