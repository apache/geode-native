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

#include "internal/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace util {
namespace concurrent {

void spinlock_mutex::lock() {
  while (flag.test_and_set(std::memory_order_acquire)) continue;
}

void spinlock_mutex::unlock() { flag.clear(std::memory_order_release); }

} /* namespace concurrent */
} /* namespace util */
} /* namespace geode */
} /* namespace apache */
