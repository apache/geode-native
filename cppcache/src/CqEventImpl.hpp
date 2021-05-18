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

#ifndef GEODE_CQEVENTIMPL_H_
#define GEODE_CQEVENTIMPL_H_

#include <string>

#include <geode/CacheableKey.hpp>
#include <geode/CqEvent.hpp>
#include <geode/CqOperation.hpp>
#include <geode/CqQuery.hpp>
#include <geode/Exception.hpp>
#include <geode/Serializable.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

/**
 * @class CqEventImpl EventImpl.hpp
 *
 * Interface for CqEvent. Offers methods to get information from
 * CqEvent.
 */

class ThinClientBaseDM;
class EventId;
class CqEventImpl : public CqEvent {
 public:
  CqEventImpl() = delete;
  CqEventImpl(std::shared_ptr<CqQuery>& cQuery, CqOperation baseOp,
              CqOperation cqOp, std::shared_ptr<CacheableKey>& key,
              std::shared_ptr<Cacheable>& value, ThinClientBaseDM* tcrdm,
              std::shared_ptr<CacheableBytes> deltaBytes,
              std::shared_ptr<EventId> eventId);
  ~CqEventImpl() override = default;

  std::shared_ptr<CqQuery> getCq() const override;

  /**
   * Get the operation on the base region that triggered this event.
   */
  CqOperation getBaseOperation() const override;

  /**
   * Get the the operation on the query results. Supported operations include
   * update, create, and destroy.
   */
  CqOperation getQueryOperation() const override;

  /**
   * Get the key relating to the event.
   * @return Object key.
   */
  std::shared_ptr<CacheableKey> getKey() const override;

  /**
   * Get the new value of the modification.
   *  If there is no new value because this is a delete, then
   *  return null.
   */
  std::shared_ptr<Cacheable> getNewValue() const override;

  bool getError();

  std::string toString();

  std::shared_ptr<CacheableBytes> getDeltaValue() const override;

 private:
  std::shared_ptr<CqQuery> m_cQuery;
  CqOperation m_baseOp;
  CqOperation m_queryOp;
  std::shared_ptr<CacheableKey> m_key;
  std::shared_ptr<Cacheable> m_newValue;
  bool m_error;
  ThinClientBaseDM* m_tcrdm;
  std::shared_ptr<CacheableBytes> m_deltaValue;
  std::shared_ptr<EventId> m_eventId;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQEVENTIMPL_H_
