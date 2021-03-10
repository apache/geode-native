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

#ifndef GEODE_VERSION_H_
#define GEODE_VERSION_H_

#include <cstdint>

namespace apache {
namespace geode {
namespace client {

class DataOutput;
class DataInput;

class Version {
 public:
  inline int16_t getOrdinal() const noexcept { return ordinal_; }

  static const Version& current() noexcept {
    static const auto version = Version{45};  // Geode 1.0.0
    return version;
  }

  static void write(DataOutput& dataOutput, const Version& version,
                    bool compressed = true);
  static Version read(DataInput& dataOutput);

 private:
  const int16_t ordinal_;

  inline explicit Version(int16_t ordinal) noexcept : ordinal_(ordinal) {}

  static constexpr int8_t kTokenOrdinal = -1;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_VERSION_H_
