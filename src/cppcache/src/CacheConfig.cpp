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

/**
 * @file
 */

// CacheConfig.cpp: implementation of the CacheConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "CacheConfig.hpp"

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace apache {
namespace geode {
namespace client {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CacheConfig::CacheConfig(const char* xmlFileName)
    : m_doc(nullptr),
      m_root_element(nullptr)

{
  m_doc = xmlParseFile(xmlFileName);
  if (m_doc == nullptr) {
    throw IllegalArgumentException("Cacheconfig : xmlParseFile");
  }
  m_root_element = xmlDocGetRootElement(m_doc);
  if (m_root_element == nullptr) {
    throw IllegalArgumentException("Cacheconfig : xmlDocGetRootElement");
  }
  if (!parse()) throw IllegalArgumentException("Cacheconfig : parse error");
  ;
}

CacheConfig::~CacheConfig() {
  if (m_doc) xmlFreeDoc(m_doc);
  xmlCleanupParser();
}

bool CacheConfig::parse() {
  if (strcmp(reinterpret_cast<const char*>(m_root_element->name), "cache") ==
      0) {
    xmlNode* cur_node = nullptr;

    for (cur_node = m_root_element->children; cur_node;
         cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        if (strcmp(reinterpret_cast<const char*>(cur_node->name),
                   "root-region") == 0) {
          parseRegion(cur_node);
        }
      }
    }
    return true;
  } else {
    return false;
  }
}

bool CacheConfig::parseRegion(xmlNode* node) {
  xmlChar* name =
      xmlGetNoNsProp(node, reinterpret_cast<const unsigned char*>("name"));

  if (name != nullptr) {
    xmlNode* cur_node = nullptr;

    for (cur_node = node->children; cur_node; cur_node = cur_node->next) {
      if (cur_node->type == XML_ELEMENT_NODE) {
        if (strcmp(reinterpret_cast<const char*>(cur_node->name),
                   "region-attributes") == 0) {
          parseAttributes(reinterpret_cast<const char*>(name), cur_node);
        }
      }
    }
    return true;
  }
  return false;
}

bool CacheConfig::parseAttributes(const char* name, xmlNode* node) {
  xmlChar* scope =
      xmlGetNoNsProp(node, reinterpret_cast<const unsigned char*>("scope"));
  xmlChar* initialCapacity = xmlGetNoNsProp(
      node, reinterpret_cast<const unsigned char*>("initial-capacity"));
  xmlChar* lruLimit = xmlGetNoNsProp(
      node, reinterpret_cast<const unsigned char*>("lru-entries-limit"));
  xmlChar* concurrency = xmlGetNoNsProp(
      node, reinterpret_cast<const unsigned char*>("concurrency-level"));
  xmlChar* caching = xmlGetNoNsProp(
      node, reinterpret_cast<const unsigned char*>("caching-enabled"));

  std::string scopeStr =
      (scope == nullptr ? "invalid" : reinterpret_cast<const char*>(scope));
  std::string initialCapacityStr =
      (initialCapacity == nullptr
           ? "1000"
           : reinterpret_cast<const char*>(initialCapacity));
  std::string limitStr =
      (lruLimit == nullptr ? "0" : reinterpret_cast<const char*>(lruLimit));
  std::string concStr =
      (concurrency == nullptr ? "0"
                              : reinterpret_cast<const char*>(concurrency));
  std::string cachingStr =
      (caching == nullptr ? "true" : reinterpret_cast<const char*>(caching));

  auto reg = std::make_shared<RegionConfig>(scopeStr, initialCapacityStr);

  reg->setLru(limitStr);
  reg->setConcurrency(concStr);
  reg->setCaching(cachingStr);

  m_regionList.insert(RegionConfigMapT::value_type(name, reg));

  return true;
}

RegionConfigMapT& CacheConfig::getRegionList() { return m_regionList; }
}  // namespace client
}  // namespace geode
}  // namespace apache
