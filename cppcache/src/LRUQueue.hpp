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

#ifndef GEODE_LRUQUEUE_H_
#define GEODE_LRUQUEUE_H_

#include <list>
#include <memory>
#include <mutex>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

class MapEntryImpl;

/**
 * This class holds a queue of entries sorted by its use order
 * @note All accesses to the queue are mutually exclusive
 */
class LRUQueue {
 public:
  using type = std::shared_ptr<MapEntryImpl>;

 public:
  /**
   * Class destructor
   */
  ~LRUQueue();

  /**
   * Push the given entry into the queue's tail
   * @param entry Entry to be pushed
   */
  void push(const type &entry);

  /**
   * Pops an entry from the queue's head
   * @return If the queue is not empty, the entry on the queue's head
   *         is returned, nullptr otherwise.
   */
  type pop();

  /**
   * Removes an entry from the queue
   * @param entry Entry to be removed
   */
  void remove(const type &entry);

  /**
   * Moves the given entry to the queue's tail
   * @param entry Entry to be moved
   */
  void move_to_end(const type &entry);

  /**
   * Clear the queue
   */
  void clear();

  /**
   * Returns the number of items in the queue
   */
  std::size_t size() const { return container_.size(); }

 protected:
  using mutex = std::mutex;
  template <class _Entry>
  using container_impl = std::list<_Entry>;
  using container = container_impl<type>;

 protected:
  mutex mutex_;
  container container_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUQUEUE_H_
