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

#include <string>

#include <geode/Struct.hpp>
#include <GeodeTypeIdsImpl.hpp>
#include <geode/DataInput.hpp>

namespace apache {
namespace geode {
namespace client {

Struct::Struct() : m_parent(nullptr), m_lastAccessIndex(0) {}

Struct::Struct(StructSet* ssPtr, std::vector<SerializablePtr>& fieldValues) {
  m_parent = ssPtr;
  m_fieldValues.insert(m_fieldValues.end(), fieldValues.begin(),
                       fieldValues.end());
  m_lastAccessIndex = 0;
}

void Struct::skipClassName(DataInput& input) {
  if (input.read() == GeodeTypeIdsImpl::Class) {
    input.read();  // ignore string type id - assuming its a normal
                              // (under 64k) string.
    uint16_t len = input.readInt16();
    input.advanceCursor(len);
  } else {
    throw IllegalStateException(
        "Struct: Did not get expected class header byte");
  }
}

int32_t Struct::classId() const { return 0; }

int8_t Struct::typeId() const { return GeodeTypeIds::Struct; }

int8_t Struct::DSFID() const { return GeodeTypeIdsImpl::FixedIDByte; }

void Struct::toData(DataOutput& output) const {
  throw UnsupportedOperationException("Struct::toData: should not be called.");
}

int32_t Struct::length() const {
  return static_cast<int32_t>(m_fieldValues.size());
}

void Struct::fromData(DataInput& input) {
  input.advanceCursor(2); // ignore classType
  skipClassName(input);

  int32_t numOfFields = input.readArrayLen();

  m_parent = nullptr;
  for (int32_t i = 0; i < numOfFields; i++) {
    CacheableStringPtr fieldName = input.readNativeString();
    m_fieldNames.emplace(fieldName->asChar(), i);
  }
  int32_t lengthForTypes = input.readArrayLen();
  skipClassName(input);
  for (int i = 0; i < lengthForTypes; i++) {
    input.advanceCursor(2); // ignore classType
    skipClassName(input);
  }
  int32_t numOfSerializedValues = input.readArrayLen();
  skipClassName(input);
  for (int i = 0; i < numOfSerializedValues; i++) {
    SerializablePtr val;
    input.readObject(val);  // need to look
    m_fieldValues.push_back(val);
  }
}

const std::string& Struct::getFieldName(const int32_t index) const {
  if (m_parent != nullptr) {
    return m_parent->getFieldName(index);
  } else {
    for (const auto& iter : m_fieldNames) {
      if (iter.second == index) return (iter.first);
    }
  }

  throw OutOfRangeException("Struct: fieldName not found.");
}

const SerializablePtr Struct::operator[](int32_t index) const {
  if (index >= m_fieldValues.size()) {
    return nullptr;
  }

  return m_fieldValues[index];
}

const SerializablePtr Struct::operator[](const std::string& fieldName) const {
  int32_t index;
  if (m_parent == nullptr) {
    const auto& iter = m_fieldNames.find(fieldName);
    if (iter == m_fieldNames.end()) {
      throw OutOfRangeException("Struct: fieldName not found.");
    }
    index = iter->second;
  } else {
    index = m_parent->getFieldIndex(fieldName);
  }
  return m_fieldValues[index];
}

const StructSetPtr Struct::getStructSet() const {
  return StructSetPtr(m_parent);
}

bool Struct::hasNext() const {
  if (m_lastAccessIndex + 1 <= m_fieldValues.size()) {
    return true;
  }
  return false;
}

const SerializablePtr Struct::next() {
  m_lastAccessIndex++;
  return m_fieldValues[m_lastAccessIndex - 1];
}

Serializable* Struct::createDeserializable() { return new Struct(); }
}  // namespace client
}  // namespace geode
}  // namespace apache
