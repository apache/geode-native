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

#include "TcrMessageDestroy.hpp"

#include <geode/SystemProperties.hpp>

#include "Connector.hpp"
#include "EventOperation.hpp"

namespace apache {
namespace geode {
namespace client {

TcrMessageDestroy::TcrMessageDestroy(DataOutput* dataOutput,
                                     const Region* region,
                                     const std::shared_ptr<CacheableKey>& key,
                                     const std::shared_ptr<Cacheable>& value,
                                     const std::shared_ptr<Serializable>& cbArg,
                                     EventOperation operation,
                                     ThinClientBaseDM* dm) {
  if (!key) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  m_request.reset(dataOutput);
  m_msgType = TcrMessage::DESTROY;
  m_tcdm = dm;
  m_key = key;
  m_regionName = (!region ? "INVALID_REGION_NAME" : region->getFullPath());
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;
  uint32_t numOfParts = 5;
  if (cbArg) {
    ++numOfParts;
  }

  writeHeader(TcrMessage::DESTROY, numOfParts);
  writeRegionPart(m_regionName);
  writeObjectPart(key);
  writeObjectPart(value);                          // expectedOldValue part
  writeBytePart(static_cast<uint8_t>(operation));  // operation part
  writeEventIdPart();

  if (cbArg) {
    writeObjectPart(cbArg);
  }

  writeMessageLength();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
