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

// Standard headers
#include <memory>
#include <string>
#include <iostream>

// C++ client public headers
#include "geode/CacheFactory.hpp"

// C++ client private headers
#include "util/Log.hpp"

// C client public headers
#include "geode/cache.h"
#include "geode/cache/factory.h"
#include "geode/client.h"

// C client private headers
#include "auth_initialize.hpp"
#include "cache.hpp"
#include "client.hpp"
#include "factory.hpp"

CacheFactoryWrapper::CacheFactoryWrapper(apache_geode_client_t* client)
    : ClientKeeper{reinterpret_cast<ClientWrapper*>(client)} {
  AddRecord(this, "CacheFactoryWrapper");
  cacheFactory_.set("log-level", "debug");
  std::cout << __FUNCTION__ << " " << static_cast<void*>(this) << "\n";
}

CacheFactoryWrapper::~CacheFactoryWrapper() {
  RemoveRecord(this);
}

const char* CacheFactoryWrapper::getVersion() {
  return cacheFactory_.getVersion().c_str();
}

const char* CacheFactoryWrapper::getProductDescription() {
  return cacheFactory_.getProductDescription().c_str();
}

void CacheFactoryWrapper::setPdxIgnoreUnreadFields(bool pdxIgnoreUnreadFields) {
  std::cout << __FUNCTION__ << " " << static_cast<void*>(this) << "\n";
  cacheFactory_.setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void CacheFactoryWrapper::setAuthInitialize(
    void (*getCredentials)(apache_geode_properties_t*), void (*close)()) {
  authInit_ = std::make_shared<AuthInitializeWrapper>(getCredentials, close);
  cacheFactory_.setAuthInitialize(authInit_);
}

void CacheFactoryWrapper::setPdxReadSerialized(bool pdxReadSerialized) {
  cacheFactory_.setPdxReadSerialized(pdxReadSerialized);
}

void CacheFactoryWrapper::setProperty(const std::string& key,
                                      const std::string& value) {
  cacheFactory_.set(key, value);
}

CacheWrapper* CacheFactoryWrapper::createCache() {
  std::cout << __FUNCTION__ << " " << static_cast<void*>(this) << "\n";
  return new CacheWrapper(this, cacheFactory_.create());
}

apache_geode_cache_factory_t* apache_geode_CreateCacheFactory(
    apache_geode_client_t* client) {
  auto factory = new CacheFactoryWrapper(client);
  std::cout << __FUNCTION__ << " factory: " << static_cast<void*>(factory) << "\n";
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  return reinterpret_cast<apache_geode_cache_factory_t*>(factory);
}

apache_geode_cache_t* apache_geode_CacheFactory_CreateCache(
    apache_geode_cache_factory_t* factory) {
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  CacheWrapper* cache = cacheFactory->createCache();
  std::cout << __FUNCTION__ << " factory: " << static_cast<void*>(factory) << "cache: " << static_cast<void*>(cache) << "\n";
  LOGDEBUG("%s: factory=%p, cache=%p", __FUNCTION__, factory, cache);
  return reinterpret_cast<apache_geode_cache_t*>(cache);
}

const char* apache_geode_CacheFactory_GetVersion(
    apache_geode_cache_factory_t* factory) {
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getVersion();
}

const char* apache_geode_CacheFactory_GetProductDescription(
    apache_geode_cache_factory_t* factory) {
  LOGDEBUG("%s: factory=%p", __FUNCTION__, factory);
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  return cacheFactory->getProductDescription();
}

void apache_geode_CacheFactory_SetPdxIgnoreUnreadFields(
    apache_geode_cache_factory_t* factory, bool pdxIgnoreUnreadFields) {
  auto cacheFactory =
      reinterpret_cast<CacheFactoryWrapper*>(factory);
  auto ignoreUnreadFields = pdxIgnoreUnreadFields ? "true" : "false";
  std::cout << __FUNCTION__ << " factory: " << static_cast<void*>(factory) << "\n";
  LOGDEBUG("%s: factory=%p, ignoreUnreadFields=%s", __FUNCTION__, factory,
           ignoreUnreadFields);
  cacheFactory->setPdxIgnoreUnreadFields(pdxIgnoreUnreadFields);
}

void apache_geode_CacheFactory_SetAuthInitialize(
    apache_geode_cache_factory_t* factory,
    void (*getCredentials)(apache_geode_properties_t*), void (*close)()) {
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  LOGDEBUG("%s: factory=%p, getCredentials=%p, close=%p", __FUNCTION__, factory,
           getCredentials, close);
  cacheFactory->setAuthInitialize(getCredentials, close);
}

void apache_geode_CacheFactory_SetPdxReadSerialized(
    apache_geode_cache_factory_t* factory, bool pdxReadSerialized) {
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  auto readSerialized = pdxReadSerialized ? "true" : "false";
  LOGDEBUG("%s: factory=%p, readSerialized=%s", __FUNCTION__, factory,
           readSerialized);
  cacheFactory->setPdxReadSerialized(pdxReadSerialized);
}

void apache_geode_CacheFactory_SetProperty(
    apache_geode_cache_factory_t* factory, const char* key, const char* value) {
  LOGDEBUG("%s: factory=%p, (k, v)=(%s, %s)", __FUNCTION__, factory,
           std::string(key).c_str(), std::string(value).c_str());
  auto cacheFactory = reinterpret_cast<CacheFactoryWrapper*>(factory);
  cacheFactory->setProperty(key, value);
}

void apache_geode_DestroyCacheFactory(apache_geode_cache_factory_t* factory) {
  std::cout << __FUNCTION__ << " factory: " << static_cast<void*>(factory) << "\n";
  delete reinterpret_cast<CacheFactoryWrapper*>(factory);
}
