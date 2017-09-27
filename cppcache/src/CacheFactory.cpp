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

extern ACE_Recursive_Thread_Mutex* g_disconnectLock;

namespace apache {
namespace geode {
namespace client {

CacheFactoryPtr CacheFactory::createCacheFactory(
    const PropertiesPtr& configPtr) {
  return std::make_shared<CacheFactory>(configPtr);
}

void CacheFactory::create_(const char* name, const char* id_data,
                           CachePtr& cptr, bool readPdxSerialized) {
  cptr = nullptr;
  if (name == nullptr) {
    throw IllegalArgumentException("CacheFactory::create: name is nullptr");
  }
  if (name[0] == '\0') {
    name = "NativeCache";
  }

  cptr = std::make_shared<Cache>(name, dsProp, ignorePdxUnreadFields,
                                 readPdxSerialized, authInitialize);
}  // namespace client

const char* CacheFactory::getVersion() { return PRODUCT_VERSION; }

const char* CacheFactory::getProductDescription() {
  return PRODUCT_VENDOR " " PRODUCT_NAME " " PRODUCT_VERSION " (" PRODUCT_BITS
                        ") " PRODUCT_BUILDDATE;
}

CacheFactory::CacheFactory() {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  dsProp = nullptr;
}

CacheFactory::CacheFactory(const PropertiesPtr dsProps) {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  this->dsProp = dsProps;
}

CachePtr CacheFactory::create() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);

  LOGFINE("CacheFactory called DistributedSystem::connect");
  auto cache = create(DEFAULT_CACHE_NAME, nullptr);

  auto& cacheImpl = cache->m_cacheImpl;
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

  pdxTypeRegistry->setPdxIgnoreUnreadFields(cache->getPdxIgnoreUnreadFields());
  pdxTypeRegistry->setPdxReadSerialized(cache->getPdxReadSerialized());

  return cache;
}

CachePtr CacheFactory::create(const char* name,
                              const CacheAttributesPtr& attrs /*= nullptr*/) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);

  CachePtr cptr;
  create_(name, "", cptr, pdxReadSerialized);
  cptr->m_cacheImpl->setAttributes(attrs);
  try {
    const char* cacheXml =
        cptr->getDistributedSystem().getSystemProperties().cacheXMLFile();
    if (cacheXml != 0 && strlen(cacheXml) > 0) {
      cptr->initializeDeclarativeCache(cacheXml);
    } else {
      cptr->m_cacheImpl->initServices();
    }
  } catch (const apache::geode::client::RegionExistsException&) {
    LOGWARN("Attempt to create existing regions declaratively");
  } catch (const apache::geode::client::Exception&) {
    if (!cptr->isClosed()) {
      cptr->close();
      cptr = nullptr;
    }
    throw;
  } catch (...) {
    if (!cptr->isClosed()) {
      cptr->close();
      cptr = nullptr;
    }
    throw apache::geode::client::UnknownException(
        "Exception thrown in CacheFactory::create");
  }

  return cptr;
}

CacheFactory::~CacheFactory() {}

CacheFactoryPtr CacheFactory::set(const char* name, const char* value) {
  if (this->dsProp == nullptr) {
    this->dsProp = Properties::create();
  }
  this->dsProp->insert(name, value);
  return shared_from_this();
}

CacheFactoryPtr CacheFactory::setAuthInitialize(
    const AuthInitializePtr& authInitialize) {
  this->authInitialize = authInitialize;
  return shared_from_this();
}

CacheFactoryPtr CacheFactory::setPdxIgnoreUnreadFields(bool ignore) {
  ignorePdxUnreadFields = ignore;
  return shared_from_this();
}

CacheFactoryPtr CacheFactory::setPdxReadSerialized(bool prs) {
  pdxReadSerialized = prs;
  return shared_from_this();
}
}  // namespace client
}  // namespace geode
}  // namespace apache
