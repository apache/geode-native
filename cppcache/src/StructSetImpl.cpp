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

#include <vector>
#include <stdexcept>

#include "StructSetImpl.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

StructSetImpl::StructSetImpl(const std::shared_ptr<CacheableVector>& response,
                             const std::vector<std::string>& fieldNames) {
  m_nextIndex = 0;

  size_t i = 0;
  for (auto&& fieldName : fieldNames) {
    LOGDEBUG("StructSetImpl: pushing fieldName = %s with index = %zd",
             fieldName.c_str(), i);
    m_fieldNameIndexMap.emplace(fieldName, i++);
  }

  auto numOfValues = response->size();
  size_t valStoredCnt = 0;

  const auto numOfFields = fieldNames.size();
  m_structVector = CacheableVector::create();
  while (valStoredCnt < numOfValues) {
    std::vector<std::shared_ptr<Serializable>> tmpVec;
    for (size_t i = 0; i < numOfFields; i++) {
      tmpVec.push_back(response->operator[](valStoredCnt++));
    }
    auto siPtr = std::make_shared<Struct>(this, tmpVec);
    m_structVector->push_back(siPtr);
  }
}

bool StructSetImpl::isModifiable() const { return false; }

size_t StructSetImpl::size() const { return m_structVector->size(); }

const std::shared_ptr<Serializable> StructSetImpl::operator[](
    size_t index) const {
  if (index >= m_structVector->size()) {
    throw IllegalArgumentException("Index out of bounds");
  }

  return m_structVector->operator[](index);
}

SelectResultsIterator StructSetImpl::getIterator() {
  return SelectResultsIterator(m_structVector, shared_from_this());
}

size_t StructSetImpl::getFieldIndex(const std::string& fieldname) {
  const auto& iter = m_fieldNameIndexMap.find(fieldname);
  if (iter != m_fieldNameIndexMap.end()) {
    return iter->second;
  } else {
    throw std::invalid_argument("fieldname not found");
  }
}

const std::string& StructSetImpl::getFieldName(int32_t index) {
  for (const auto& iter : m_fieldNameIndexMap) {
    if (iter.second == index) return (iter.first);
  }

  throw std::out_of_range("Struct: fieldName not found.");
}

SelectResults::Iterator StructSetImpl::begin() const {
  return m_structVector->begin();
}

SelectResults::Iterator StructSetImpl::end() const {
  return m_structVector->end();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
