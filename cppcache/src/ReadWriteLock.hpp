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

#ifndef GEODE_READWRITELOCK_H_
#define GEODE_READWRITELOCK_H_

#include <boost/thread/shared_mutex.hpp>

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

class TryReadGuard {
 public:
  TryReadGuard(boost::shared_mutex& lock, const volatile bool& exit_cond);
  ~TryReadGuard() {
    if (locked_) {
      mutex_.unlock_shared();
    }
  }
  bool locked() const { return locked_; }

 private:
  boost::shared_mutex& mutex_;
  bool locked_;
};

class TryWriteGuard {
 public:
  TryWriteGuard(boost::shared_mutex& mutex, const volatile bool& exit_cond);
  ~TryWriteGuard() {
    if (locked_) {
      mutex_.unlock();
    }
  }
  bool locked() const { return locked_; }

 private:
  boost::shared_mutex& mutex_;
  bool locked_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_READWRITELOCK_H_
