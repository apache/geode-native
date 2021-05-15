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
/*
 * PdxType.cpp
 *
 *  Created on: Nov 3, 2011
 *      Author: npatel
 */

#include "PdxType.hpp"

#include "PdxFieldType.hpp"
#include "PdxHelper.hpp"
#include "PdxTypeRegistry.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

const char* PdxType::m_javaPdxClass = "org.apache.geode.pdx.internal.PdxType";

PdxType::~PdxType() noexcept {
  _GEODE_SAFE_DELETE(m_pdxFieldTypes);
  _GEODE_SAFE_DELETE_ARRAY(m_remoteToLocalFieldMap);
  _GEODE_SAFE_DELETE_ARRAY(m_localToRemoteFieldMap);
}

PdxType::PdxType(PdxTypeRegistry& pdxTypeRegistry,
                 const std::string& pdxDomainClassName, bool isLocal,
                 bool expectDomainClass)
    : Serializable(),
      m_pdxFieldTypes(new std::vector<std::shared_ptr<PdxFieldType>>()),
      m_className(pdxDomainClassName),
      m_geodeTypeId(0),
      m_isLocal(isLocal),
      m_numberOfVarLenFields(0),
      m_varLenFieldIdx(0),
      m_numberOfFieldsExtra(0),
      m_isVarLenFieldAdded(false),
      m_remoteToLocalFieldMap(nullptr),
      m_localToRemoteFieldMap(nullptr),
      is_java_class_(expectDomainClass),
      m_pdxTypeRegistry(pdxTypeRegistry) {}

PdxType::PdxType(PdxTypeRegistry& pdxTypeRegistry,
                 const std::string& pdxDomainClassName, bool isLocal)
    : PdxType(pdxTypeRegistry, pdxDomainClassName, isLocal, true) {}

void PdxType::toData(DataOutput& output) const {
  output.write(static_cast<int8_t>(DSCode::DataSerializable));  // 45
  output.write(static_cast<int8_t>(DSCode::Class));             // 43
  output.writeString(m_javaPdxClass);

  // m_className
  output.writeString(m_className);

  // m_noJavaClass
  output.writeBoolean(!is_java_class_);

  // m_geodeTypeId
  output.writeInt(m_geodeTypeId);

  // m_varLenFieldIdx
  output.writeInt(m_varLenFieldIdx);

  output.writeArrayLen(static_cast<int32_t>(m_pdxFieldTypes->size()));

  for (std::vector<std::shared_ptr<PdxFieldType>>::iterator it =
           m_pdxFieldTypes->begin();
       it != m_pdxFieldTypes->end(); ++it) {
    auto pdxPtr = *it;
    pdxPtr->toData(output);
  }
}

void PdxType::fromData(DataInput& input) {
  input.read();        // ignore dsByte
  input.read();        // ignore classByte
  input.readString();  // ignore classtypeId

  m_className = input.readString();

  is_java_class_ = !input.readBoolean();

  m_geodeTypeId = input.readInt32();

  m_varLenFieldIdx = input.readInt32();

  int len = input.readArrayLength();

  bool foundVarLenType = false;

  for (int i = 0; i < len; i++) {
    auto pft = std::make_shared<PdxFieldType>();
    pft->fromData(input);

    m_pdxFieldTypes->push_back(pft);

    if (pft->IsVariableLengthType() == true) foundVarLenType = true;
  }

  // as m_varLenFieldIdx starts with 0
  if (m_varLenFieldIdx != 0) {
    m_numberOfVarLenFields = m_varLenFieldIdx + 1;
  } else if (foundVarLenType) {
    m_numberOfVarLenFields = 1;
  }

  InitializeType();
}

void PdxType::addFixedLengthTypeField(const std::string& fieldName,
                                      const std::string& className,
                                      PdxFieldTypes typeId, int32_t size) {
  if (m_fieldNameVsPdxType.find(fieldName) != m_fieldNameVsPdxType.end()) {
    throw IllegalStateException("Field: " + fieldName +
                                " is already added to PdxWriter");
  }
  auto pfxPtr = std::make_shared<PdxFieldType>(
      fieldName, className, typeId,
      static_cast<int32_t>(m_pdxFieldTypes->size()), false, size, 0);
  m_pdxFieldTypes->push_back(pfxPtr);
  m_fieldNameVsPdxType[fieldName] = pfxPtr;
}

