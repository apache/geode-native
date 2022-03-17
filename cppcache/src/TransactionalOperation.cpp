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

#include "TransactionalOperation.hpp"

#include <geode/Cache.hpp>
#include <geode/CacheableString.hpp>
#include <geode/FunctionService.hpp>

#include "CacheImpl.hpp"
#include "RegionInternal.hpp"
#include "internal/exception.hpp"

namespace apache {
namespace geode {
namespace client {

TransactionalOperation::TransactionalOperation(
    ServerRegionOperation op, const char* regionName,
    std::shared_ptr<CacheableKey> key,
    std::shared_ptr<std::vector<std::shared_ptr<Cacheable>>> arguments)
    : m_operation(op),
      m_regionName(regionName),
      m_key(key),
      m_arguments(arguments) {}

TransactionalOperation::~TransactionalOperation() {}
std::shared_ptr<Cacheable> TransactionalOperation::replay(
    CacheImpl* cacheImpl) {
  std::shared_ptr<Cacheable> result = nullptr;

  switch (m_operation) {
    case GF_CONTAINS_KEY:
      result = CacheableBoolean::create(cacheImpl->getCache()
                                            ->getRegion(m_regionName)
                                            ->containsKeyOnServer(m_key));
      break;
    case GF_CONTAINS_VALUE:
      GfErrTypeThrowException("Contains value not supported in transaction",
                              GF_NOTSUP);
      // result = Boolean.valueOf(containsValue(args[0], true));
      break;
    case GF_CONTAINS_VALUE_FOR_KEY:
      result = CacheableBoolean::create(cacheImpl->getCache()
                                            ->getRegion(m_regionName)
                                            ->containsValueForKey(m_key));
      break;
    case GF_DESTROY:  // includes REMOVE(k,v)
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->destroy(m_key, m_arguments->at(0));
      break;
    case GF_EXECUTE_FUNCTION: {
      Execution execution;
      if (m_regionName == nullptr) {
        execution = FunctionService::onServer(*cacheImpl->getCache());
      } else {
        execution = FunctionService::onRegion(
            cacheImpl->getCache()->getRegion(m_regionName));
      }
      result = std::dynamic_pointer_cast<Cacheable>(
          execution.withArgs(m_arguments->at(0))
              .withFilter(std::dynamic_pointer_cast<CacheableVector>(
                  m_arguments->at(1)))
              .withCollector(std::dynamic_pointer_cast<ResultCollector>(
                  m_arguments->at(2)))
              .execute(m_arguments->at(3)->toString().c_str(),
                       std::chrono::milliseconds(
                           std::dynamic_pointer_cast<CacheableInt32>(
                               m_arguments->at(4))
                               ->value())));
    } break;
    case GF_GET:
      result = cacheImpl->getCache()
                   ->getRegion(m_regionName)
                   ->get(m_key, m_arguments->at(0));
      break;
    case GF_GET_ENTRY:
      GfErrTypeThrowException("getEntry not supported in transaction",
                              GF_NOTSUP);
      break;
    case GF_GET_ALL: {
      const auto allInternal =
          std::static_pointer_cast<RegionInternal>(
              cacheImpl->getCache()->getRegion(m_regionName))
              ->getAll_internal(*std::dynamic_pointer_cast<
                                    std::vector<std::shared_ptr<CacheableKey>>>(
                                    m_arguments->at(0)),
                                nullptr,
                                std::dynamic_pointer_cast<CacheableBoolean>(
                                    m_arguments->at(3))
                                    ->value());

      auto values =
          std::dynamic_pointer_cast<HashMapOfCacheable>(m_arguments->at(1));
      values->insert(allInternal.begin(), allInternal.end());
      break;
    }
    case GF_INVALIDATE:
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->invalidate(m_key, m_arguments->at(0));
      break;
    case GF_REMOVE:
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->remove(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_KEY_SET: {
      auto tmp = cacheImpl->getCache()->getRegion(m_regionName)->serverKeys();
      auto serverKeys =
          std::dynamic_pointer_cast<std::vector<std::shared_ptr<CacheableKey>>>(
              m_arguments->at(0));
      serverKeys->insert(serverKeys->end(), tmp.begin(), tmp.end());
      break;
    }
    case GF_CREATE:  // includes PUT_IF_ABSENT
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->create(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_PUT:  // includes PUT_IF_ABSENT
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->put(m_key, m_arguments->at(0), m_arguments->at(1));
      break;
    case GF_PUT_ALL:
      cacheImpl->getCache()
          ->getRegion(m_regionName)
          ->putAll(
              *std::dynamic_pointer_cast<HashMapOfCacheable>(
                  m_arguments->at(0)),
              std::chrono::milliseconds(
                  std::dynamic_pointer_cast<CacheableInt32>(m_arguments->at(1))
                      ->value()));
      break;
  }

  return result;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
