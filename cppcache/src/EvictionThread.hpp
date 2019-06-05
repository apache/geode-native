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

#ifndef GEODE_EVICTIONTHREAD_H_
#define GEODE_EVICTIONTHREAD_H_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace apache {
namespace geode {
namespace client {

class EvictionController;

/**
 * This class does the actual evictions
 */
class EvictionThread {
 public:
   EvictionThread(EvictionController* parent);
  void start();
  void stop();
  void svc();
  void putEvictionInfo(int32_t info);

 private:
  std::thread m_thread;
  std::atomic<bool> m_run;
  EvictionController* m_pParent;
  std::deque<int32_t> m_queue;
  std::mutex m_queueMutex;
  std::condition_variable m_queueCondition;

  static const char* NC_Evic_Thread;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EVICTIONTHREAD_H_
