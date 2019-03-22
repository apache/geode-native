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

#ifndef GEODE_LRULIST_H_
#define GEODE_LRULIST_H_

#include <atomic>
#include <memory>

#include <geode/internal/geode_globals.hpp>

#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {

// Bit mask for recently used
#define RECENTLY_USED_BITS 1u
// Bit mask for evicted
#define EVICTED_BITS 2u

/**
 * @brief This class encapsulates LRU specific properties for a LRUList node.
 */
class APACHE_GEODE_EXPORT LRUEntryProperties {
 public:
  inline LRUEntryProperties() : m_bits(0), m_persistenceInfo(nullptr) {}

  inline void setRecentlyUsed() { m_bits |= RECENTLY_USED_BITS; }

  inline void clearRecentlyUsed() { m_bits &= ~RECENTLY_USED_BITS; }

  inline bool testRecentlyUsed() const {
    return (m_bits.load() & RECENTLY_USED_BITS) == RECENTLY_USED_BITS;
  }

  inline bool testEvicted() const {
    return (m_bits.load() & EVICTED_BITS) == EVICTED_BITS;
  }

  inline void setEvicted() { m_bits |= EVICTED_BITS; }

  inline void clearEvicted() { m_bits &= ~EVICTED_BITS; }

  inline const std::shared_ptr<void>& getPersistenceInfo() const {
    return m_persistenceInfo;
  }

  inline void setPersistenceInfo(const std::shared_ptr<void>& persistenceInfo) {
    m_persistenceInfo = persistenceInfo;
  }

 protected:
  // this constructor deliberately skips initializing any fields
  inline explicit LRUEntryProperties(bool) {}

 private:
  std::atomic<uint32_t> m_bits;
  std::shared_ptr<void> m_persistenceInfo;
};

using util::concurrent::spinlock_mutex;

/**
 * @brief Maintains a list of entries returning them through head in
 * approximate LRU order. The <code>TEntry</code> template argument
 * must provide a <code>getLRUProperties</code> method that returns an
 * object of class <code>LRUEntryProperties</code>.
 */
template <typename TEntry, typename TCreateEntry>
class LRUList {
 protected:
  /**
   * @brief The entries in the LRU List are instances of LRUListNode.
   * This maintains the evicted and recently used state for each entry.
   */
  class LRUListNode {
   public:
    inline explicit LRUListNode(const std::shared_ptr<TEntry>& entry)
        : m_entry(entry), m_nextLRUListNode(nullptr) {}

    inline ~LRUListNode() {}

    inline void getEntry(std::shared_ptr<TEntry>& result) const {
      result = m_entry;
    }

    inline LRUListNode* getNextLRUListNode() const { return m_nextLRUListNode; }

    inline void setNextLRUListNode(LRUListNode* next) {
      m_nextLRUListNode = next;
    }

    inline void clearNextLRUListNode() { m_nextLRUListNode = nullptr; }

   private:
    std::shared_ptr<TEntry> m_entry;
    LRUListNode* m_nextLRUListNode;

    // disabled
    LRUListNode(const LRUListNode&);
    LRUListNode& operator=(const LRUListNode&);
  };

 public:
  LRUList();
  ~LRUList();

  /**
   * @brief add an entry to the tail of the list.
   */
  void appendEntry(const std::shared_ptr<TEntry>& entry);

  /**
   * @brief return the least recently used node from the list,
   * and removing it from the list.
   */
  void getLRUEntry(std::shared_ptr<TEntry>& result);

 private:
  /**
   * @brief add a node to the tail of the list.
   */
  void appendNode(LRUListNode* aNode);

  /**
   * @brief return the head entry in the list,
   * and removing it from the list.
   */
  LRUListNode* getHeadNode(bool& isLast);

  spinlock_mutex m_headLock;
  spinlock_mutex m_tailLock;

  LRUListNode* m_headNode;
  LRUListNode* m_tailNode;

};  // LRUList

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRULIST_H_
