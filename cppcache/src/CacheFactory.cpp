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

#include <functional>
#include <map>
#include <string>

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <geode/CacheFactory.hpp>
#include <geode/Cache.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>

#include "config.h"
#include "version.h"

#include "CacheImpl.hpp"
#include "CppCacheLibrary.hpp"
#include "PoolAttributes.hpp"
#include "CacheConfig.hpp"
#include "DistributedSystemImpl.hpp"
#include "SerializationRegistry.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "DiskVersionTag.hpp"
#include "TXCommitMessage.hpp"
#include "PdxHelper.hpp"

#define DEFAULT_CACHE_NAME "default_GeodeCache"

namespace apache {
namespace geode {
namespace client {

std::shared_ptr<CacheFactory> CacheFactory::createCacheFactory(
    const std::shared_ptr<Properties>& configPtr) {
  return std::make_shared<CacheFactory>(configPtr);
}

const std::string& CacheFactory::getVersion() {
  static std::string version{PRODUCT_VERSION};
  return version;
}

const std::string& CacheFactory::getProductDescription() {
  static std::string description{PRODUCT_VENDOR
                                 " " PRODUCT_NAME " " PRODUCT_VERSION
                                 " (" PRODUCT_BITS ") " PRODUCT_BUILDDATE};
  return description;
}

CacheFactory::CacheFactory()
    : ignorePdxUnreadFields(false), pdxReadSerialized(false), dsProp(nullptr) {}

CacheFactory::CacheFactory(const std::shared_ptr<Properties> dsProps) {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  this->dsProp = dsProps;
}

Cache CacheFactory::create() const {
  LOGFINE("CacheFactory called DistributedSystem::connect");
  auto cache = create(DEFAULT_CACHE_NAME, nullptr);

  auto& cacheImpl = cache.m_cacheImpl;
  const auto& serializationRegistry = cacheImpl->getSerializationRegistry();
  const auto& pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  const auto& memberListForVersionStamp =
      std::ref(*(cacheImpl->getMemberListForVersionStamp()));

  serializationRegistry->addType2(
      std::bind(TXCommitMessage::create, memberListForVersionStamp));

  serializationRegistry->addType(
      GeodeTypeIds::PdxType,
      std::bind(PdxType::CreateDeserializable, pdxTypeRegistry));

  serializationRegistry->addType(
      std::bind(VersionTag::createDeserializable, memberListForVersionStamp));

  serializationRegistry->addType2(
      GeodeTypeIdsImpl::DiskVersionTag,
      std::bind(DiskVersionTag::createDeserializable,
                memberListForVersionStamp));

  serializationRegistry->setPdxTypeHandler([](DataInput& dataInput) {
    return PdxHelper::deserializePdx(dataInput, false);
  });

  pdxTypeRegistry->setPdxIgnoreUnreadFields(cache.getPdxIgnoreUnreadFields());
  pdxTypeRegistry->setPdxReadSerialized(cache.getPdxReadSerialized());

  return cache;
}

Cache CacheFactory::create(
    std::string name,
    const std::shared_ptr<CacheAttributes>& attrs /*= nullptr*/) const {
  auto cache = Cache(std::move(name), dsProp, ignorePdxUnreadFields,
                     pdxReadSerialized, authInitialize);
  cache.m_cacheImpl->setAttributes(attrs);

  try {
    auto&& cacheXml =
        cache.getDistributedSystem().getSystemProperties().cacheXMLFile();
    if (!cacheXml.empty()) {
      cache.initializeDeclarativeCache(cacheXml);
    } else {
      cache.m_cacheImpl->initServices();
    }
  } catch (const apache::geode::client::RegionExistsException&) {
    LOGWARN("Attempt to create existing regions declaratively");
  } catch (const apache::geode::client::Exception&) {
    if (!cache.isClosed()) {
      cache.close();
    }
    throw;
  } catch (...) {
    if (!cache.isClosed()) {
      cache.close();
    }
    throw apache::geode::client::UnknownException(
        "Exception thrown in CacheFactory::create");
  }

  return cache;
}

std::shared_ptr<CacheFactory> CacheFactory::set(const std::string& name,
                                                const std::string& value) {
  if (this->dsProp == nullptr) {
    this->dsProp = Properties::create();
  }
  this->dsProp->insert(name, value);
  return shared_from_this();
}
 std::shared_ptr<CacheFactory> CacheFactory::setAuthInitialize(
    const std::shared_ptr<AuthInitialize>& handler) {
  this->authInitialize = handler;
  return shared_from_this();
 }
 std::shared_ptr<CacheFactory> CacheFactory::setPdxIgnoreUnreadFields(
     bool ignore) {
   ignorePdxUnreadFields = ignore;
   return shared_from_this();
}
std::shared_ptr<CacheFactory> CacheFactory::setPdxReadSerialized(bool prs) {
  pdxReadSerialized = prs;
  return shared_from_this();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
