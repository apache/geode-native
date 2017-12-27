#pragma once

#ifndef GEODE_STACKTRACE_H_
#define GEODE_STACKTRACE_H_

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

#include <memory>
#include <string>

#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
#include <boost/stacktrace.hpp>

namespace apache {
namespace geode {
namespace client {

class StackTrace {
 public:
  StackTrace();
  virtual ~StackTrace();
  std::string getString() const;

 private:
  boost::stacktrace::stacktrace stacktrace;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STACKTRACE_H_
