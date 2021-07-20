#pragma once

#ifndef GEODE_TESTOBJECT_ARRAYOFBYTE_H_
#define GEODE_TESTOBJECT_ARRAYOFBYTE_H_

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

#include <string>

#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "testobject_export.h"

namespace testobject {

using apache::geode::client::CacheableBytes;

class TESTOBJECT_EXPORT ArrayOfByte {
 public:
  static std::shared_ptr<CacheableBytes> init(int size, bool encodeKey,
                                              bool encodeTimestamp);

  static int64_t getTimestamp(std::shared_ptr<CacheableBytes> bytes);

  static void resetTimestamp(std::shared_ptr<CacheableBytes> bytes);
};
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_ARRAYOFBYTE_H_
