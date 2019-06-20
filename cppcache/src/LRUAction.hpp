#pragma once

#ifndef GEODE_LRUACTION_H_
#define GEODE_LRUACTION_H_

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

#include <geode/Cache.hpp>
#include <geode/PersistenceManager.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheableToken.hpp"
#include "MapEntry.hpp"
#include "RegionInternal.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @brief abstract behavior for different eviction actions.
 */
class LRUEntriesMap;
class APACHE_GEODE_EXPORT LRUAction {
 protected:
  bool m_invalidates;
  bool m_destroys;
  bool m_distributes;
  bool m_overflows;

  LRUAction() {
    m_invalidates = false;
    m_destroys = false;
    m_distributes = false;
    m_overflows = false;
  }

 public:
  // types of action

  typedef enum {
    /** When the region or cached object expires, it is invalidated. */
    INVALIDATE = 0,
    /** When expired, invalidated locally only. */
    LOCAL_INVALIDATE,

    /** When the region or cached object expires, it is destroyed. */
    DESTROY,
    /** When expired, destroyed locally only. */
    LOCAL_DESTROY,

    /** invalid type. */
    INVALID_ACTION,

    /** over flow type */
    OVERFLOW_TO_DISK
  } Action;

 public:
  static LRUAction* newLRUAction(const LRUAction::Action& lruAction,
                                 RegionInternal* regionPtr,
                                 LRUEntriesMap* entriesMapPtr);

  virtual ~LRUAction() = default;

  virtual bool evict(const std::shared_ptr<MapEntryImpl>& mePtr) = 0;

  virtual LRUAction::Action getType() = 0;

  bool invalidates();

  bool destroys();

  bool distributes();

  bool overflows();
};

/**
 * @brief LRUAction for destroy (distributed)
 */
class APACHE_GEODE_EXPORT LRUDestroyAction : public virtual LRUAction {
 private:
  RegionInternal* m_regionPtr;

  explicit LRUDestroyAction(RegionInternal* regionPtr);

 public:
  virtual ~LRUDestroyAction() = default;

  virtual bool evict(const std::shared_ptr<MapEntryImpl>& mePtr);

  virtual LRUAction::Action getType();

  friend class LRUAction;
};

/**
 * @brief LRUAction for invalidate.
 */
class APACHE_GEODE_EXPORT LRULocalInvalidateAction : public virtual LRUAction {
 private:
  RegionInternal* m_regionPtr;

  explicit LRULocalInvalidateAction(RegionInternal* regionPtr);

 public:
  virtual ~LRULocalInvalidateAction() = default;

  virtual bool evict(const std::shared_ptr<MapEntryImpl>& mePtr);

  virtual LRUAction::Action getType();

  friend class LRUAction;
};

/**
 * @brief LRUAction for invalidate.
 */
class APACHE_GEODE_EXPORT LRUOverFlowToDiskAction : public virtual LRUAction {
 private:
  RegionInternal* m_regionPtr;
  LRUEntriesMap* m_entriesMapPtr;

  LRUOverFlowToDiskAction(RegionInternal* regionPtr,
                          LRUEntriesMap* entriesMapPtr);

 public:
  virtual ~LRUOverFlowToDiskAction() = default;

  virtual bool evict(const std::shared_ptr<MapEntryImpl>& mePtr);

  virtual LRUAction::Action getType();

  friend class LRUAction;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_LRUACTION_H_
