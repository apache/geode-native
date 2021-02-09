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

#include "CacheableWrapper.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testing {

void CacheableWrapper::initKey(int32_t, int32_t) {
  throw IllegalArgumentException("Cannot call initKey.");
}

std::map<DSCode, CacheableWrapperFunc>
    CacheableWrapperFactory::m_registeredKeyMap;
std::map<DSCode, CacheableWrapperFunc>
    CacheableWrapperFactory::m_registeredValueMap;
std::map<DSCode, std::string> CacheableWrapperFactory::m_typeIdNameMap;

CacheableWrapper* CacheableWrapperFactory::createInstance(DSCode typeId) {
  if (getValueMap().find(typeId) != getValueMap().end()) {
    CacheableWrapperFunc wrapperFunc = getValueMap()[typeId];
    return wrapperFunc();
  }
  return nullptr;
}

void CacheableWrapperFactory::registerType(
    DSCode typeId, const std::string wrapperType,
    const CacheableWrapperFunc factoryFunc, const bool isKey) {
  if (isKey) {
    getKeyMap()[typeId] = factoryFunc;
  }
  getValueMap()[typeId] = factoryFunc;
  getTypeIdNameMap()[typeId] = wrapperType;
}

std::vector<DSCode> CacheableWrapperFactory::getRegisteredKeyTypes() {
  std::vector<DSCode> keyVector;
  std::map<DSCode, CacheableWrapperFunc>::iterator keyMapIterator;

  for (keyMapIterator = getKeyMap().begin();
       keyMapIterator != getKeyMap().end(); ++keyMapIterator) {
    keyVector.push_back(keyMapIterator->first);
  }
  return keyVector;
}

std::vector<DSCode> CacheableWrapperFactory::getRegisteredValueTypes() {
  std::vector<DSCode> valueVector;
  std::map<DSCode, CacheableWrapperFunc>::iterator valueMapIterator;

  for (valueMapIterator = getValueMap().begin();
       valueMapIterator != getValueMap().end(); ++valueMapIterator) {
    valueVector.push_back(valueMapIterator->first);
  }
  return valueVector;
}

std::string CacheableWrapperFactory::getTypeForId(DSCode typeId) {
  std::map<DSCode, std::string>::iterator findType =
      getTypeIdNameMap().find(typeId);
  if (findType != getTypeIdNameMap().end()) {
    return findType->second;
  } else {
    return "";
  }
}

}  // namespace testing
}  // namespace client
}  // namespace geode
}  // namespace apache
