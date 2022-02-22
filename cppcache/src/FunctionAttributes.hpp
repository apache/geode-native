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

#ifndef GEODE_FUNCTIONATTRIBUTES_H_
#define GEODE_FUNCTIONATTRIBUTES_H_

#include <cstdint>

namespace apache {
namespace geode {
namespace client {

class FunctionAttributes {
public:
 enum : uint8_t {
   IS_HA = 1,
   HAS_RESULT = 2,
   OPTIMIZE_FOR_WRITE = 4,
   RETRY = 8,
   VALID = 128U,
 };

public:
 FunctionAttributes() : flags_{0} {}
 FunctionAttributes(bool isHA, bool hasResult, bool optimizeForWrite)
     : flags_{
           static_cast<uint8_t>(static_cast<uint8_t>(isHA) |
                                (static_cast<uint8_t>(hasResult) << 1) |
                                (static_cast<uint8_t>(optimizeForWrite) << 2) |
                                static_cast<uint8_t>(VALID))} {}

 explicit operator bool() const {
   return (flags_ & VALID) != 0;
 }

 bool isHA() const
 {
   return (flags_ & IS_HA) != 0;
 }

 bool hasResult() const
 {
   return (flags_ & HAS_RESULT) != 0;
 }

 bool isOptimizedForWrite() const
 {
   return (flags_ & OPTIMIZE_FOR_WRITE) != 0;
 }

 bool isRetry() const {
   return (flags_ & RETRY) != 0;
 }

 FunctionAttributes& markRetry() {
   flags_ |= RETRY;
   return *this;
 }

 uint8_t getFlags() const {
   return flags_ & ~VALID;
 }

protected:
 uint8_t flags_;
};  // class FunctionAttributes
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FUNCTIONATTRIBUTES_H_
