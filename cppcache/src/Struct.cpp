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

#include <geode/DataInput.hpp>
#include <geode/Struct.hpp>

namespace apache {
namespace geode {
namespace client {

Struct::Struct(StructSet* ssPtr,
               std::vector<std::shared_ptr<Serializable>>& fieldValues)
    : m_parent(ssPtr), m_fieldValues(fieldValues) {}

void Struct::skipClassName(DataInput& input) {
  if (input.read() == static_cast<int8_t>(DSCode::Class)) {
    input.read();  // ignore string type id - assuming its a normal
                   // (under 64k) string.
    uint16_t len = input.readInt16();
    input.advanceCursor(len);
  } else {
    throw IllegalStateException(
        "Struct: Did not get expected class header byte");
  }
}

void Struct::toData(DataOutput&) const {
  throw UnsupportedOperationException("Struct::toData: should not be called.");
}

int32_t Struct::size() const {
  return static_cast<int32_t>(m_fieldValues.size());
}

void Struct::fromData(DataInput& input) {
  input.advanceCursor(2);  // ignore classType
  skipClassName(input);

  int32_t numOfFields = input.readArrayLength();

  m_parent = nullptr;
  for (int32_t i = 0; i < numOfFields; i++) {
    m_fieldNameToIndex.emplace(input.readString(), i);
  }
  int32_t lengthForTypes = input.readArrayLength();
  skipClassName(input);
  for (int i = 0; i < lengthForTypes; i++) {
    input.advanceCursor(2);  // ignore classType
    skipClassName(input);
  }
  int32_t numOfSerializedValues = input.readArrayLength();
  skipClassName(input);
  for (int i = 0; i < numOfSerializedValues; i++) {
    std::shared_ptr<Serializable> val;
    input.readObject(val);  // need to look
    m_fieldValues.push_back(val);
  }
}

const std::string& Struct::getFieldName(const int32_t index) const {
  if (m_parent) {
    return m_parent->getFieldName(index);
  } else {
    for (const auto& iter : m_fieldNameToIndex) {
      if (iter.second == index) return (iter.first);
    }
  }

  throw OutOfRangeException("Struct: fieldName not found.");
}

const std::shared_ptr<Serializable> Struct::operator[](int32_t index) const {
  if (index >= static_cast<int32_t>(m_fieldValues.size())) {
    return nullptr;
  }

  return m_fieldValues[index];
}

const std::shared_ptr<Serializable> Struct::operator[](
    const std::string& fieldName) const {
  size_t index;
  if (m_parent) {
    index = m_parent->getFieldIndex(fieldName);
  } else {
    const auto& iter = m_fieldNameToIndex.find(fieldName);
    if (iter == m_fieldNameToIndex.end()) {
      throw OutOfRangeException("Struct: fieldName not found.");
    }
    index = iter->second;
  }
  return m_fieldValues[index];
}

const std::shared_ptr<StructSet> Struct::getStructSet() const {
  return std::shared_ptr<StructSet>(m_parent);
}

Struct::iterator Struct::begin() { return m_fieldValues.begin(); }

Struct::iterator Struct::end() { return m_fieldValues.end(); }

std::shared_ptr<Serializable> Struct::createDeserializable() {
  return std::make_shared<Struct>();
}
}  // namespace client
}  // namespace geode
}  // namespace apache
