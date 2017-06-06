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

#include <geode/Serializable.hpp>
#include "PdxFieldType.hpp"
#include <geode/CacheableBuiltins.hpp>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <ace/ACE.h>
#include <ace/Recursive_Thread_Mutex.h>
#include "ReadWriteLock.hpp"

#include "NonCopyable.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, PdxFieldTypePtr> NameVsPdxType;
class PdxType;
typedef std::shared_ptr<PdxType> PdxTypePtr;
class PdxTypeRegistry;
typedef std::shared_ptr <PdxTypeRegistry> PdxTypeRegistryPtr;

class PdxType : public Serializable,
                private NonCopyable,
                private NonAssignable {
 private:
  ACE_RW_Thread_Mutex m_lockObj;

  static const char* m_javaPdxClass;

  std::vector<PdxFieldTypePtr>* m_pdxFieldTypes;

  std::list<PdxTypePtr> m_otherVersions;

  char* m_className;

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

  PdxTypeRegistryPtr m_pdxTypeRegistryPtr;

  void initRemoteToLocal();
  void initLocalToRemote();
  int32_t fixedLengthFieldPosition(PdxFieldTypePtr fixLenField,
                                   uint8_t* offsetPosition, int32_t offsetSize,
                                   int32_t pdxStreamlen);
  int32_t variableLengthFieldPosition(PdxFieldTypePtr varLenField,
                                      uint8_t* offsetPosition,
                                      int32_t offsetSize, int32_t pdxStreamlen);

  PdxTypePtr isContains(PdxTypePtr first, PdxTypePtr second);
  PdxTypePtr clone();
  void generatePositionMap();

  PdxTypePtr isLocalTypeContains(PdxTypePtr otherType);
  PdxTypePtr isRemoteTypeContains(PdxTypePtr localType);

  PdxTypePtr shared_from_this() {
    return std::static_pointer_cast<PdxType>(Serializable::shared_from_this());
  }

 public:

  PdxType(PdxTypeRegistryPtr pdxTypeRegistryPtr, const char* pdxDomainClassName, bool isLocal);

  virtual ~PdxType();

  virtual void toData(DataOutput& output) const;

  virtual Serializable* fromData(DataInput& input);

  virtual int32_t classId() const { return GeodeTypeIds::PdxType; }

  static Serializable* CreateDeserializable(PdxTypeRegistryPtr pdxTypeRegistryPtr) { return new PdxType(pdxTypeRegistryPtr, nullptr, false); }

  virtual uint32_t objectSize() const {
    uint32_t size = sizeof(PdxType);
    if (m_pdxFieldTypes != nullptr) {
      for (size_t i = 0; i < m_pdxFieldTypes->size(); i++) {
        size += m_pdxFieldTypes->at(i)->objectSize();
      }
    }
    size += static_cast<uint32_t>(strlen(m_className));
    for (NameVsPdxType::const_iterator iter = m_fieldNameVsPdxType.begin();
         iter != m_fieldNameVsPdxType.end(); ++iter) {
      size += static_cast<uint32_t>(iter->first.length());
      size += iter->second->objectSize();
    }
    if (m_remoteToLocalFieldMap != nullptr) {
      if (m_pdxFieldTypes != nullptr) {
        size +=
            static_cast<uint32_t>(sizeof(int32_t) * m_pdxFieldTypes->size());
      }
    }
    if (m_localToRemoteFieldMap != nullptr) {
      if (m_pdxFieldTypes != nullptr) {
        size +=
            static_cast<uint32_t>(sizeof(int32_t) * m_pdxFieldTypes->size());
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

  char* getPdxClassName() const { return m_className; }

  void setPdxClassName(char* className) { m_className = className; }

  int32_t getNumberOfExtraFields() const { return m_numberOfFieldsExtra; }

  void setVarLenFieldIdx(int32_t value) { m_varLenFieldIdx = value; }

  int32_t getVarLenFieldIdx() const { return m_varLenFieldIdx; }

  PdxFieldTypePtr getPdxField(const char* fieldName) {
    NameVsPdxType::iterator iter = m_fieldNameVsPdxType.find(fieldName);
    if (iter != m_fieldNameVsPdxType.end()) {
      return (*iter).second;
    }
    return nullptr;
  }

  bool isLocal() const { return m_isLocal; }

  void setLocal(bool local) { m_isLocal = local; }

  std::vector<PdxFieldTypePtr>* getPdxFieldTypes() const {
    return m_pdxFieldTypes;
  }

  void addFixedLengthTypeField(const char* fieldName, const char* className,
                               int8_t typeId, int32_t size);
  void addVariableLengthTypeField(const char* fieldName, const char* className,
                                  int8_t typeId);
  void InitializeType();
  PdxTypePtr mergeVersion(PdxTypePtr otherVersion);
  int32_t getFieldPosition(const char* fieldName, uint8_t* offsetPosition,
                           int32_t offsetSize, int32_t pdxStreamlen);
  int32_t getFieldPosition(int32_t fieldIdx, uint8_t* offsetPosition,
                           int32_t offsetSize, int32_t pdxStreamlen);
  int32_t* getLocalToRemoteMap();
  int32_t* getRemoteToLocalMap();
  bool Equals(PdxTypePtr otherObj);
  // This is for PdxType as key in std map.
  bool operator<(const PdxType& other) const;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXTYPE_H_
