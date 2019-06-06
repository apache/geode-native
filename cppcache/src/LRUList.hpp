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
#include <mutex>

#include <geode/internal/geode_globals.hpp>

#include "LRUEntryProperties.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

namespace apache {
namespace geode {
namespace client {
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
    inline LRUListNode(const std::shared_ptr<TEntry>& entry)
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
  LRUList() : m_headLock(), m_tailLock() {
    std::shared_ptr<TEntry> headEntry(TCreateEntry::create(nullptr));
    headEntry->getLRUProperties().setEvicted();  // create empty evicted entry.
    m_headNode = new LRUListNode(headEntry);
    m_tailNode = m_headNode;
  }

  ~LRUList() {
    m_tailNode = nullptr;
    LRUListNode* next;
    while (m_headNode != nullptr) {
      next = m_headNode->getNextLRUListNode();
      delete m_headNode;
      m_headNode = next;
    }
  }

  /**
   * @brief add an entry to the tail of the list.
   */
  void appendEntry(const std::shared_ptr<TEntry>& entry) {
    std::lock_guard<spinlock_mutex> lk(m_tailLock);

    LRUListNode* aNode = new LRUListNode(entry);
    m_tailNode->setNextLRUListNode(aNode);
    m_tailNode = aNode;
  }

  /**
   * @brief return the least recently used node from the list,
   * and removing it from the list.
   */
  void getLRUEntry(std::shared_ptr<TEntry>& result) {
    bool isLast = false;
    LRUListNode* aNode;
    while (true) {
      aNode = getHeadNode(isLast);
      if (aNode == nullptr) {
        result = nullptr;
        break;
      }
      aNode->getEntry(result);
      if (isLast) {
        break;
      }
      // otherwise, check if it should be discarded or put back on the list
      // instead of returned...
      LRUEntryProperties& lruProps = result->getLRUProperties();
      if (!lruProps.testEvicted()) {
        if (lruProps.testRecentlyUsed()) {
          lruProps.clearRecentlyUsed();
          appendNode(aNode);
          // now try again.
        } else {
          delete aNode;
          break;  // found unused entry
        }
      } else {
        result = nullptr;  // remove the reference to entry
        delete aNode;      // drop the entry to the floor ...
      }
    }
  }

 private:
  /**
   * @brief add a node to the tail of the list.
   */
  void appendNode(LRUListNode* aNode) {
    std::lock_guard<spinlock_mutex> lk(m_tailLock);

    aNode->clearNextLRUListNode();
    m_tailNode->setNextLRUListNode(aNode);
    m_tailNode = aNode;
  }

  /**
   * @brief return the head entry in the list,
   * and removing it from the list.
   */
  LRUListNode* getHeadNode(bool& isLast) {
    std::lock_guard<spinlock_mutex> lk(m_headLock);

    LRUListNode* result = m_headNode;
    LRUListNode* nextNode;

    {
      std::lock_guard<spinlock_mutex> lk(m_tailLock);

      nextNode = m_headNode->getNextLRUListNode();
      if (nextNode == nullptr) {
        // last one in the list...
        isLast = true;
        std::shared_ptr<TEntry> entry;
        result->getEntry(entry);
        if (entry->getLRUProperties().testEvicted()) {
          // list is empty.
          return nullptr;
        } else {
          entry->getLRUProperties().setEvicted();
          return result;
        }
      }
    }

    isLast = false;
    // advance head node, and return old value.
    m_headNode = nextNode;

    return result;
  }

  spinlock_mutex m_headLock;
  spinlock_mutex m_tailLock;

  LRUListNode* m_headNode;
  LRUListNode* m_tailNode;

};  // LRUList

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRULIST_H_
