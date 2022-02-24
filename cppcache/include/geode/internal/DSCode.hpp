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

#ifndef NATIVECLIENT_DSCODE_H
#define NATIVECLIENT_DSCODE_H

namespace apache {
namespace geode {
namespace client {
namespace internal {

enum class DSCode : int32_t {
  FixedIDDefault = 0,
  FixedIDByte = 1,
  FixedIDInt = 3,
  FixedIDNone = 4,
  FixedIDShort = 2,
  CacheableLinkedList = 10,
  Properties = 11,
  PdxType = 17,  // internal hack to read pdxtype in c# layer, look usuage in
                 // TcrMessage and  C# DistributedM.cpp
  BooleanArray = 26,
  CharArray = 27,
  CacheableUserData = 39,
  CacheableUserData2 = 38,
  CacheableUserData4 = 37,
  NullObj = 41,
  CacheableString = 42,
  Class = 43,
  JavaSerializable = 44,
  DataSerializable = 45,
  CacheableBytes = 46,
  CacheableInt16Array = 47,
  CacheableInt32Array = 48,
  CacheableInt64Array = 49,
  CacheableFloatArray = 50,
  CacheableDoubleArray = 51,
  CacheableObjectArray = 52,
  CacheableBoolean = 53,
  CacheableCharacter = 54,
  CacheableByte = 55,
  CacheableInt16 = 56,
  CacheableInt32 = 57,
  CacheableInt64 = 58,
  CacheableFloat = 59,
  CacheableDouble = 60,
  CacheableDate = 61,
  CacheableFileName = 63,
  CacheableStringArray = 64,
  CacheableArrayList = 65,
  CacheableHashSet = 66,
  CacheableHashMap = 67,
  CacheableTimeUnit = 68,
  CacheableNullString = 69,
  CacheableHashTable = 70,
  CacheableVector = 71,
  CacheableIdentityHashMap = 72,
  CacheableLinkedHashSet = 73,
  CacheableStack = 74,
  CacheableASCIIString = 87,
  CacheableASCIIStringHuge = 88,
  CacheableStringHuge = 89,
  PDX = 93,
  PDX_ENUM = 94
};

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // NATIVECLIENT_DSCODE_H