void PdxType::addVariableLengthTypeField(const std::string& fieldName,
                                         const std::string& className,
                                         PdxFieldTypes typeId) {
  if (m_fieldNameVsPdxType.find(fieldName) != m_fieldNameVsPdxType.end()) {
    throw IllegalStateException("Field: " + fieldName +
                                " is already added to PdxWriter");
  }

  if (m_isVarLenFieldAdded) {
    m_varLenFieldIdx++;  // it initial value is zero so variable length field
                         // idx start with zero
  }
  m_numberOfVarLenFields++;
  m_isVarLenFieldAdded = true;
  auto pfxPtr = std::make_shared<PdxFieldType>(
      fieldName, className, typeId,
      static_cast<int32_t>(m_pdxFieldTypes->size()), true, -1,
      m_varLenFieldIdx);
  m_pdxFieldTypes->push_back(pfxPtr);
  m_fieldNameVsPdxType[fieldName] = pfxPtr;
}

void PdxType::initRemoteToLocal() {
  // This method is to check if we have some extra fields in remote PdxType that
  // are absent in local PdxType.
  // 1. Get local PdxType from type registry
  // 2. Get list of PdxFieldTypes from it
  // 3. Use m_pdxFieldTypes for remote PdxType, that is populated in fromData
  // 4. Iterate over local and remote PdxFieldTypes to compare individual type
  // 5. If found update in m_remoteToLocalFieldMap that field there in remote
  // type. else update m_numberOfFieldsExtra
  // 6. else update in m_remoteToLocalFieldMap that local type don't have this
  // fields
  // 7. 1 = field there in remote type
  //  -1 = local type don't have this VariableLengthType field
  //  -2 = local type don't have this FixedLengthType field

  std::shared_ptr<PdxType> localPdxType = nullptr;
  //[TODO - open this up once PdxTypeRegistry is done]
  localPdxType = m_pdxTypeRegistry.getLocalPdxType(m_className);
  m_numberOfFieldsExtra = 0;

  if (localPdxType != nullptr) {
    std::vector<std::shared_ptr<PdxFieldType>>* localPdxFields =
        localPdxType->getPdxFieldTypes();
    int32_t fieldIdx = 0;

    if (m_remoteToLocalFieldMap != nullptr) {
      _GEODE_SAFE_DELETE_ARRAY(m_remoteToLocalFieldMap);
    }
    m_remoteToLocalFieldMap = new int32_t[m_pdxFieldTypes->size()];
    // LOG_DEBUG(
    //    "PdxType::initRemoteToLocal m_pdxFieldTypes->size() ={} AND "
    //    "localPdxFields->size()={}",
    //    m_pdxFieldTypes->size(), localPdxFields->size());
    for (std::vector<std::shared_ptr<PdxFieldType>>::iterator remotePdxField =
             m_pdxFieldTypes->begin();
         remotePdxField != m_pdxFieldTypes->end(); ++remotePdxField) {
      bool found = false;

      for (std::vector<std::shared_ptr<PdxFieldType>>::iterator localPdxfield =
               localPdxFields->begin();
           localPdxfield != localPdxFields->end(); ++localPdxfield) {
        // PdxFieldType* remotePdx = (*(remotePdxField)).get();
        // PdxFieldType* localPdx = (*(localPdxfield)).get();
        if ((*localPdxfield)->equals(*remotePdxField)) {
          found = true;
          m_remoteToLocalFieldMap[fieldIdx++] = 1;  // field there in remote
                                                    // type
          break;
        }
      }

      if (!found) {
        // while writing take this from weakhashmap
        // local field is not in remote type
        // if((*(remotePdxField)).get()->IsVariableLengthType()) {
        if ((*remotePdxField)->IsVariableLengthType()) {
          m_remoteToLocalFieldMap[fieldIdx++] =
              -1;  // local type don't have this fields
        } else {
          m_remoteToLocalFieldMap[fieldIdx++] =
              -2;  // local type don't have this fields
        }
        m_numberOfFieldsExtra++;
      }
    }
  }
}

