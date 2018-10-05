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

#ifndef INTERNAL_HACKS_ACETHREADID_H_
#define INTERNAL_HACKS_ACETHREADID_H_

#include <cstdint>
#include <type_traits>

namespace hacks {

template <class _T>
uint64_t aceThreadId(
    _T&& thread,
    typename std::enable_if<std::is_pointer<_T>::value>::type* = nullptr) {
  return reinterpret_cast<uintptr_t>(thread);
}

template <class _T>
uint64_t aceThreadId(
    _T&& thread,
    typename std::enable_if<std::is_integral<_T>::value>::type* = nullptr) {
  return thread;
}

}  // namespace hacks

#endif  // INTERNAL_HACKS_ACETHREADID_H_
