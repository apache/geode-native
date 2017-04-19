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

#include "config.h"
#include <geode/CacheFactory.hpp>
#include <CppCacheLibrary.hpp>
#include <geode/Cache.hpp>
#include <CacheImpl.hpp>
#include <geode/SystemProperties.hpp>
#include <geode/PoolManager.hpp>
#include <PoolAttributes.hpp>
#include <CacheConfig.hpp>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <string>
#include <DistributedSystemImpl.hpp>
#include <SerializationRegistry.hpp>
#include <PdxInstantiator.hpp>
#include <PdxEnumInstantiator.hpp>
#include <PdxType.hpp>
#include <PdxTypeRegistry.hpp>
#include <CacheFactoryImpl.hpp>

#include "version.h"

#define DEFAULT_DS_NAME "default_GeodeDS"
#define DEFAULT_CACHE_NAME "default_GeodeCache"


extern ACE_Recursive_Thread_Mutex* g_disconnectLock;

namespace apache {
namespace geode {
namespace client {

CacheFactoryPtr CacheFactory::createCacheFactory(
    const PropertiesPtr& configPtr) {

  return CacheFactoryPtr(new CacheFactory(configPtr));
}

void CacheFactory::create_(const char* name, DistributedSystemPtr& system,
                           const char* id_data, CachePtr& cptr,
                           bool ignorePdxUnreadFields, bool readPdxSerialized) {
  CppCacheLibrary::initLib();

  cptr = NULLPTR;
  if (system == NULLPTR) {
    throw IllegalArgumentException(
        "CacheFactory::create: system uninitialized");
  }
  if (name == NULL) {
    throw IllegalArgumentException("CacheFactory::create: name is NULL");
  }
  if (name[0] == '\0') {
    name = "NativeCache";
  }

  CachePtr cp = NULLPTR;

  if ((cp == NULLPTR) || (cp->isClosed() == true)) {
    if ((cp == NULLPTR) || (cp->isClosed() == true)) {
      Cache *cep = new Cache(name, system, id_data, ignorePdxUnreadFields,
                             readPdxSerialized);
      if (!cep) {
        throw OutOfMemoryException("Out of Memory");
      }
      cptr = cep;
      return;
    }
    throw CacheExistsException("an open cache exists with the specified system");
  }
}



const char* CacheFactory::getVersion() { return PRODUCT_VERSION; }



CacheFactory::CacheFactory() {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  dsProp = NULLPTR;
  pimpl = std::unique_ptr<CacheFactoryImpl>(new CacheFactoryImpl());
}

CacheFactory::CacheFactory(const PropertiesPtr dsProps) {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  this->dsProp = dsProps;
  pimpl = std::unique_ptr<CacheFactoryImpl>(new CacheFactoryImpl());
}

CachePtr CacheFactory::create(DistributedSystemPtr distributedSystemPtr) {

  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);
  if ( distributedSystemPtr == NULLPTR)
  {
    GF_NEW(distributedSystemPtr, DistributedSystem(DEFAULT_DS_NAME));
    if (distributedSystemPtr->isConnected()) {
      ;;
    }
      else {
      if (distributedSystemPtr->connect(DEFAULT_DS_NAME, dsProp ) == true) {
      LOGFINE("CacheFactory called DistributedSystem::connected");
      }
    }

  }

  // should we compare deafult DS properties here??

  CachePtr cache = NULLPTR;
  if (cache == NULLPTR)
  {
	cache = create(DEFAULT_CACHE_NAME, distributedSystemPtr,
                 distributedSystemPtr->getSystemProperties()->cacheXMLFile(), NULLPTR);
  }


  SerializationRegistry::addType(GeodeTypeIdsImpl::PDX,
                                 PdxInstantiator::createDeserializable);
  SerializationRegistry::addType(GeodeTypeIds::CacheableEnum,
                                 PdxEnumInstantiator::createDeserializable);
  SerializationRegistry::addType(GeodeTypeIds::PdxType,
                                 PdxType::CreateDeserializable);
  PdxTypeRegistry::setPdxIgnoreUnreadFields(cache->getPdxIgnoreUnreadFields());
  PdxTypeRegistry::setPdxReadSerialized(cache->getPdxReadSerialized());

  return cache;
}

CachePtr CacheFactory::create(const char* name,
                              DistributedSystemPtr system /*= NULLPTR*/,
                              const char* cacheXml /*= 0*/,
                              const CacheAttributesPtr& attrs /*= NULLPTR*/) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);

  CachePtr cptr;
  CacheFactory::create_(name, system, "", cptr, ignorePdxUnreadFields,
                        pdxReadSerialized);
  cptr->m_cacheImpl->setAttributes(attrs);
  try {
    if (cacheXml != 0 && strlen(cacheXml) > 0) {
      cptr->initializeDeclarativeCache(cacheXml);
    } else {
      std::string file = system->getSystemProperties()->cacheXMLFile();
      if (file != "") {
        cptr->initializeDeclarativeCache(file.c_str());
      } else {
        cptr->m_cacheImpl->initServices();
      }
    }
  } catch (const apache::geode::client::RegionExistsException&) {
    LOGWARN("Attempt to create existing regions declaratively");
  } catch (const apache::geode::client::Exception&) {
    if (!cptr->isClosed()) {
      cptr->close();
      cptr = NULLPTR;
    }
    throw;
  } catch (...) {
    if (!cptr->isClosed()) {
      cptr->close();
      cptr = NULLPTR;
    }
    throw apache::geode::client::UnknownException(
        "Exception thrown in CacheFactory::create");
  }

  return cptr;
}

CacheFactory::~CacheFactory() {}



CacheFactoryPtr CacheFactory::set(const char* name, const char* value) {
  if (this->dsProp == NULLPTR) this->dsProp = Properties::create();
  this->dsProp->insert(name, value);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}



CacheFactoryPtr CacheFactory::setPdxIgnoreUnreadFields(bool ignore) {
  ignorePdxUnreadFields = ignore;
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}

CacheFactoryPtr CacheFactory::setPdxReadSerialized(bool prs) {
  pdxReadSerialized = prs;
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