void PdxType::initLocalToRemote() {
  // This method is to check if we have some extra fields in remote PdxType that
  // are absent in local PdxType.
  // 1. Get local PdxType from type registry
  // 2. Get list of PdxFieldTypes from it
  // 3. Iterate over local PdxFields to compare it with remote PdxFields and if
  // they r equal that means there sequence is same mark it with -2
  // 4. Iterate over local PdxFields and remote PdxFields and if they are equal
  // then mark m_localToRemoteFieldMap with remotePdxField sequenceId
  // 5. else if local field is not in remote type then -1

  std::shared_ptr<PdxType> localPdxType = nullptr;
  localPdxType = m_pdxTypeRegistry.getLocalPdxType(m_className);

  if (localPdxType != nullptr) {
    std::vector<std::shared_ptr<PdxFieldType>>* localPdxFields =
        localPdxType->getPdxFieldTypes();

    int32_t fieldIdx = 0;
    // type which need to read/write should control local type
    int32_t localToRemoteFieldMapSize =
        static_cast<int32_t>(localPdxType->m_pdxFieldTypes->size());
    if (m_localToRemoteFieldMap != nullptr) {
      _GEODE_SAFE_DELETE_ARRAY(m_localToRemoteFieldMap);
    }
    m_localToRemoteFieldMap = new int32_t[localToRemoteFieldMapSize];

    for (int32_t i = 0; i < localToRemoteFieldMapSize &&
                        i < static_cast<int32_t>(m_pdxFieldTypes->size());
         i++) {
      // PdxFieldType* localPdx = localPdxFields->at(fieldIdx).get();
      // PdxFieldType* remotePdx = m_pdxFieldTypes->at(i).get();
      if (localPdxFields->at(fieldIdx)->equals(m_pdxFieldTypes->at(i))) {
        // fields are in same order, we can read as it is
        m_localToRemoteFieldMap[fieldIdx++] = -2;
      } else {
        break;
      }
    }

    for (; fieldIdx < localToRemoteFieldMapSize;) {
      auto localPdxField = localPdxType->m_pdxFieldTypes->at(fieldIdx);
      bool found = false;

      for (std::vector<std::shared_ptr<PdxFieldType>>::iterator remotePdxfield =
               m_pdxFieldTypes->begin();
           remotePdxfield != m_pdxFieldTypes->end(); ++remotePdxfield)
      // for each(PdxFieldType^ remotePdxfield in m_pdxFieldTypes)
      {
        // PdxFieldType* localPdx = localPdxField.get();
        PdxFieldType* remotePdx = (*(remotePdxfield)).get();
        if (localPdxField->equals(*remotePdxfield)) {
          found = true;
          // store pdxfield type position to get the offset quickly
          m_localToRemoteFieldMap[fieldIdx++] = remotePdx->getSequenceId();
          break;
        }
      }

      if (!found) {
        // local field is not in remote type
        m_localToRemoteFieldMap[fieldIdx++] = -1;
      }
    }
  }
}

void PdxType::InitializeType() {
  initRemoteToLocal();  // for writing
  initLocalToRemote();  // for reading
  generatePositionMap();
}

int32_t PdxType::getFieldPosition(const std::string& fieldName,
                                  uint8_t* offsetPosition, int32_t offsetSize,
                                  int32_t pdxStreamlen) {
  auto pft = this->getPdxField(fieldName);
  if (pft != nullptr) {
    if (pft->IsVariableLengthType()) {
      return variableLengthFieldPosition(pft, offsetPosition, offsetSize);
    } else {
      return fixedLengthFieldPosition(pft, offsetPosition, offsetSize,
                                      pdxStreamlen);
    }
  }
  return -1;
}

int32_t PdxType::getFieldPosition(int32_t fieldIdx, uint8_t* offsetPosition,
                                  int32_t offsetSize, int32_t pdxStreamlen) {
  auto pft = m_pdxFieldTypes->at(fieldIdx);
  if (pft != nullptr) {
    if (pft->IsVariableLengthType()) {
      return variableLengthFieldPosition(pft, offsetPosition, offsetSize);
    } else {
      return fixedLengthFieldPosition(pft, offsetPosition, offsetSize,
                                      pdxStreamlen);
    }
  }
  return -1;
}

int32_t PdxType::fixedLengthFieldPosition(
    std::shared_ptr<PdxFieldType> fixLenField, uint8_t* offsetPosition,
    int32_t offsetSize, int32_t pdxStreamlen) {
  int32_t offset = fixLenField->getVarLenOffsetIndex();
  if (fixLenField->getRelativeOffset() >= 0) {
    // starting fields
    return fixLenField->getRelativeOffset();
  } else if (offset == -1)  // Pdx length
  {
    // there is no var len field so just subtracts relative offset from behind
    return pdxStreamlen + fixLenField->getRelativeOffset();
  } else {
    // need to read offset and then subtract relative offset
    // TODO
    return PdxHelper::readInt(
               offsetPosition +
                   (m_numberOfVarLenFields - offset - 1) * offsetSize,
               offsetSize) +
           fixLenField->getRelativeOffset();
  }
}

