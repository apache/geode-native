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

#ifndef GEODE_TESTOBJECT_TIMESTAMPEDOBJECT_H_
#define GEODE_TESTOBJECT_TIMESTAMPEDOBJECT_H_

#include "testobject_export.h"

namespace testobject {

using namespace apache::geode::client;
using namespace testframework;

class TESTOBJECT_EXPORT TimestampedObject : public Serializable {
 public:
  virtual uint64_t getTimestamp() { return 0; }
  virtual void resetTimestamp() {}
  void fromData(DataInput&) override {}
  void toData(DataOutput&) const override {}
  int32_t classId() const override { return 0; }
  size_t objectSize() const override { return 0; }
  ~TimestampedObject() override = default;
};
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_TIMESTAMPEDOBJECT_H_
