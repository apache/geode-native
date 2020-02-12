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

#ifndef GEODE_PDXTYPE_H_
#define GEODE_PDXTYPE_H_

#include <list>
#include <map>
#include <string>
#include <vector>

#include <ace/RW_Thread_Mutex.h>

#include <geode/CacheableBuiltins.hpp>
#include <geode/PdxFieldTypes.hpp>
#include <geode/Serializable.hpp>

#include "NonCopyable.hpp"
#include "PdxFieldType.hpp"
#include "ReadWriteLock.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, std::shared_ptr<PdxFieldType>> NameVsPdxType;
class PdxType;
class PdxTypeRegistry;

class PdxType : public internal::DataSerializableInternal,
                public std::enable_shared_from_this<PdxType>,
                private NonCopyable,
                private NonAssignable {
 private:
  ACE_RW_Thread_Mutex m_lockObj;

  static const char* m_javaPdxClass;

  std::vector<std::shared_ptr<PdxFieldType>>* m_pdxFieldTypes;

  std::list<std::shared_ptr<PdxType>> m_otherVersions;

  std::string m_className;

  int32_t m_geodeTypeId;

  bool m_isLocal;

  int32_t m_numberOfVarLenFields;

  int32_t m_varLenFieldIdx;

  int32_t m_numberOfFieldsExtra;

  bool m_isVarLenFieldAdded;

  int32_t* m_remoteToLocalFieldMap;

  int32_t* m_localToRemoteFieldMap;

  NameVsPdxType m_fieldNameVsPdxType;

  bool m_noJavaClass;

  PdxTypeRegistry& m_pdxTypeRegistry;

  void initRemoteToLocal();

  void initLocalToRemote();

  int32_t fixedLengthFieldPosition(std::shared_ptr<PdxFieldType> fixLenField,
                                   uint8_t* offsetPosition, int32_t offsetSize,
                                   int32_t pdxStreamlen);

  int32_t variableLengthFieldPosition(std::shared_ptr<PdxFieldType> varLenField,
                                      uint8_t* offsetPosition,
                                      int32_t offsetSize);

  std::shared_ptr<PdxType> isContains(std::shared_ptr<PdxType> first,
                                      std::shared_ptr<PdxType> second);
  std::shared_ptr<PdxType> clone();
  void generatePositionMap();

  std::shared_ptr<PdxType> isLocalTypeContains(
      std::shared_ptr<PdxType> otherType);
  std::shared_ptr<PdxType> isRemoteTypeContains(
      std::shared_ptr<PdxType> localType);

 public:
  PdxType(PdxTypeRegistry& pdxTypeRegistryPtr,
          const std::string& pdxDomainClassName, bool isLocal);

  ~PdxType() noexcept override;

  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  static std::shared_ptr<Serializable> CreateDeserializable(
      PdxTypeRegistry& pdxTypeRegistry) {
    return std::make_shared<PdxType>(pdxTypeRegistry, "", false);
  }

  size_t objectSize() const override {
    auto size = sizeof(PdxType);
    if (m_pdxFieldTypes != nullptr) {
      for (size_t i = 0; i < m_pdxFieldTypes->size(); i++) {
        size += m_pdxFieldTypes->at(i)->objectSize();
      }
    }
    size += static_cast<uint32_t>(m_className.length());
    for (auto&& iter : m_fieldNameVsPdxType) {
      size += iter.first.length();
      size += iter.second->objectSize();
    }
    if (m_remoteToLocalFieldMap != nullptr) {
      if (m_pdxFieldTypes != nullptr) {
        size += sizeof(int32_t) * m_pdxFieldTypes->size();
      }
    }
    if (m_localToRemoteFieldMap != nullptr) {
      if (m_pdxFieldTypes != nullptr) {
        size += sizeof(int32_t) * m_pdxFieldTypes->size();
      }
    }
    return size;
  }

  virtual int32_t getTypeId() const { return m_geodeTypeId; }

  virtual void setTypeId(int32_t typeId) { m_geodeTypeId = typeId; }

  int32_t getNumberOfVarLenFields() const { return m_numberOfVarLenFields; }

  void setNumberOfVarLenFields(int32_t value) {
    m_numberOfVarLenFields = value;
  }

  int32_t getTotalFields() const {
    return static_cast<int32_t>(m_pdxFieldTypes->size());
  }

  const std::string& getPdxClassName() const { return m_className; }

  void setPdxClassName(std::string className) { m_className = className; }

  int32_t getNumberOfExtraFields() const { return m_numberOfFieldsExtra; }

  void setVarLenFieldIdx(int32_t value) { m_varLenFieldIdx = value; }

  int32_t getVarLenFieldIdx() const { return m_varLenFieldIdx; }

  std::shared_ptr<PdxFieldType> getPdxField(const std::string& fieldName) {
    auto&& iter = m_fieldNameVsPdxType.find(fieldName);
    if (iter != m_fieldNameVsPdxType.end()) {
      return iter->second;
    }
    return nullptr;
  }

  bool isLocal() const { return m_isLocal; }

  void setLocal(bool local) { m_isLocal = local; }

  std::vector<std::shared_ptr<PdxFieldType>>* getPdxFieldTypes() const {
    return m_pdxFieldTypes;
  }

  void addFixedLengthTypeField(const std::string& fieldName,
                               const std::string& className,
                               PdxFieldTypes typeId, int32_t size);

  void addVariableLengthTypeField(const std::string& fieldName,
                                  const std::string& className,
                                  PdxFieldTypes typeId);
  void InitializeType();

  std::shared_ptr<PdxType> mergeVersion(std::shared_ptr<PdxType> otherVersion);

  int32_t getFieldPosition(const std::string& fieldName,
                           uint8_t* offsetPosition, int32_t offsetSize,
                           int32_t pdxStreamlen);

  int32_t getFieldPosition(int32_t fieldIdx, uint8_t* offsetPosition,
                           int32_t offsetSize, int32_t pdxStreamlen);

  int32_t* getLocalToRemoteMap();

  int32_t* getRemoteToLocalMap();

  bool Equals(std::shared_ptr<PdxType> otherObj);

  // This is for PdxType as key in std map.
  bool operator<(const PdxType& other) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPE_H_
