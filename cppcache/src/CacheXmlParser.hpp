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

#pragma once

#ifndef GEODE_CACHEXMLPARSER_H_
#define GEODE_CACHEXMLPARSER_H_

#include <map>
#include <stack>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

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
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

// Factory function typedefs to register the managed
// cacheloader/writer/listener/resolver
template <typename T>
using FactoryLoaderFn = std::function<T*(const char*, const char*)>;

class CacheXmlParser : public xercesc::DefaultHandler {
  void startElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;
  void endElement(const XMLCh* const uri, const XMLCh* const localname,
                  const XMLCh* const qname) override;
  void fatalError(const xercesc::SAXParseException&) override;

  std::map<std::string,
           std::function<void(CacheXmlParser&, const xercesc::Attributes&)>>
      start_element_map_;
  std::map<std::string, std::function<void(CacheXmlParser&)>> end_element_map_;

 private:
  std::stack<std::shared_ptr<void>> _stack;
  CacheXmlCreation* cacheCreation_;
  int32_t nestedRegions_;
  std::shared_ptr<Properties> config_;
  std::string parserMessage_;
  bool flagCacheXmlException_;
  bool flagIllegalStateException_;
  bool flagAnyOtherException_;
  bool flagExpirationAttribute_;
  std::map<std::string, RegionAttributes> namedRegions_;
  std::shared_ptr<PoolFactory> poolFactory_;

  Cache* cache_;

 public:
  explicit CacheXmlParser(Cache* cache);
  ~CacheXmlParser() override;
  static CacheXmlParser* parse(const char* cachexml, Cache* cache);
  void parseFile(const char* filename);
  void parseMemory(const char* buffer, int size);
  void setAttributes(Cache* cache);
  void create(Cache* cache);
  void endRegion();
  void startExpirationAttributes(const xercesc::Attributes& attrs);
  void startPersistenceManager(const xercesc::Attributes& attrs);
  void startPersistenceProperty(const xercesc::Attributes& attrs);
  void startRegionAttributes(const xercesc::Attributes& attrs);
  void startPdx(const xercesc::Attributes& attrs);
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
  void incNesting() { nestedRegions_++; }
  void decNesting() { nestedRegions_--; }
  bool isRootLevel() { return (nestedRegions_ == 1); }

  /** Pool handlers */
  void startPool(const xercesc::Attributes& attrs);
  void endPool();
  void startLocator(const xercesc::Attributes& attrs);
  void startServer(const xercesc::Attributes& attrs);

  // getters/setters for flags and other members
  inline bool isCacheXmlException() const { return flagCacheXmlException_; }

  inline void setCacheXmlException() { flagCacheXmlException_ = true; }

  inline bool isIllegalStateException() const {
    return flagIllegalStateException_;
  }

  inline void setIllegalStateException() { flagIllegalStateException_ = true; }

  inline bool isAnyOtherException() const { return flagAnyOtherException_; }

  inline void setAnyOtherException() { flagAnyOtherException_ = true; }

  inline bool isExpirationAttribute() const { return flagExpirationAttribute_; }

  inline void setExpirationAttribute() { flagExpirationAttribute_ = true; }

  inline const std::string& getParserMessage() const { return parserMessage_; }

  inline void setParserMessage(const std::string& str) { parserMessage_ = str; }

  // hooks for .NET managed cache listener/loader/writers
  static FactoryLoaderFn<CacheLoader> managedCacheLoaderFn_;
  static FactoryLoaderFn<CacheListener> managedCacheListenerFn_;
  static FactoryLoaderFn<PartitionResolver> managedPartitionResolverFn_;
  static FactoryLoaderFn<CacheWriter> managedCacheWriterFn_;
  static FactoryLoaderFn<PersistenceManager> managedPersistenceManagerFn_;

 private:
  std::string getOptionalAttribute(const xercesc::Attributes& attrs,
                                   const char* attributeName);
  std::string getRequiredAttribute(const xercesc::Attributes& attrs,
                                   const char* attributeName);
  std::string getLibraryName(const xercesc::Attributes& attrs);
  std::string getLibraryFunctionName(const xercesc::Attributes& attrs);

  template <typename T>
  void verifyFactoryFunction(FactoryLoaderFn<T> loader,
                             const std::string& libraryName,
                             const std::string& functionName) {
    try {
      if (loader && functionName.find('.') != std::string::npos) {
        // this is a managed library
        (loader)(libraryName.c_str(), functionName.c_str());
      } else {
        apache::geode::client::Utils::getFactoryFunction<void*()>(libraryName,
                                                                  functionName);
      }
    } catch (IllegalArgumentException& ex) {
      throw CacheXmlException(ex.what());
    }
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEXMLPARSER_H_
