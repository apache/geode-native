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

#include "fwklib/GsRandom.hpp"
#include "ArrayOfByte.hpp"
#include "DeltaPSTObject.hpp"

using namespace apache::geode::client;
using namespace testframework;
using namespace testobject;

DeltaPSTObject::DeltaPSTObject(int size, bool encodeKey, bool encodeTimestamp)
    : Delta(nullptr) {
  ACE_Time_Value startTime;
  startTime = ACE_OS::gettimeofday();
  ACE_UINT64 tusec = 0;
  startTime.to_usec(tusec);
  timestamp = tusec * 1000;
  field1 = 1234;
  field2 = '*';
  if (size == 0) {
    valueData = nullptr;
  } else {
    encodeKey = true;
    valueData = ArrayOfByte::init(size, encodeKey, false);
  }
}

void DeltaPSTObject::fromDelta(DataInput& input) {
  field1 = input.readInt32();
  timestamp = input.readInt64();
}

void DeltaPSTObject::toDelta(DataOutput& output) const {
  output.writeInt(static_cast<int32_t>(field1));
  output.writeInt(static_cast<int64_t>(timestamp));
}

void DeltaPSTObject::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(static_cast<int64_t>(timestamp));
  output.writeInt(static_cast<int32_t>(field1));
  output.write(field2);
  output.writeObject(valueData);
}

void DeltaPSTObject::fromData(apache::geode::client::DataInput& input) {
  timestamp = input.readInt64();
  field1 = input.readInt32();
  field2 = input.read();
  valueData = input.readObject<CacheableBytes>();
}

CacheableStringPtr DeltaPSTObject::toString() const {
  char buf[102500];
  sprintf(
      buf,
      "DeltaPSTObject:[timestamp = %lld field1 = %d field2 = %c valueData=%d ]",
      timestamp, field1, field2, valueData->length());
  return CacheableString::create(buf);
}
