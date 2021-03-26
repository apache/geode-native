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

#include <benchmark/benchmark.h>

#include <mutex>
#include <thread>

#include "ConnectionQueue.hpp"

class TestObject {
 public:
  void close() {}
};

template <class T>
void ConnectionQueueBM_getUntil(benchmark::State& state) {
  T queue;
  for (int64_t i = 0; i < state.range(0); ++i) {
    queue.put(new TestObject(), true);
  }

  for (auto _ : state) {
    auto v = queue.getUntil(std::chrono::hours(1));
    queue.put(v, true);
  }
}

const auto MAX_THREADS = std::thread::hardware_concurrency() * 8;

BENCHMARK_TEMPLATE(ConnectionQueueBM_getUntil,
                   apache::geode::client::ConnectionQueue<TestObject>)
    ->Range(1, MAX_THREADS * 2)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();

BENCHMARK_TEMPLATE(
    ConnectionQueueBM_getUntil,
    apache::geode::client::ConnectionQueue<TestObject, std::recursive_mutex>)
    ->Range(1, MAX_THREADS * 2)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();
