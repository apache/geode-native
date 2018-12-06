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

#ifndef GEODE_UTIL_SYNCHRONIZED_SET_H_
#define GEODE_UTIL_SYNCHRONIZED_SET_H_

#include <mutex>
#include <utility>

namespace apache {
namespace geode {
namespace client {

/**
 * Wrapper around std::set, std::unordered_set and other set like
 * implementations.
 *
 * This is a very incomplete implementation. Add methods as needed.
 *
 * @tparam Set type to wrap.
 * @tparam Mutex type to synchronize with. Defaults to std::recursive_mutex
 */
template <class Set, class Mutex = std::recursive_mutex>
class synchronized_set {
 private:
  Set set_;
  mutable Mutex mutex_;

 public:
  typedef Mutex mutex_type;
  typedef Set set_type;
  typedef typename Set::key_type key_type;
  typedef typename Set::allocator_type allocator_type;
  typedef typename Set::value_type value_type;
  typedef typename Set::reference reference;
  typedef typename Set::const_reference const_reference;
  typedef typename Set::iterator iterator;
  typedef typename Set::const_iterator const_iterator;
  typedef typename Set::difference_type difference_type;
  typedef typename Set::size_type size_type;

  inline mutex_type& mutex() const noexcept { return mutex_; }

  inline set_type& set() noexcept { return set_; }
  inline const set_type& set() const noexcept { return set_; }

  /**
   * Allocates a Lock object around the Mutex and locks the Mutex.
   *
   * Example:
   * \code{.cpp}
   * auto&& guard = exampleSet.make_lock();
   * \endcode
   *
   * Equivalent to:
   * \code{.cpp}
   * std::lock_guard<decltype(exampleSet)::mutex_type> guard(exampleSet.mutex);
   * \endcode
   *
   * @tparam Lock type to allocate. Defaults to std::lock_guard.
   * @return allocated Lock object with lock taken.
   * @throws Any exception throws by Mutex::lock()
   */
  template <template <class...> class Lock = std::lock_guard>
  inline Lock<Mutex> make_lock() const {
    mutex_.lock();
    return {mutex_, std::adopt_lock};
  }

  /**
   * Allocates a Lock object around the Mutex passing any args to the Lock
   * constructor.
   *
   * Example:
   * \code{.cpp}
   * auto&& guard = exampleSet.make_lock<std::unique_lock>(std::defer_lock);
   * \endcode
   *
   * Equivalent to:
   * \code{.cpp}
   * std::unique_lock<decltype(exampleSet)::mutex_type> guard(exampleSet.mutex,
   * std::defer_lock); \endcode
   *
   * @tparam Lock type to allocate. Defaults to std::lock_guard.
   * @tparam Args types passed to Lock constructor.
   * @param args values passed to Lock constructor.
   * @return allocated Lock object.
   * @throws Any exception throws by Lock constructors.
   */
  template <template <class...> class Lock = std::lock_guard, class... Args>
  inline Lock<Mutex> make_lock(Args&&... args) const {
    return {mutex_, std::forward<Args>(args)...};
  }

  template <class... Args>
  inline std::pair<typename Set::iterator, bool> emplace(Args&&... args) {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.emplace(std::forward<Args>(args)...);
  }

  inline size_type erase(const key_type& key) {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.erase(key);
  }

  inline bool empty() const noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.empty();
  }

  inline size_type size() const noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.size();
  }

  inline void clear() noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    set_.clear();
  }

  inline iterator find(const key_type& key) {
    //    std::lock_guard<Mutex> lock(mutex_);
    return set_.find(key);
  }

  inline const_iterator find(const key_type& key) const {
    //    std::lock_guard<Mutex> lock(mutex_);
    return set_.find(key);
  }

  inline iterator begin() noexcept { return set_.begin(); }
  inline iterator end() noexcept { return set_.end(); }
  inline const_iterator begin() const noexcept { return set_.begin(); }
  inline const_iterator end() const noexcept { return set_.end(); }
  inline const_iterator cbegin() const noexcept { return set_.begin(); }
  inline const_iterator cend() const noexcept { return set_.end(); }

  template <class InputIterator>
  inline void insert(InputIterator first, InputIterator last) {
    std::lock_guard<Mutex> lock(mutex_);
    set_.insert(first, last);
  }

  inline std::pair<iterator, bool> insert(value_type&& value) {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.insert(std::move(value));
  }

  inline std::pair<iterator, bool> insert(const value_type& value) {
    std::lock_guard<Mutex> lock(mutex_);
    return set_.insert(value);
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_SYNCHRONIZED_SET_H_
