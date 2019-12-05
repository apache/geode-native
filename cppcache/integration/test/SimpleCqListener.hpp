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

#ifndef SIMPLE_CQ_LISTENER_H
#define SIMPLE_CQ_LISTENER_H

#include <memory>

#include <boost/thread/latch.hpp>

#include <geode/CacheableString.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqOperation.hpp>

class SimpleCqListener : public apache::geode::client::CqListener {
 public:
  SimpleCqListener(std::shared_ptr<boost::latch> createLatch,
                   std::shared_ptr<boost::latch> updateLatch,
                   std::shared_ptr<boost::latch> destroyLatch);
  void onEvent(const apache::geode::client::CqEvent& cqEvent) override;
  void onError(const apache::geode::client::CqEvent& cqEvent) override;
  void close() override;

 private:
  std::shared_ptr<boost::latch> createLatch_;
  std::shared_ptr<boost::latch> updateLatch_;
  std::shared_ptr<boost::latch> destroyLatch_;
};

#endif  // SIMPLE_CQ_LISTENER_H