int32_t PdxType::variableLengthFieldPosition(
    std::shared_ptr<PdxFieldType> varLenField, uint8_t* offsetPosition,
    int32_t offsetSize) {
  int32_t offset = varLenField->getVarLenOffsetIndex();
  if (offset == -1) {
    return /*first var len field*/ varLenField->getRelativeOffset();
  } else {
    // we write offset from behind
    return PdxHelper::readInt(
        offsetPosition + (m_numberOfVarLenFields - offset - 1) * offsetSize,
        offsetSize);
  }
}

int32_t* PdxType::getLocalToRemoteMap() {
  if (m_localToRemoteFieldMap != nullptr) {
    return m_localToRemoteFieldMap;
  }

  ReadGuard guard(m_lockObj);
  if (m_localToRemoteFieldMap != nullptr) {
    return m_localToRemoteFieldMap;
  }
  initLocalToRemote();

  return m_localToRemoteFieldMap;
}

int32_t* PdxType::getRemoteToLocalMap() {
  if (m_remoteToLocalFieldMap != nullptr) {
    return m_remoteToLocalFieldMap;
  }

  ReadGuard guard(m_lockObj);
  if (m_remoteToLocalFieldMap != nullptr) {
    return m_remoteToLocalFieldMap;
  }
  initRemoteToLocal();

  return m_remoteToLocalFieldMap;
}
std::shared_ptr<PdxType> PdxType::isContains(std::shared_ptr<PdxType> first,
                                             std::shared_ptr<PdxType> second) {
  int j = 0;
  for (int i = 0; i < static_cast<int>(second->m_pdxFieldTypes->size()); i++) {
    auto secondPdt = second->m_pdxFieldTypes->at(i);
    bool matched = false;
    for (; j < static_cast<int>(first->m_pdxFieldTypes->size()); j++) {
      auto firstPdt = first->m_pdxFieldTypes->at(j);
      // PdxFieldType* firstType = firstPdt.get();
      // PdxFieldType* secondType = secondPdt.get();
      if (firstPdt->equals(secondPdt)) {
        matched = true;
        break;
      }
    }
    if (!matched) return nullptr;
  }
  return first;
}
std::shared_ptr<PdxType> PdxType::clone() {
  auto clone = std::make_shared<PdxType>(m_pdxTypeRegistry, m_className, false);
  clone->m_geodeTypeId = 0;
  clone->m_numberOfVarLenFields = m_numberOfVarLenFields;

  for (std::vector<std::shared_ptr<PdxFieldType>>::iterator it =
           m_pdxFieldTypes->begin();
       it != m_pdxFieldTypes->end(); ++it) {
    auto pdxPtr = *it;
    clone->m_pdxFieldTypes->push_back(pdxPtr);
  }
  return clone;
}
std::shared_ptr<PdxType> PdxType::isLocalTypeContains(
    std::shared_ptr<PdxType> otherType) {
  if (m_pdxFieldTypes->size() >= otherType->m_pdxFieldTypes->size()) {
    return isContains(shared_from_this(), otherType);
  }
  return nullptr;
}
std::shared_ptr<PdxType> PdxType::isRemoteTypeContains(
    std::shared_ptr<PdxType> remoteType) {
  if (m_pdxFieldTypes->size() <= remoteType->m_pdxFieldTypes->size()) {
    return isContains(remoteType, shared_from_this());
  }
  return nullptr;
}
std::shared_ptr<PdxType> PdxType::mergeVersion(
    std::shared_ptr<PdxType> otherVersion) {
  // int nTotalFields = otherVersion->m_pdxFieldTypes->size();
  std::shared_ptr<PdxType> contains = nullptr;

  if (isLocalTypeContains(otherVersion) != nullptr) return shared_from_this();

  if (isRemoteTypeContains(otherVersion) != nullptr) return otherVersion;

  // need to create new one, clone of local
  auto newone = clone();
  int varLenFields = newone->getNumberOfVarLenFields();

  for (std::vector<std::shared_ptr<PdxFieldType>>::iterator it =
           otherVersion->m_pdxFieldTypes->begin();
       it != otherVersion->m_pdxFieldTypes->end(); ++it) {
    bool found = false;
    // for each(PdxFieldType^ tmpNew in newone->m_pdxFieldTypes)
    for (std::vector<std::shared_ptr<PdxFieldType>>::iterator it2 =
             newone->m_pdxFieldTypes->begin();
         it2 != newone->m_pdxFieldTypes->end(); ++it2) {
      if ((*it2)->equals(*it)) {
        found = true;
        break;
      }
    }
    if (!found) {
      auto newFt = std::make_shared<PdxFieldType>(
          (*it)->getFieldName(), (*it)->getClassName(), (*it)->getTypeId(),
          static_cast<int32_t>(newone->m_pdxFieldTypes->size()),  // sequence id
          (*it)->IsVariableLengthType(), (*it)->getFixedSize(),
          ((*it)->IsVariableLengthType()
               ? varLenFields++ /*it increase after that*/
               : 0));
      newone->m_pdxFieldTypes->push_back(
          newFt);  // fieldnameVsPFT will happen after that
    }
  }

  newone->setNumberOfVarLenFields(varLenFields);
  if (varLenFields > 0) newone->setVarLenFieldIdx(varLenFields);

  // need to keep all versions in local version
  // m_otherVersions->Add(newone);
  return newone;
}

