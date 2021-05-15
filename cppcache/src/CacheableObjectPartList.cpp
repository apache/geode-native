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
#include "CacheableObjectPartList.hpp"

#include <geode/CacheableString.hpp>
#include <geode/ExceptionTypes.hpp>

#include "CacheableToken.hpp"
#include "ThinClientRegion.hpp"

namespace apache {
namespace geode {
namespace client {

void CacheableObjectPartList::toData(DataOutput&) const {
  // don't really care about toData() and should never get invoked
  throw UnsupportedOperationException(
      "CacheableObjectPartList::toData not implemented");
}

void CacheableObjectPartList::fromData(DataInput& input) {
  const auto hasKeys = input.readBoolean();
  int32_t len = input.readInt32();
  if (len > 0) {
    std::shared_ptr<CacheableKey> key;
    std::shared_ptr<Cacheable> value;
    std::shared_ptr<Exception> ex;
    // bool isException;
    int32_t keysOffset = (m_keysOffset != nullptr ? *m_keysOffset : 0);
    for (int32_t index = keysOffset; index < keysOffset + len; ++index) {
      if (hasKeys) {
        key = std::dynamic_pointer_cast<CacheableKey>(input.readObject());
      } else if (m_keys != nullptr) {
        key = m_keys->operator[](index);
      } else {
        throw FatalInternalException(
            "CacheableObjectPartList: "
            "hasKeys is false and m_keys is also nullptr");
      }
      if (m_resultKeys != nullptr) {
        m_resultKeys->push_back(key);
      }
      // input.readBoolean(&isException);
      uint8_t byte = input.read();

      if (byte == 2 /* for exception*/) {
        input.advanceCursor(input.readArrayLength());
        // input.readObject(exMsgPtr, true);// Changed
        auto exMsg = input.readString();
        if (exMsg == "org.apache.geode.security.NotAuthorizedException") {
          std::string message("Authorization exception at server: ");
          message += exMsg;
          ex = std::make_shared<NotAuthorizedException>(message);
        } else {
          std::string message("Exception at remote server: ");
          message += exMsg;
          ex = std::make_shared<CacheServerException>(message);
        }
        m_exceptions->emplace(key, ex);
      } else {
        input.readObject(value);
        std::shared_ptr<Cacheable> oldValue;
        if (m_addToLocalCache) {
          // for both  register interest  and getAll it is desired
          // to overwrite an invalidated entry
          // TODO: what about destroyed token? need to handle
          // destroys during  register interest  by not creating them
          // same for invalidates?
          int updateCount = -1;
          MapOfUpdateCounters::iterator pos = m_updateCountMap->find(key);
          if (pos != m_updateCountMap->end()) {
            updateCount = pos->second;
            m_updateCountMap->erase(pos);
          }
          std::shared_ptr<VersionTag> versionTag;
          GfErrType err =
              m_region->putLocal("getAll", false, key, value, oldValue, true,
                                 updateCount, m_destroyTracker, versionTag);
          if (err == GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION) {
            LOG_DEBUG(
                "CacheableObjectPartList::fromData putLocal for key [%s] failed because the cache \
                already contains an entry with higher version.",
                Utils::nullSafeToString(key).c_str());
          }
        } else {
          m_region->getEntry(key, oldValue);
        }
        // if value has already been received via notification or put by
        // another thread, then return that
        if (oldValue != nullptr && !CacheableToken::isInvalid(oldValue)) {
          value = oldValue;
        }
        if (m_values != nullptr) {
          m_values->emplace(key, value);
        }
      }
    }
    if (m_keysOffset != nullptr) {
      *m_keysOffset += len;
    }
  }
}

size_t CacheableObjectPartList::objectSize() const { return 0; }
}  // namespace client
}  // namespace geode
}  // namespace apache
