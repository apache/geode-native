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

#include "Version.hpp"

#include <limits>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace apache {
namespace geode {
namespace client {

void Version::write(DataOutput& dataOutput, const Version& version,
                    bool compressed) {
  const auto ordinal = version.getOrdinal();
  if (compressed && (ordinal <= std::numeric_limits<int8_t>::max())) {
    dataOutput.write(static_cast<int8_t>(ordinal));
  } else {
    dataOutput.write(kTokenOrdinal);
    dataOutput.writeInt(ordinal);
  }
}

Version Version::read(DataInput& dataInput) {
  int16_t ordinal = dataInput.read();
  if (kTokenOrdinal == ordinal) {
    ordinal = dataInput.readInt16();
  }
  return Version(ordinal);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
