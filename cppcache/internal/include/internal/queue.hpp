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

#ifndef NATIVECLIENT_UTIL_QUEUE_H
#define NATIVECLIENT_UTIL_QUEUE_H

namespace apache {
namespace geode {
namespace client {
namespace queue {

/**
 * Coalesce the contents of a Queue structure based on the given value. Will
 * remove all values at the front of the queue that equal value.
 *
 * @tparam Queue type to coalesce
 * @tparam Type of value to coalesce
 * @param queue to coalesce
 * @param value to coalesce
 */
template <class Queue, class Type>
void coalesce(Queue& queue, const Type& value) {
  while (!queue.empty()) {
    const auto& next = queue.front();
    if (next == value) {
      queue.pop_front();
    } else {
      break;
    }
  }
}

}  // namespace queue
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // NATIVECLIENT_UTIL_QUEUE_H
