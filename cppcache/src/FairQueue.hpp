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

#ifndef GEODE_FAIRQUEUE_H_
#define GEODE_FAIRQUEUE_H_

#include <deque>

#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>
#include <ace/Time_Value.h>
#include <ace/Token.h>

#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

template <class T, class MUTEX = ACE_Thread_Mutex>
class FairQueue {
 public:
  FairQueue() : m_cond(m_queueLock), m_closed(false) {}

  virtual ~FairQueue() { close(); }

  int waiters() { return m_queueGetLock.waiters(); }

  /** get without wait */
  T* getNoWait() {
    bool isClosed;
    T* mp = getNoGetLock(isClosed);
    return mp;
  }

  /** wait sec time until notified */
  T* getUntil(std::chrono::microseconds& sec) {
    bool isClosed;
    T* mp = getNoGetLock(isClosed);

    if (mp == nullptr && !isClosed) {
      mp = getUntilWithToken(sec, isClosed, static_cast<void*>(nullptr));
    }
    return mp;
  }

  void put(T* mp, bool openQueue) {
    bool delMp = false;
    {
      ACE_Guard<MUTEX> _guard(m_queueLock);
      if (openQueue || !m_closed) {
        m_queue.push_front(mp);
        m_closed = false;
        m_cond.signal();
      } else {
        delMp = true;
      }
    }
    if (delMp) {
      mp->close();
      delete mp;
    }
  }

  uint32_t size() {
    ACE_Guard<MUTEX> _guard(m_queueLock);

    return static_cast<uint32_t>(m_queue.size());
  }

  bool empty() { return (size() == 0); }

  void close() {
    ACE_Guard<MUTEX> _guard(m_queueLock);

    m_closed = true;
    LOGDEBUG("Internal fair queue size while closing is %zu", m_queue.size());
    while (m_queue.size() > 0) {
      T* mp = m_queue.back();
      m_queue.pop_back();
      mp->close();
      delete mp;
      deleteAction();
    }
    LOGDEBUG("FairQueue::close( ): queue closed ");
    m_cond.signal();
  }

  void reset() {
    ACE_Guard<MUTEX> _guard(m_queueLock);

    m_closed = false;
  }

  MUTEX& getQueueLock() { return m_queueLock; }

 private:
  T* getNoGetLock(bool& isClosed) {
    ACE_Guard<MUTEX> _guard(m_queueLock);

    return popFromQueue(isClosed);
  }

  ACE_Condition<MUTEX> m_cond;
  ACE_Token m_queueGetLock;
  bool m_closed;

  bool exclude(T*, void*) { return false; }

 protected:
  std::deque<T*> m_queue;
  MUTEX m_queueLock;

  inline T* popFromQueue(bool& isClosed) {
    T* mp = nullptr;

    isClosed = m_closed;
    if (!isClosed && m_queue.size() > 0) {
      mp = m_queue.back();
      m_queue.pop_back();
    }
    return mp;
  }

  template <typename U>
  T* getUntilWithToken(std::chrono::microseconds& sec, bool& isClosed,
                       U* excludeList = nullptr) {
    T* mp = nullptr;

    ACE_Guard<ACE_Token> _guard(m_queueGetLock);

    ACE_Time_Value currTime(ACE_OS::gettimeofday());
    ACE_Time_Value stopAt(currTime);
    stopAt += sec;

    ACE_Guard<MUTEX> _guard1(m_queueLock);

    if (!m_closed) {
      do {
        m_cond.wait(&stopAt);
        mp = popFromQueue(isClosed);
        if (mp && excludeList) {
          if (exclude(mp, excludeList)) {
            mp->close();
            _GEODE_SAFE_DELETE(mp);
            deleteAction();
          }
        }
      } while (mp == nullptr && (currTime = ACE_OS::gettimeofday()) < stopAt &&
               !isClosed);
      sec = std::chrono::microseconds((stopAt - currTime).usec());
    }

    return mp;
  }

  virtual void deleteAction() {}
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FAIRQUEUE_H_
