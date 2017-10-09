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
/*
 * TransactionalOperation.cpp
 *
 *  Created on: 10-May-2011
 *      Author: ankurs
 */

#include "TransactionalOperation.hpp"
#include <geode/CacheableString.hpp>
#include <geode/Cache.hpp>
#include <geode/FunctionService.hpp>

#include "RegionInternal.hpp"

namespace apache {
namespace geode {
namespace client {

TransactionalOperation::TransactionalOperation(ServerRegionOperation op,
                                               const char* regionName,
                                               CacheableKeyPtr key,
                                               VectorOfCacheablePtr arguments)
    : m_operation(op),
      m_regionName(regionName),
      m_key(key),
      m_arguments(arguments) {}

TransactionalOperation::~TransactionalOperation() {}

CacheablePtr TransactionalOperation::replay(Cache* cache) {
  CacheablePtr result = nullptr;

  switch (m_operation) {
    case GF_CONTAINS_KEY:
      result = CacheableBoolean::create(
          cache->getRegion(m_regionName)->containsKeyOnServer(m_key));
      break;
    case GF_CONTAINS_VALUE:
      GfErrTypeThrowException("Contains value not supported in transaction",
                              GF_NOTSUP);
      // result = Boolean.valueOf(containsValue(args[0], true));
      break;
    case GF_CONTAINS_VALUE_FOR_KEY:
      result = CacheableBoolean::create(
          cache->getRegion(m_regionName)->containsValueForKey(m_key));
      break;
    case GF_DESTROY:  // includes REMOVE(k,v)
      cache->getRegion(m_regionName)->destroy(m_key, m_arguments->at(0));
      break;
    case GF_EXECUTE_FUNCTION: {
      ExecutionPtr execution;
      if (m_regionName == nullptr) {
        execution = FunctionService::onServer(CachePtr(cache));
      } else {
        execution = FunctionService::onRegion(cache->getRegion(m_regionName));
      }
      result = std::dynamic_pointer_cast<Cacheable>(
          execution->withArgs(m_arguments->at(0))
              ->withFilter(
                  std::static_pointer_cast<CacheableVector>(m_arguments->at(1)))
              ->withCollector(std::dynamic_pointer_cast<ResultCollector>(
                  m_arguments->at(2)))
              ->execute(
                  m_arguments->at(3)->toString()->asChar(),
                  std::static_pointer_cast<CacheableInt32>(m_arguments->at(4))
                      ->value()));
    } break;
    case GF_GET:
      result = cache->getRegion(m_regionName)->get(m_key, m_arguments->at(0));
      break;
    case GF_GET_ENTRY:
      GfErrTypeThrowException("getEntry not supported in transaction",
                              GF_NOTSUP);
      break;
    case GF_GET_ALL: {
      const auto result =
          std::static_pointer_cast<RegionInternal>(
              cache->getRegion(m_regionName))
              ->getAll_internal(
                  *std::dynamic_pointer_cast<VectorOfCacheableKey>(
                      m_arguments->at(0)),
                  nullptr,
                  std::static_pointer_cast<CacheableBoolean>(m_arguments->at(3))
                      ->value());

      auto values =
          std::dynamic_pointer_cast<HashMapOfCacheable>(m_arguments->at(1));
      values->insert(result.begin(), result.end());
      break;
    }
    case GF_INVALIDATE:
      cache->getRegion(m_regionName)->invalidate(m_key, m_arguments->at(0));
      break;
    case GF_REMOVE:
      cache->getRegion(m_regionName)
          ->remove(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_KEY_SET: {
      auto tmp = cache->getRegion(m_regionName)->serverKeys();
      auto serverKeys =
          std::dynamic_pointer_cast<VectorOfCacheableKey>(m_arguments->at(0));
      serverKeys->insert(serverKeys->end(), tmp.begin(), tmp.end());
      break;
    }
    case GF_CREATE:  // includes PUT_IF_ABSENT
      cache->getRegion(m_regionName)
          ->create(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_PUT:  // includes PUT_IF_ABSENT
      cache->getRegion(m_regionName)
          ->put(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_PUT_ALL:
      cache->getRegion(m_regionName)
          ->putAll(*std::dynamic_pointer_cast<HashMapOfCacheable>(
                       m_arguments->at(0)),
                   std::static_pointer_cast<CacheableInt32>(m_arguments->at(1))
                       ->value());
      break;
    default:
      throw UnsupportedOperationException(
          "unknown operation encountered during transaction replay");
  }

  return result;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
