#pragma once

#ifndef GEODE_PDXTYPES_H_
#define GEODE_PDXTYPES_H_

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

#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

class PdxTypes {
 public:
  static const int8_t kPdxByteSize = 1;

  static const int8_t kPdxBooleanSize = 1;

  static const int8_t kPdxCharSize = 2;

  static const int8_t kPdxShortSize = 2;

  static const int8_t kPdxIntegerSize = 4;

  static const int8_t kPdxFloatSize = 4;

  static const int8_t kPdxLongSize = 8;

  static const int8_t kPdxDoubleSize = 8;

  static const int8_t kPdxDateSize = 8;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPES_H_
