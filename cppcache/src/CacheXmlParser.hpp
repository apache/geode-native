#pragma once

#ifndef GEODE_CACHEXMLPARSER_H_
#define GEODE_CACHEXMLPARSER_H_

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

#include <map>
#include <stack>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include <geode/Cache.hpp>
#include <geode/CacheListener.hpp>
#include <geode/CacheLoader.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/ExpirationAction.hpp>
#include <geode/ExpirationAttributes.hpp>
#include <geode/PartitionResolver.hpp>
#include <geode/RegionShortcut.hpp>

#include "CacheXmlCreation.hpp"
#include "RegionXmlCreation.hpp"

namespace apache {
namespace geode {
namespace client {

// Factory function typedefs to register the managed
// cacheloader/writer/listener/resolver
typedef CacheLoader* (*LibraryCacheLoaderFn)(const char* assemblyPath,
                                             const char* factFuncName);
typedef CacheListener* (*LibraryCacheListenerFn)(const char* assemblyPath,
                                                 const char* factFuncName);
typedef PartitionResolver* (*LibraryPartitionResolverFn)(
    const char* assemblyPath, const char* factFuncName);
typedef CacheWriter* (*LibraryCacheWriterFn)(const char* assemblyPath,
                                             const char* factFuncName);
typedef PersistenceManager* (*LibraryPersistenceManagerFn)(
    const char* assemblyPath, const char* factFuncName);

class APACHE_GEODE_EXPORT CacheXmlParser : public xercesc::DefaultHandler {
  void startElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname, const xercesc::Attributes& attrs);
  void endElement(const XMLCh* const uri, const XMLCh* const localname,
                  const XMLCh* const qname);
  void fatalError(const xercesc::SAXParseException&);

  std::map<std::string,
           std::function<void(CacheXmlParser&, const xercesc::Attributes&)>>
      start_element_map_;
  std::map<std::string, std::function<void(CacheXmlParser&)>> end_element_map_;

 private:
  std::stack<std::shared_ptr<void>> _stack;
  CacheXmlCreation* m_cacheCreation;
  std::string m_error;
  int32_t m_nestedRegions;
  std::shared_ptr<Properties> m_config;
  std::string m_parserMessage;
  bool m_flagCacheXmlException;
  bool m_flagIllegalStateException;
  bool m_flagAnyOtherException;
  bool m_flagExpirationAttribute;
  std::map<std::string, RegionAttributes> namedRegions;
  std::shared_ptr<PoolFactory> m_poolFactory;

  Cache* m_cache;
  /** Pool helper */
  void setPoolInfo(PoolFactory* poolFactory, const char* name,
                   const char* value);

  void handleParserErrors(int res);

 public:
  explicit CacheXmlParser(Cache* cache);
  ~CacheXmlParser();
  static CacheXmlParser* parse(const char* cachexml, Cache* cache);
  void parseFile(const char* filename);
  void parseMemory(const char* buffer, int size);
  void setAttributes(Cache* cache);
  void create(Cache* cache);
  void endRegion();
  void startExpirationAttributes(const xercesc::Attributes& attrs);
  void startPersistenceManager(const xercesc::Attributes& attrs);
  void startPersistenceProperties(const xercesc::Attributes& attrs);
  void startRegionAttributes(const xercesc::Attributes& attrs);
  void startPdx(const xercesc::Attributes& attrs);
  void endPdx();
  void startRegion(const xercesc::Attributes& attrs);
  void startCache(const xercesc::Attributes& attrs);
  void endCache();
  void startCacheLoader(const xercesc::Attributes& attrs);
  void startCacheListener(const xercesc::Attributes& attrs);
  void startPartitionResolver(const xercesc::Attributes& attrs);
  void startCacheWriter(const xercesc::Attributes& attrs);
  void endEntryIdleTime();
  void endEntryTimeToLive();
  void endRegionIdleTime();
  void endRegionTimeToLive();
  void endRegionAttributes();
  void endPersistenceManager();
  void setError(const std::string& s);
  const std::string& getError() const;
  void incNesting() { m_nestedRegions++; }
  void decNesting() { m_nestedRegions--; }
  bool isRootLevel() { return (m_nestedRegions == 1); }

  /** Pool handlers */
  void startPool(const xercesc::Attributes& attrs);
  void endPool();
  void startLocator(const xercesc::Attributes& attrs);
  void startServer(const xercesc::Attributes& attrs);

  // getters/setters for flags and other members
  inline bool isCacheXmlException() const { return m_flagCacheXmlException; }

  inline void setCacheXmlException() { m_flagCacheXmlException = true; }

  inline bool isIllegalStateException() const {
    return m_flagIllegalStateException;
  }

  inline void setIllegalStateException() { m_flagIllegalStateException = true; }

  inline bool isAnyOtherException() const { return m_flagAnyOtherException; }

  inline void setAnyOtherException() { m_flagAnyOtherException = true; }

  inline bool isExpirationAttribute() const {
    return m_flagExpirationAttribute;
  }

  inline void setExpirationAttribute() { m_flagExpirationAttribute = true; }

  inline const std::string& getParserMessage() const { return m_parserMessage; }

  inline void setParserMessage(const std::string& str) {
    m_parserMessage = str;
  }

  // hooks for .NET managed cache listener/loader/writers
  static LibraryCacheLoaderFn managedCacheLoaderFn;
  static LibraryCacheListenerFn managedCacheListenerFn;
  static LibraryPartitionResolverFn managedPartitionResolverFn;
  static LibraryCacheWriterFn managedCacheWriterFn;
  static LibraryPersistenceManagerFn managedPersistenceManagerFn;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEXMLPARSER_H_
