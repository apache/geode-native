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

#ifndef GEODE_UTIL_SIZE_TRACKING_ALLOCATOR_H_
#define GEODE_UTIL_SIZE_TRACKING_ALLOCATOR_H_

#include <functional>
#include <mutex>

namespace apache {
namespace geode {
namespace client {

template <typename _Tp>
class size_tracking_allocator {
 public:
  //    typedefs
  using value_type = _Tp;
  using pointer = value_type *;
  using size_type = std::size_t;
  using reference = value_type &;
  using difference_type = std::ptrdiff_t;
  using const_pointer = const value_type *;
  using const_reference = const value_type &;
  using allocate_callback = std::function<void(difference_type)>;

 public:
  template <typename U>
  class rebind {
   public:
    using other = size_tracking_allocator<U>;
  };

 public:
  explicit size_tracking_allocator() = default;
  explicit size_tracking_allocator(const allocate_callback &cb)
      : allocate_cb_{cb} {}

  size_tracking_allocator(const size_tracking_allocator &other)
      : allocate_cb_{other.allocate_cb_} {}

  template <class U>
  explicit size_tracking_allocator(const size_tracking_allocator<U> &other)
      : allocate_cb_{other.allocate_cb_} {}

  //    memory allocation
  pointer allocate(size_type n, const void * = nullptr) {
    auto size = n * sizeof(value_type);
    pointer p = reinterpret_cast<pointer>(::operator new(size));
    if (allocate_cb_) {
      allocate_cb_(size);
    }

    return p;
  }

  void deallocate(pointer pAddress, size_type n) {
    ::operator delete(pAddress);
    if (allocate_cb_) {
      allocate_cb_(-sizeof(value_type) * n);
    }
  }

  //    size
  size_type max_size() const {
    return std::numeric_limits<size_type>::max() / sizeof(_Tp);
  }

  //    construction/destruction
  void construct(pointer address, const value_type &object) {
    new (address) value_type(object);
  }

  void destroy(pointer object) { object->~value_type(); }

  bool operator==(size_tracking_allocator const &) const { return true; }

  bool operator!=(size_tracking_allocator const &other) const {
    return !operator==(other);
  }

 protected:
  allocate_callback allocate_cb_;

 private:
  template <class U>
  friend class size_tracking_allocator;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_SIZE_TRACKING_ALLOCATOR_H_
