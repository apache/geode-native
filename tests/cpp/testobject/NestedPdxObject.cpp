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
 * NestedPdxObject.cpp
 *
 */

#include "NestedPdxObject.hpp"

namespace testobject {

void ChildPdx::toData(PdxWriter& pw) const {
  LOG_DEBUG("ChildPdx::toData() Started......");

  pw.writeInt("m_childId", m_childId);
  pw.markIdentityField("m_childId");
  pw.writeObject("m_enum", m_enum);
  pw.writeString("m_childName", m_childName);

  LOG_DEBUG("ChildPdx::toData() Done......");
}

void ChildPdx::fromData(PdxReader& pr) {
  LOG_INFO("ChildPdx::fromData() start...");

  m_childId = pr.readInt("m_childId");
  LOG_INFO("ChildPdx::fromData() m_childId = %d ", m_childId);
  m_enum = std::dynamic_pointer_cast<CacheableEnum>(pr.readObject("m_enum"));
  m_childName = pr.readString("m_childName");

  LOG_INFO("ChildPdx::fromData() end...");
}

std::string ChildPdx::toString() const {
  return "ChildPdx: [m_childId=" + std::to_string(m_childId) +
         "] [ m_childName=" + m_childName + " ]";
}

bool ChildPdx::equals(ChildPdx& other) const {
  LOG_INFO("ChildPdx::equals");
  ChildPdx* ot = dynamic_cast<ChildPdx*>(&other);
  // Cacheable* ot = dynamic_cast<Cacheable*>(&other);
  if (!ot) {
    LOG_INFO("ChildPdx::equals1");
    return false;
  }
  if ((m_childName == other.m_childName) && (m_childId == other.m_childId) &&
      (m_enum->getEnumOrdinal() == other.m_enum->getEnumOrdinal()) &&
      (m_enum->getEnumClassName() == other.m_enum->getEnumClassName()) &&
      (m_enum->getEnumName() == other.m_enum->getEnumName())) {
    LOG_INFO("ChildPdx::equals2");
    return true;
  }
  return false;
}

void ParentPdx::toData(PdxWriter& pw) const {
  LOG_DEBUG("ParentPdx::toData() Started......");

  pw.writeInt("m_parentId", m_parentId);
  LOG_DEBUG("ParentPdx::toData() m_parentId......");
  pw.markIdentityField("m_parentId");
  pw.writeObject("m_enum", m_enum);
  LOG_DEBUG("ParentPdx::toData() m_enum......");
  pw.writeString("m_parentName", m_parentName);
  LOG_DEBUG("ParentPdx::toData() m_parentName......");
  pw.writeObject("m_childPdx", m_childPdx);
  LOG_DEBUG("ParentPdx::toData() m_childPdx......");
  pw.markIdentityField("m_childPdx");

  pw.writeChar("m_char", m_char);
  pw.writeCharArray("m_charArray", m_charArray);

  LOG_DEBUG("ParentPdx::toData() Done......");
}

void ParentPdx::fromData(PdxReader& pr) {
  LOG_INFO("ParentPdx::fromData() start...");

  m_parentId = pr.readInt("m_parentId");
  LOG_INFO("ParentPdx::fromData() m_parentId = %d ", m_parentId);
  m_enum = std::dynamic_pointer_cast<CacheableEnum>(pr.readObject("m_enum"));
  LOG_INFO("ParentPdx::fromData() read gender ");
  m_parentName = pr.readString("m_parentName");
  LOG_INFO("ParentPdx::fromData() m_parentName = %s ", m_parentName.c_str());
  m_childPdx = pr.readObject("m_childPdx");
  LOG_INFO("ParentPdx::fromData() start3...");

  m_char = pr.readChar("m_char");
  m_charArray = pr.readCharArray("m_charArray");

  LOG_INFO("ParentPdx::fromData() end...");
}

std::string ParentPdx::toString() const {
  char idbuf[1024];
  sprintf(idbuf,
          "ParentPdx: [m_parentId=%d] [ m_parentName=%s ] [m_childPdx = %s ] ",
          m_parentId, m_parentName.c_str(), m_childPdx->toString().c_str());
  return idbuf;
}

bool ParentPdx::equals(const ParentPdx& other, bool isPdxReadSerialized) const {
  if ((m_parentName == other.m_parentName) &&
      (m_parentId == other.m_parentId) &&
      (m_enum->getEnumOrdinal() == other.m_enum->getEnumOrdinal()) &&
      (m_enum->getEnumClassName() == other.m_enum->getEnumClassName()) &&
      (m_enum->getEnumName() == other.m_enum->getEnumName()) &&
      m_char == other.m_char && m_charArray == other.m_charArray) {
    if (!isPdxReadSerialized) {
      ChildPdx* ch1 = dynamic_cast<ChildPdx*>(m_childPdx.get());
      ChildPdx* ch2 = dynamic_cast<ChildPdx*>(other.m_childPdx.get());

      if (ch1->equals(*ch2)) {
        return true;
      }
    }
    return true;
  }

  return false;
}
}  // namespace testobject
