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
#include "LRUList.hpp"
#include "util/concurrent/spinlock_mutex.hpp"

#include <mutex>

namespace apache {
namespace geode {
namespace client {

using util::concurrent::spinlock_mutex;

template <typename TEntry, typename TCreateEntry>
LRUList<TEntry, TCreateEntry>::LRUList() : m_headLock(), m_tailLock() {
  std::shared_ptr<TEntry> headEntry(TCreateEntry::create(nullptr));
  headEntry->getLRUProperties().setEvicted();  // create empty evicted entry.
  m_headNode = new LRUListNode(headEntry);
  m_tailNode = m_headNode;
}

template <typename TEntry, typename TCreateEntry>
LRUList<TEntry, TCreateEntry>::~LRUList() {
  m_tailNode = nullptr;
  LRUListNode* next;
  while (m_headNode != nullptr) {
    next = m_headNode->getNextLRUListNode();
    delete m_headNode;
    m_headNode = next;
  }
}

template <typename TEntry, typename TCreateEntry>
void LRUList<TEntry, TCreateEntry>::appendEntry(const std::shared_ptr<TEntry>& entry) {
  std::lock_guard<spinlock_mutex> lk(m_tailLock);

  LRUListNode* aNode = new LRUListNode(entry);
  m_tailNode->setNextLRUListNode(aNode);
  m_tailNode = aNode;
}

template <typename TEntry, typename TCreateEntry>
void LRUList<TEntry, TCreateEntry>::appendNode(LRUListNode* aNode) {
  std::lock_guard<spinlock_mutex> lk(m_tailLock);

  GF_D_ASSERT(aNode != nullptr);

  aNode->clearNextLRUListNode();
  m_tailNode->setNextLRUListNode(aNode);
  m_tailNode = aNode;
}

template <typename TEntry, typename TCreateEntry>
void LRUList<TEntry, TCreateEntry>::getLRUEntry(std::shared_ptr<TEntry>& result) {
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

template <typename TEntry, typename TCreateEntry>
typename LRUList<TEntry, TCreateEntry>::LRUListNode*
LRUList<TEntry, TCreateEntry>::getHeadNode(bool& isLast) {
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

}  // namespace client
}  // namespace geode
}  // namespace apache
