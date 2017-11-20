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

#define ROOT_NAME "testLinkage"

#include <geode/AttributesFactory.hpp>
#include "fw_helper.hpp"

using namespace apache::geode::client;

/**
 * @brief Test that we can link to all classes.
 */
BEGIN_TEST(LinkageTest)
  // just create one for now...

  AttributesFactory af;

  {
    std::shared_ptr<Cacheable> cacheablePtr;
    std::shared_ptr<CacheableKey> cacheableKeyPtr;
    std::shared_ptr<Region> regionPtr;
    AttributesMutator am(regionPtr);
    std::shared_ptr<RegionEntry> regionEntryPtr;
    std::shared_ptr<CacheableString> cacheableStringPtr;
    std::shared_ptr<Cache> cachePtr;
    // add other ptr types here...
  }
  {
    Exception e("test message");
    // all exceptions.
    IllegalArgumentException aIllegalArgumentException(
        "IllegalArgumentException");
    IllegalStateException aIllegalStateException("IllegalStateException");
    CacheExistsException aCacheExistsException("CacheExistsException");
    CacheXmlException aCacheXmlException("CacheXmlException");
    TimeoutException aTimeoutException("TimeoutException");
    CacheWriterException aCacheWriterException("CacheWriterException");
    RegionExistsException aRegionExistsException("RegionExistsException");
    CacheClosedException aCacheClosedException("CacheClosedException");
    LeaseExpiredException aLeaseExpiredException("LeaseExpiredException");
    CacheLoaderException aCacheLoaderException("CacheLoaderException");
    RegionDestroyedException aRegionDestroyedException(
        "RegionDestroyedException");
    EntryDestroyedException aEntryDestroyedException("EntryDestroyedException");
    NoSystemException aNoSystemException("NoSystemException");
    AlreadyConnectedException aAlreadyConnectedException(
        "AlreadyConnectedException");
    FileNotFoundException aFileNotFoundException("FileNotFoundException");
    InterruptedException aInterruptedException("InterruptedException");
    UnsupportedOperationException aUnsupportedOperationException(
        "UnsupportedOperationException");
    StatisticsDisabledException aStatisticsDisabledException(
        "StatisticsDisabledException");
    ConcurrentModificationException aConcurrentModificationException(
        "ConcurrentModificationException");
    UnknownException aUnknownException("UnknownException");
    ClassCastException aClassCastException("ClassCastException");
    EntryNotFoundException aEntryNotFoundException("EntryNotFoundException");
    GeodeIOException aGeodeIOException("GeodeIOException");
    GeodeConfigException aGeodeConfigException("GeodeConfigException");
    NullPointerException aNullPointerException("NullPointerException");
    EntryExistsException aEntryExistsException("EntryExistsException");
  }

  std::shared_ptr<Cache> cachePtr;
  auto cacheFactoryPtr = CacheFactory::createCacheFactory();
  cachePtr.reset(new Cache(cacheFactoryPtr->create()));
  // Cache cache;
  ASSERT((!cachePtr->isClosed()), "cache shouldn't be closed.");
  std::shared_ptr<Region> rptr;
  std::shared_ptr<Serializable> callback;
  //    CacheListener cl;
  std::shared_ptr<CacheListener> clPtr;
  //    CacheLoader cacheloader;
  std::shared_ptr<CacheLoader> cldPtr;
  //    CacheStatistics cstats; NOT yet...

  //    CacheWriter cwriter;
  std::shared_ptr<CacheWriter> cwPtr;
END_TEST(LinkageTest)