void PdxType::generatePositionMap() {
  bool foundVarLen = false;
  int lastVarLenSeqId = 0;
  int prevFixedSizeOffsets = 0;
  // set offsets from back first
  std::shared_ptr<PdxFieldType> previousField = nullptr;

  for (int i = static_cast<int>(m_pdxFieldTypes->size()) - 1; i >= 0; i--) {
    auto tmpft = m_pdxFieldTypes->at(i);
    std::string temp = tmpft->getFieldName();
    std::pair<std::string, std::shared_ptr<PdxFieldType>> pc(temp, tmpft);
    m_fieldNameVsPdxType.insert(pc);

    if (tmpft->IsVariableLengthType()) {
      tmpft->setVarLenOffsetIndex(tmpft->getVarLenFieldIdx());
      tmpft->setRelativeOffset(0);
      foundVarLen = true;
      lastVarLenSeqId = tmpft->getVarLenFieldIdx();
    } else {
      if (foundVarLen) {
        tmpft->setVarLenOffsetIndex(lastVarLenSeqId);
        // relative offset is subtracted from var len offsets
        tmpft->setRelativeOffset(-tmpft->getFixedSize() +
                                 previousField->getRelativeOffset());
      } else {
        tmpft->setVarLenOffsetIndex(-1);  // Pdx header length
        // relative offset is subtracted from var len offsets
        tmpft->setRelativeOffset(-tmpft->getFixedSize());
        if (previousField != nullptr) {  // boundary condition
          tmpft->setRelativeOffset(-tmpft->getFixedSize() +
                                   previousField->getRelativeOffset());
        }
      }
    }

    previousField = tmpft;
  }

  foundVarLen = false;
  prevFixedSizeOffsets = 0;
  // now do optimization till you don't fine var len
  for (uint32_t i = 0; (i < m_pdxFieldTypes->size()) && !foundVarLen; i++) {
    auto tmpft = m_pdxFieldTypes->at(i);

    if (tmpft->IsVariableLengthType()) {
      tmpft->setVarLenOffsetIndex(-1);  // first var len field
      tmpft->setRelativeOffset(prevFixedSizeOffsets);
      foundVarLen = true;
    } else {
      tmpft->setVarLenOffsetIndex(0);  // no need to read offset
      tmpft->setRelativeOffset(prevFixedSizeOffsets);
      prevFixedSizeOffsets += tmpft->getFixedSize();
    }
  }
}

bool PdxType::operator==(const PdxType& other) const {
  if (this->m_className != other.m_className) {
    return false;
  }

  if (this->is_java_class_ != other.is_java_class_) {
    return false;
  }

  if (this->getTotalFields() != other.getTotalFields()) {
    return false;
  }

  auto thisIt = this->m_fieldNameVsPdxType.begin();
  auto otherIt = other.m_fieldNameVsPdxType.begin();
  for (; thisIt != this->m_fieldNameVsPdxType.end(); thisIt++, otherIt++) {
    auto thisEntry = *thisIt;
    auto otherEntry = *otherIt;
    if (thisEntry.first != otherEntry.first ||
        *(thisEntry.second) != *(otherEntry.second)) {
      return false;
    }
  }
  return true;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
