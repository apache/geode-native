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

#ifndef GEODE_PDXFIELDTYPES_H_
#define GEODE_PDXFIELDTYPES_H_

namespace apache {
namespace geode {
namespace client {

enum class PdxFieldTypes {
  UNKNOWN = -1,
  BOOLEAN = 0,
  BYTE = 1,
  CHAR = 2,
  SHORT = 3,
  INT = 4,
  LONG = 5,
  FLOAT = 6,
  DOUBLE = 7,
  DATE = 8,
  STRING = 9,
  OBJECT = 10,
  BOOLEAN_ARRAY = 11,
  CHAR_ARRAY = 12,
  BYTE_ARRAY = 13,
  SHORT_ARRAY = 14,
  INT_ARRAY = 15,
  LONG_ARRAY = 16,
  FLOAT_ARRAY = 17,
  DOUBLE_ARRAY = 18,
  STRING_ARRAY = 19,
  OBJECT_ARRAY = 20,
  ARRAY_OF_BYTE_ARRAYS = 21
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXFIELDTYPES_H_
