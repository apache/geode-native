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

#ifndef INTERNAL_HACKS_RANGE_H
#define INTERNAL_HACKS_RANGE_H

#include <utility>

namespace hacks {

/**
 * Fixes for-range loop bug in Solaris Studio 12.6. Does not generate correctly
 * where begin/end methods are virtual.
 *
 * @see https://community.oracle.com/message/14647160#14647160
 */

#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x5150

template <class Range>
struct range_wrapper {
  Range&& range_;
  range_wrapper(Range&& range) : range_(range) {}
  inline decltype(range_.begin()) begin() { return range_.begin(); }
  inline decltype(range_.end()) end() { return range_.end(); }
};

template <class Range>
inline range_wrapper<Range> range(Range&& range) {
  return range_wrapper<Range>(std::forward<Range>(range));
}

#else

template <class Range>
inline Range range(Range&& range) {
  return std::forward<Range>(range);
}

#endif

}  // namespace hacks

#endif  // INTERNAL_HACKS_RANGE_H
