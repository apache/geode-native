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
#include <geode/CacheableFileName.hpp>
#include <geode/GeodeTypeIds.hpp>
#include <geode/DataOutput.hpp>
#include <geode/DataInput.hpp>

#include <ace/ACE.h>
#include <ace/OS.h>

namespace apache {
namespace geode {
namespace client {

void CacheableFileName::toData(DataOutput& output) const {
  output.write(m_type);
  CacheableString::toData(output);
}

void CacheableFileName::fromData(DataInput& input) {
  m_type = input.read();
  CacheableString::fromData(input);
}

int8_t CacheableFileName::getDsCode() const {
  return GeodeTypeIds::CacheableFileName;
}

int32_t CacheableFileName::hashcode() const {
  if (m_hashcode == 0) {
#ifdef _WIN32
    int localHashcode = 0;
    for (auto&& c : CacheableString::value()) {
      localHashcode = 31 * localHashcode + std::tolower(c, std::locale());
    }
    m_hashcode = localHashcode ^ 1234321;
#else
    m_hashcode = CacheableString::hashcode() ^ 1234321;
#endif
  }
  return m_hashcode;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
