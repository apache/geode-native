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

#include <geode/CacheableString.hpp>
#include <geode/CqListener.hpp>
#include <geode/CqOperation.hpp>

class SimpleCqListener : public apache::geode::client::CqListener {
 public:
  SimpleCqListener();
  void onEvent(const apache::geode::client::CqEvent& cqEvent) override;
  void onError(const apache::geode::client::CqEvent& cqEvent) override;
  void close() override;

  int32_t getCreationCount();
  int32_t getUpdateCount();
  int32_t getDestructionCount();

 private:
  int32_t creationCount_;
  int32_t updateCount_;
  int32_t destructionCount_;
};

#endif // SIMPLE_CQ_LISTENER_H

