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

#ifndef GEODE_FUNCTIONEXPIRYTASK_H_
#define GEODE_FUNCTIONEXPIRYTASK_H_

#include "ExpiryTask.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class FunctionExpiryTask FunctionExpiryTask.hpp
 *
 * This class is a placeholder for generic expiry tasks
 * It runs the given callback upon expiration.
 * Given that the callback is a std::function, it can take anything ranging
 * from a classical function, a bound function, a lambda, or a instance of a
 * class implementing operator()
 */
class FunctionExpiryTask : public ExpiryTask {
 public:
  using callback_t = std::function<void()>;

 public:
  /// Class constructors

  /**
   * Class constructor
   * @param manager A reference to the ExpiryTaskManager
   * @param callback Callback to be executed by the task
   * @see ExpiryTask::on_expire for more info about the return value of the
   *      callback.
   */
  FunctionExpiryTask(ExpiryTaskManager& manager, callback_t callback)
      : ExpiryTask(manager), on_expire_(callback) {}

 protected:
  bool on_expire() override {
    on_expire_();
    return true;
  }

 protected:
  callback_t on_expire_;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FUNCTIONEXPIRYTASK_H_
