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
#define DEFAULT_SERVER_PORT 40404
#define DEFAULT_SERVER_HOST "localhost"

extern ACE_Recursive_Thread_Mutex* g_disconnectLock;

namespace apache {
namespace geode {
namespace client {

CacheFactoryPtr CacheFactory::s_factory = NULLPTR;

PoolPtr CacheFactory::createOrGetDefaultPool(CacheImpl& cacheimpl) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);

  if (cacheimpl.isClosed() == false &&
      cacheimpl.getDefaultPool() != NULLPTR) {
    return cacheimpl.getDefaultPool();
  }

  PoolPtr pool = PoolManager::find(DEFAULT_POOL_NAME);

  // if default_poolFactory is null then we are not using latest API....
  if (pool == NULLPTR && s_factory != NULLPTR) {
    pool = s_factory->determineDefaultPool(cacheimpl);
  }

  return pool;
}

CacheFactoryPtr CacheFactory::createCacheFactory(
    const PropertiesPtr& configPtr) {
  // need to create PoolFactory instance
  s_factory = new CacheFactory(configPtr);
  return s_factory;
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
  basicGetInstance(system, true, cp);
  if ((cp == NULLPTR) || (cp->isClosed() == true)) {
    Cache* cep = new Cache(name, system, id_data, ignorePdxUnreadFields,
                           readPdxSerialized);
    if (!cep) {
      throw OutOfMemoryException("Out of Memory");
    }
    cptr = cep;
    std::string key(system->getName());
    if (cp != NULLPTR) {
      ACE_Guard<ACE_Recursive_Thread_Mutex> guard(pimpl->m_lock);
      m_cacheMap.erase(m_cacheMap.find(key));
    }
    std::pair<std::string, CachePtr> pc(key, cptr);
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(pimpl->m_lock);
    m_cacheMap.insert(pc);
    return;
  }
  throw CacheExistsException("an open cache exists with the specified system");
}

CachePtr CacheFactory::getInstance(const DistributedSystemPtr& system) {
  CachePtr cptr;
  CppCacheLibrary::initLib();
  if (system == NULLPTR) {
    throw IllegalArgumentException(
        "CacheFactory::getInstance: system uninitialized");
  }
  GfErrType err = s_factory->basicGetInstance(system, false, cptr);
  GfErrTypeToException("CacheFactory::getInstance", err);
  return cptr;
}

CachePtr CacheFactory::getInstanceCloseOk(const DistributedSystemPtr& system) {
  CachePtr cptr;
  CppCacheLibrary::initLib();
  if (system == NULLPTR) {
    throw IllegalArgumentException(
        "CacheFactory::getInstanceClosedOK: system uninitialized");
  }
  GfErrType err = s_factory->basicGetInstance(system, true, cptr);
  GfErrTypeToException("CacheFactory::getInstanceCloseOk", err);
  return cptr;
}

CachePtr CacheFactory::getAnyInstance() {
  return s_factory->getAnyInstance(true);
}

CachePtr CacheFactory::getAnyInstance(bool throwException) {
  CachePtr cptr;
  CppCacheLibrary::initLib();
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(pimpl->m_lock);
  if (m_cacheMap.empty() == true) {
    if (throwException) {
      throw EntryNotFoundException(
          "CacheFactory::getAnyInstance: not found, no cache created yet");
    } else {
      return NULLPTR;
    }
  }
  for (std::map<std::string, CachePtr>::iterator p = m_cacheMap.begin();
       p != m_cacheMap.end(); ++p) {
    if (!(p->second->isClosed())) {
      cptr = p->second;
      return cptr;
    }
  }
  return NULLPTR;
}

const char* CacheFactory::getVersion() { return PRODUCT_VERSION; }

const char* CacheFactory::getProductDescription() {
  return PRODUCT_VENDOR " " PRODUCT_NAME " " PRODUCT_VERSION " (" PRODUCT_BITS
                        ") " PRODUCT_BUILDDATE;
}

CacheFactory::CacheFactory() {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  dsProp = NULLPTR;
  pf = NULLPTR;
  pimpl = std::unique_ptr<CacheFactoryImpl>(new CacheFactoryImpl());
}

CacheFactory::CacheFactory(const PropertiesPtr dsProps) {
  ignorePdxUnreadFields = false;
  pdxReadSerialized = false;
  this->dsProp = dsProps;
  this->pf = NULLPTR;
  pimpl = std::unique_ptr<CacheFactoryImpl>(new CacheFactoryImpl());
}

CachePtr CacheFactory::create() {
  // bool pdxReadSerialized = false;

  ACE_Guard<ACE_Recursive_Thread_Mutex> connectGuard(*g_disconnectLock);
  DistributedSystemPtr dsPtr = NULLPTR;

  // should we compare deafult DS properties here??
  if (DistributedSystem::isConnected()) {
    dsPtr = DistributedSystem::getInstance();
  } else {
    dsPtr = DistributedSystem::connect(DEFAULT_DS_NAME, dsProp);
    LOGFINE("CacheFactory called DistributedSystem::connect");
  }

  CachePtr cache = NULLPTR;
  basicGetInstance(dsPtr, false, cache);
  if (cache == NULLPTR)
  {
	cache = create(DEFAULT_CACHE_NAME, dsPtr,
	               dsPtr->getSystemProperties()->cacheXMLFile(), NULLPTR);
  }

  /*if (cache == NULLPTR) {
    CacheFactoryPtr cacheFac(this);
    default_CacheFactory = cacheFac;
    Cache_CreatedFromCacheFactory = true;
    cache = create(DEFAULT_CACHE_NAME, dsPtr,
                   dsPtr->getSystemProperties()->cacheXMLFile(), NULLPTR);
    // if(cache->m_cacheImpl->getDefaultPool() == NULLPTR)
    // determineDefaultPool(cache);
  } else {
    if (cache->m_cacheImpl->getDefaultPool() != NULLPTR) {
      // we already choose or created deafult pool
      determineDefaultPool(cache);
    } else {
      // not yet created, create from first cacheFactory instance
      if (default_CacheFactory != NULLPTR) {
        default_CacheFactory->determineDefaultPool(cache);
        default_CacheFactory = NULLPTR;
      }
      determineDefaultPool(cache);
    }
  }*/

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

PoolPtr CacheFactory::determineDefaultPool(CacheImpl& cacheimpl) {
  PoolPtr pool = NULLPTR;
  HashMapOfPools allPools = PoolManager::getAll();
  size_t currPoolSize = allPools.size();

  // means user has not set any pool attributes
  if (this->pf == NULLPTR) {
    this->pf = getPoolFactory();
    if (currPoolSize == 0) {
      if (!this->pf->m_addedServerOrLocator) {
        this->pf->addServer(DEFAULT_SERVER_HOST, DEFAULT_SERVER_PORT);
      }

      pool = this->pf->create(DEFAULT_POOL_NAME);
      // creatubg default pool so setting this as default pool
      LOGINFO("Set default pool with localhost:40404");
      cacheimpl.setDefaultPool(pool);
      return pool;
    } else if (currPoolSize == 1) {
      pool = allPools.begin().second();
      LOGINFO("Set default pool from existing pool.");
      cacheimpl.setDefaultPool(pool);
      return pool;
    } else {
      // can't set anything as deafult pool
      return NULLPTR;
    }
  } else {
    PoolPtr defaulPool = cacheimpl.getDefaultPool();

    if (!this->pf->m_addedServerOrLocator) {
      this->pf->addServer(DEFAULT_SERVER_HOST, DEFAULT_SERVER_PORT);
    }

    if (defaulPool != NULLPTR) {
      // once default pool is created, we will not create
      if (*(defaulPool->m_attrs) == *(this->pf->m_attrs)) {
        return defaulPool;
      } else {
        throw IllegalStateException(
            "Existing cache's default pool was not compatible");
      }
    }

    pool = NULLPTR;

    // return any existing pool if it matches
    for (HashMapOfPools::Iterator iter = allPools.begin();
         iter != allPools.end(); ++iter) {
      PoolPtr currPool(iter.second());
      if (*(currPool->m_attrs) == *(this->pf->m_attrs)) {
        return currPool;
      }
    }

    // defaul pool is null
    GF_DEV_ASSERT(defaulPool == NULLPTR);

    if (defaulPool == NULLPTR) {
      pool = this->pf->create(DEFAULT_POOL_NAME);
      LOGINFO("Created default pool");
      // creating default so setting this as defaul pool
      cacheimpl.setDefaultPool(pool);
    }

    return pool;
  }
}

PoolFactoryPtr CacheFactory::getPoolFactory() {
  if (this->pf == NULLPTR) {
    this->pf = PoolManager::createFactory();
  }
  return this->pf;
}

CacheFactory::~CacheFactory() {}
void CacheFactory::cleanup() {
  m_cacheMap.clear();
}

GfErrType CacheFactory::basicGetInstance(const DistributedSystemPtr& system,
                                         const bool closeOk, CachePtr& cptr) {
  GfErrType err = GF_NOERR;
  if (system == NULLPTR) {
    return GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION;
  }
  cptr = NULLPTR;
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(pimpl->m_lock);
  if (m_cacheMap.empty() == true) {
    return GF_CACHE_ENTRY_NOT_FOUND;
  }
  std::string key(system->getName());
  std::map<std::string, CachePtr>::iterator p = m_cacheMap.find(key);
  if (p != m_cacheMap.end()) {
    if ((closeOk == true) || (!(p->second->isClosed()))) {
      cptr = p->second;
    } else {
      return GF_CACHE_ENTRY_NOT_FOUND;
    }
  } else {
    return GF_CACHE_ENTRY_NOT_FOUND;
  }
  return err;
}

CacheFactoryPtr CacheFactory::set(const char* name, const char* value) {
  if (this->dsProp == NULLPTR) this->dsProp = Properties::create();
  this->dsProp->insert(name, value);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}

CacheFactoryPtr CacheFactory::setFreeConnectionTimeout(int connectionTimeout) {
  getPoolFactory()->setFreeConnectionTimeout(connectionTimeout);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setLoadConditioningInterval(
    int loadConditioningInterval) {
  getPoolFactory()->setLoadConditioningInterval(loadConditioningInterval);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setSocketBufferSize(int bufferSize) {
  getPoolFactory()->setSocketBufferSize(bufferSize);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setThreadLocalConnections(
    bool threadLocalConnections) {
  getPoolFactory()->setThreadLocalConnections(threadLocalConnections);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setReadTimeout(int timeout) {
  getPoolFactory()->setReadTimeout(timeout);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setMinConnections(int minConnections) {
  getPoolFactory()->setMinConnections(minConnections);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setMaxConnections(int maxConnections) {
  getPoolFactory()->setMaxConnections(maxConnections);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setIdleTimeout(long idleTimeout) {
  getPoolFactory()->setIdleTimeout(idleTimeout);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setRetryAttempts(int retryAttempts) {
  getPoolFactory()->setRetryAttempts(retryAttempts);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setPingInterval(long pingInterval) {
  getPoolFactory()->setPingInterval(pingInterval);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setUpdateLocatorListInterval(
    long updateLocatorListInterval) {
  getPoolFactory()->setUpdateLocatorListInterval(updateLocatorListInterval);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setStatisticInterval(int statisticInterval) {
  getPoolFactory()->setStatisticInterval(statisticInterval);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setServerGroup(const char* group) {
  getPoolFactory()->setServerGroup(group);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::addLocator(const char* host, int port) {
  getPoolFactory()->addLocator(host, port);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::addServer(const char* host, int port) {
  getPoolFactory()->addServer(host, port);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setSubscriptionEnabled(bool enabled) {
  getPoolFactory()->setSubscriptionEnabled(enabled);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setSubscriptionRedundancy(int redundancy) {
  getPoolFactory()->setSubscriptionRedundancy(redundancy);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setSubscriptionMessageTrackingTimeout(
    int messageTrackingTimeout) {
  getPoolFactory()->setSubscriptionMessageTrackingTimeout(
      messageTrackingTimeout);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setSubscriptionAckInterval(int ackInterval) {
  getPoolFactory()->setSubscriptionAckInterval(ackInterval);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}
CacheFactoryPtr CacheFactory::setMultiuserAuthentication(
    bool multiuserAuthentication) {
  getPoolFactory()->setMultiuserAuthentication(multiuserAuthentication);
  CacheFactoryPtr cfPtr(this);
  return cfPtr;
}

CacheFactoryPtr CacheFactory::setPRSingleHopEnabled(bool enabled) {
  getPoolFactory()->setPRSingleHopEnabled(enabled);
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
