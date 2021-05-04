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

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/PoolManager.hpp>
#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "CppCacheLibrary.hpp"
#include "DiskVersionTag.hpp"
#include "DistributedSystemImpl.hpp"
#include "PdxHelper.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "PoolAttributes.hpp"
#include "SerializationRegistry.hpp"
#include "TXCommitMessage.hpp"
#include "config.h"
#include "version.h"

namespace apache {
namespace geode {
namespace client {

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

CacheFactory::CacheFactory() noexcept
    : dsProp(nullptr), ignorePdxUnreadFields(false), pdxReadSerialized(false) {}

CacheFactory::CacheFactory(
    const std::shared_ptr<Properties>& properties) noexcept
    : dsProp(properties),
      ignorePdxUnreadFields(false),
      pdxReadSerialized(false) {}

Cache CacheFactory::create() const {
  auto cache =
      Cache(dsProp, ignorePdxUnreadFields, pdxReadSerialized, authInitialize);

  try {
    auto&& cacheXml = cache.m_cacheImpl->getDistributedSystem()
                          .getSystemProperties()
                          .cacheXMLFile();
    if (!cacheXml.empty()) {
      cache.initializeDeclarativeCache(cacheXml);
    } else {
      cache.m_cacheImpl->initServices();
    }
  } catch (const RegionExistsException&) {
    LOG_WARN("Attempt to create existing regions declaratively");
  } catch (const Exception&) {
    if (!cache.isClosed()) {
      cache.close();
    }
    throw;
  } catch (...) {
    if (!cache.isClosed()) {
      cache.close();
    }
    throw UnknownException("Exception thrown in CacheFactory::create");
  }

  auto& cacheImpl = cache.m_cacheImpl;
  const auto& serializationRegistry = cacheImpl->getSerializationRegistry();
  const auto& pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  const auto& memberListForVersionStamp =
      std::ref(*(cacheImpl->getMemberListForVersionStamp()));

  serializationRegistry->addDataSerializableFixedIdType(
      std::bind(TXCommitMessage::create, memberListForVersionStamp));

  serializationRegistry->setDataSerializablePrimitiveType(
      // TODO: This looks like the only thing to do here, but I'm not sure
      std::bind(PdxType::CreateDeserializable, std::ref(*pdxTypeRegistry)),
      DSCode::PdxType);

  serializationRegistry->addDataSerializableFixedIdType(
      std::bind(VersionTag::createDeserializable, memberListForVersionStamp));

  serializationRegistry->addDataSerializableFixedIdType(
      DSFid::DiskVersionTag, std::bind(DiskVersionTag::createDeserializable,
                                       memberListForVersionStamp));

  serializationRegistry->setPdxTypeHandler(new PdxTypeHandler());
  serializationRegistry->setDataSerializableHandler(
      new DataSerializableHandler());

  pdxTypeRegistry->setPdxIgnoreUnreadFields(cache.getPdxIgnoreUnreadFields());
  pdxTypeRegistry->setPdxReadSerialized(cache.getPdxReadSerialized());

  return cache;
}

CacheFactory& CacheFactory::set(std::string name, std::string value) {
  if (this->dsProp == nullptr) {
    this->dsProp = Properties::create();
  }
  this->dsProp->insert(std::move(name), std::move(value));
  return *this;
}

CacheFactory& CacheFactory::setAuthInitialize(
    const std::shared_ptr<AuthInitialize>& handler) {
  this->authInitialize = handler;
  return *this;
}

CacheFactory& CacheFactory::setPdxIgnoreUnreadFields(bool ignore) {
  ignorePdxUnreadFields = ignore;
  return *this;
}

CacheFactory& CacheFactory::setPdxReadSerialized(bool prs) {
  pdxReadSerialized = prs;
  return *this;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
