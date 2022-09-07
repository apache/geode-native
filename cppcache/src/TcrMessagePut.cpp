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

#include "TcrMessagePut.hpp"

#include <geode/SystemProperties.hpp>

#include "Connector.hpp"

namespace apache {
namespace geode {
namespace client {

TcrMessagePut::TcrMessagePut(DataOutput* dataOutput, const Region* region,
                             const std::shared_ptr<CacheableKey>& key,
                             const std::shared_ptr<Cacheable>& value,
                             const std::shared_ptr<Serializable>& cbArg,
                             EventOperation operation, bool isDelta,
                             ThinClientBaseDM* connectionDM, bool isMetaRegion,
                             bool fullValueAfterDeltaFail,
                             const std::string& regionName) {
  m_request.reset(dataOutput);
  m_isMetaRegion = isMetaRegion;
  m_msgType = TcrMessage::PUT;
  m_tcdm = connectionDM;
  m_key = key;
  m_regionName = region != nullptr ? region->getFullPath() : regionName;
  m_region = region;
  m_timeout = DEFAULT_TIMEOUT;

  // TODO check the number of parts in this constructor. doubt because in PUT
  // value can be nullptr also.
  uint32_t numOfParts = 5;
  if (cbArg) {
    ++numOfParts;
  }

  numOfParts++;

  if (key == nullptr) {
    throw IllegalArgumentException(
        "key passed to the constructor can't be nullptr");
  }

  numOfParts++;
  writeHeader(m_msgType, numOfParts);
  writeRegionPart(m_regionName);
  writeBytePart(static_cast<int8_t>(operation));

  // Flags are not used for now
  writeIntPart(0);
  writeObjectPart(key);
  writeObjectPart(CacheableBoolean::create(isDelta));
  writeObjectPart(value, isDelta);
  writeEventIdPart(0, fullValueAfterDeltaFail);
  if (cbArg) {
    writeObjectPart(cbArg);
  }

  writeMessageLength();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
