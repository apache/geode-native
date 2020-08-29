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

#include "SimpleCqListener.hpp"

#include <iostream>

#include <geode/CqListener.hpp>
#include <geode/CqOperation.hpp>

SimpleCqListener::SimpleCqListener(std::shared_ptr<boost::latch> createLatch,
                                   std::shared_ptr<boost::latch> updateLatch,
                                   std::shared_ptr<boost::latch> destroyLatch)
    : createLatch_(createLatch),
      updateLatch_(updateLatch),
      destroyLatch_(destroyLatch) {}

void SimpleCqListener::onEvent(const apache::geode::client::CqEvent& cqEvent) {
  switch (cqEvent.getQueryOperation()) {
    case apache::geode::client::CqOperation::OP_TYPE_CREATE:
      createLatch_->count_down();
      break;
    case apache::geode::client::CqOperation::OP_TYPE_UPDATE:
      updateLatch_->count_down();
      break;
    case apache::geode::client::CqOperation::OP_TYPE_DESTROY:
      destroyLatch_->count_down();
      break;
    case apache::geode::client::CqOperation::OP_TYPE_INVALID:
    case apache::geode::client::CqOperation::OP_TYPE_INVALIDATE:
    case apache::geode::client::CqOperation::OP_TYPE_REGION_CLEAR:
    case apache::geode::client::CqOperation::OP_TYPE_MARKER:
      break;
  }
}

void SimpleCqListener::onError(const apache::geode::client::CqEvent& cqEvent) {
  std::cout << __FUNCTION__ << " called"
            << dynamic_cast<apache::geode::client::CacheableString*>(
                   cqEvent.getKey().get())
                   ->value()
            << std::endl;
}

void SimpleCqListener::close() {
  std::cout << __FUNCTION__ << " called" << std::endl;
}
