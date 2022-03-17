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

#ifndef GEODE_UTIL_SYNCHRONIZED_MAP_H_
#define GEODE_UTIL_SYNCHRONIZED_MAP_H_

#include <mutex>
#include <utility>

namespace apache {
namespace geode {
namespace client {

/**
 * Wrapper around std::map, std::unordered_map and other map like
 * implementations.
 *
 * This is a very incomplete implementation. Add methods as needed.
 *
 * @tparam Map type to wrap.
 * @tparam Mutex type to synchronize with. Defaults to std::recursive_mutex
 */
template <class Map, class Mutex = std::recursive_mutex>
class synchronized_map {
 private:
  Map map_;
  mutable Mutex mutex_;

 public:
  typedef Mutex mutex_type;
  typedef Map map_type;
  typedef typename Map::key_type key_type;
  typedef typename Map::mapped_type mapped_type;
  typedef typename Map::allocator_type allocator_type;
  typedef typename Map::value_type value_type;
  typedef typename Map::reference reference;
  typedef typename Map::const_reference const_reference;
  typedef typename Map::iterator iterator;
  typedef typename Map::const_iterator const_iterator;
  typedef typename Map::difference_type difference_type;
  typedef typename Map::size_type size_type;

  inline mutex_type& mutex() const noexcept { return mutex_; }

  inline map_type& map() noexcept { return map_; }
  inline const map_type& map() const noexcept { return map_; }

  /**
   * Allocates a Lock object around the Mutex and locks the Mutex.
   *
   * Example:
   * \code{.cpp}
   * auto&& guard = exampleMap.make_lock();
   * \endcode
   *
   * Equivalent to:
   * \code{.cpp}
   * std::lock_guard<decltype(exampleMap)::mutex_type> guard(exampleMap.mutex);
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
   * auto&& guard = exampleMap.make_lock<std::unique_lock>(std::defer_lock);
   * \endcode
   *
   * Equivalent to:
   * \code{.cpp}
   * std::unique_lock<decltype(exampleMap)::mutex_type> guard(exampleMap.mutex,
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
  inline std::pair<typename Map::iterator, bool> emplace(Args&&... args) {
    std::lock_guard<Mutex> lock(mutex_);
    return map_.emplace(std::forward<Args>(args)...);
  }

  inline size_type erase(const key_type& key) {
    std::lock_guard<Mutex> lock(mutex_);
    return map_.erase(key);
  }

  inline bool empty() const noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    return map_.empty();
  }

  inline size_type size() const noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    return map_.size();
  }

  inline void clear() noexcept {
    std::lock_guard<Mutex> lock(mutex_);
    map_.clear();
  }

  inline iterator find(const key_type& key) {
    //    std::lock_guard<Mutex> lock(mutex_);
    return map_.find(key);
  }

  inline const_iterator find(const key_type& key) const {
    //    std::lock_guard<Mutex> lock(mutex_);
    return map_.find(key);
  }

  inline iterator begin() noexcept { return map_.begin(); }
  inline iterator end() noexcept { return map_.end(); }
  inline const_iterator begin() const noexcept { return map_.begin(); }
  inline const_iterator end() const noexcept { return map_.end(); }
  inline const_iterator cbegin() const noexcept { return map_.begin(); }
  inline const_iterator cend() const noexcept { return map_.end(); }

  template <class InputIterator>
  inline void insert(InputIterator first, InputIterator last) {
    std::lock_guard<Mutex> lock(mutex_);
    map_.insert(first, last);
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_UTIL_SYNCHRONIZED_MAP_H_
