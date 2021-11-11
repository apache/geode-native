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

#include "CqEventImpl.hpp"

#include <geode/CacheableString.hpp>

#include "CacheImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "TcrMessage.hpp"
#include "ThinClientCacheDistributionManager.hpp"
#include "ThinClientPoolHADM.hpp"

namespace apache {
namespace geode {
namespace client {
CqEventImpl::CqEventImpl(std::shared_ptr<CqQuery>& cQuery, CqOperation baseOp,
                         CqOperation cqOp, std::shared_ptr<CacheableKey>& key,
                         std::shared_ptr<Cacheable>& value,
                         ThinClientBaseDM* tcrdm,
                         std::shared_ptr<CacheableBytes> deltaBytes,
                         std::shared_ptr<EventId> eventId)
    : m_error(false) {
  m_cQuery = cQuery;
  m_queryOp = cqOp;
  m_baseOp = baseOp;
  m_key = key;
  m_newValue = value;
  if (m_queryOp == CqOperation::OP_TYPE_INVALID) m_error = true;
  m_tcrdm = tcrdm;
  m_deltaValue = deltaBytes;
  m_eventId = eventId;
}
std::shared_ptr<CqQuery> CqEventImpl::getCq() const { return m_cQuery; }

CqOperation CqEventImpl::getBaseOperation() const { return m_baseOp; }

/**
 * Get the the operation on the query results. Supported operations include
 * update, create, and destroy.
 */
CqOperation CqEventImpl::getQueryOperation() const { return m_queryOp; }

/**
 * Get the key relating to the event.
 * @return Object key.
 */
std::shared_ptr<CacheableKey> CqEventImpl::getKey() const { return m_key; }
/**
 * Get the new value of the modification.
 *  If there is no new value because this is a delete, then
 *  return null.
 */
std::shared_ptr<Cacheable> CqEventImpl::getNewValue() const {
  if (m_deltaValue == nullptr) {
    return m_newValue;
  } else {
    // Get full object for delta
    TcrMessageRequestEventValue fullObjectMsg(
        new DataOutput(
            m_tcrdm->getConnectionManager().getCacheImpl()->createDataOutput()),
        m_eventId);
    TcrMessageReply reply(true, nullptr);
    ThinClientPoolHADM* poolHADM = dynamic_cast<ThinClientPoolHADM*>(m_tcrdm);
    GfErrType err = GF_NOTCON;
    if (poolHADM) {
      err = poolHADM->sendRequestToPrimary(fullObjectMsg, reply);
    } else {
      err = static_cast<ThinClientCacheDistributionManager*>(m_tcrdm)
                ->sendRequestToPrimary(fullObjectMsg, reply);
    }
    std::shared_ptr<Cacheable> fullObject = nullptr;
    if (err == GF_NOERR) {
      fullObject = reply.getValue();
    }
    return fullObject;
  }
}

bool CqEventImpl::getError() { return m_error; }

std::string CqEventImpl::toString() {
  return std::string("CqEvent CqName=") + m_cQuery->getName() +
         "; base operation=" + std::to_string(static_cast<int>(m_baseOp)) +
         "; cq operation=" + std::to_string(static_cast<int>(m_queryOp)) +
         "; key=" + m_key->toString() + "; value=" + m_newValue->toString();
}

std::shared_ptr<CacheableBytes> CqEventImpl::getDeltaValue() const {
  return m_deltaValue;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
