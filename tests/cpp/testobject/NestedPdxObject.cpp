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

#include <sstream>

namespace testobject {

void ChildPdx::toData(PdxWriter& pw) const {
  LOGDEBUG("ChildPdx::toData() Started......");

  pw.writeInt("m_childId", childId_);
  pw.markIdentityField("m_childId");
  pw.writeObject("m_enum", enum_);
  pw.writeString("m_childName", childName_);

  LOGDEBUG("ChildPdx::toData() Done......");
}

void ChildPdx::fromData(PdxReader& pr) {
  LOGINFO("ChildPdx::fromData() start...");

  childId_ = pr.readInt("m_childId");
  LOGINFO("ChildPdx::fromData() m_childId = %d ", childId_);
  enum_ = std::dynamic_pointer_cast<CacheableEnum>(pr.readObject("m_enum"));
  childName_ = pr.readString("m_childName");

  LOGINFO("ChildPdx::fromData() end...");
}

std::string ChildPdx::toString() const {
  return "ChildPdx: [m_childId=" + std::to_string(childId_) +
         "] [ m_childName=" + childName_ + " ]";
}

bool ChildPdx::equals(ChildPdx& other) const {
  LOGINFO("ChildPdx::equals");
  ChildPdx* ot = dynamic_cast<ChildPdx*>(&other);
  // Cacheable* ot = dynamic_cast<Cacheable*>(&other);
  if (!ot) {
    LOGINFO("ChildPdx::equals1");
    return false;
  }
  if ((childName_ == other.childName_) && (childId_ == other.childId_) &&
      (enum_->getEnumOrdinal() == other.enum_->getEnumOrdinal()) &&
      (enum_->getEnumClassName() == other.enum_->getEnumClassName()) &&
      (enum_->getEnumName() == other.enum_->getEnumName())) {
    LOGINFO("ChildPdx::equals2");
    return true;
  }
  return false;
}

ParentPdx::ParentPdx()
    : parentId_{-1},
      parentName_{},
      childPdx_{},
      enum_{},
      char_{L'\0'},
      charArray_{} {}

ParentPdx::ParentPdx(int id)
    : parentId_{id},
      parentName_{"name-" + std::to_string(id)},
      childPdx_{std::make_shared<ChildPdx>(id)},
      enum_{CacheableEnum::create("Gender", "male", 6)},
      char_{'C'},
      charArray_{{'X', 'Y'}} {}

const std::string& ParentPdx::getClassName() const {
  static std::string className = "testobject.ParentPdx";
  return className;
}

std::shared_ptr<PdxSerializable> ParentPdx::createDeserializable() {
  return std::make_shared<ParentPdx>();
}

void ParentPdx::toData(PdxWriter& pw) const {
  LOGDEBUG("ParentPdx::toData() Started......");

  pw.writeInt("m_parentId", parentId_);
  LOGDEBUG("ParentPdx::toData() m_parentId......");
  pw.markIdentityField("m_parentId");
  pw.writeObject("m_enum", enum_);
  LOGDEBUG("ParentPdx::toData() m_enum......");
  pw.writeString("m_parentName", parentName_);
  LOGDEBUG("ParentPdx::toData() m_parentName......");
  pw.writeObject("m_childPdx", childPdx_);
  LOGDEBUG("ParentPdx::toData() m_childPdx......");
  pw.markIdentityField("m_childPdx");

  pw.writeChar("m_char", char_);
  pw.writeCharArray("m_charArray", charArray_);

  LOGDEBUG("ParentPdx::toData() Done......");
}

void ParentPdx::fromData(PdxReader& pr) {
  LOGINFO("ParentPdx::fromData() start...");

  parentId_ = pr.readInt("m_parentId");
  LOGINFO("ParentPdx::fromData() m_parentId = %d ", parentId_);
  enum_ = std::dynamic_pointer_cast<CacheableEnum>(pr.readObject("m_enum"));
  LOGINFO("ParentPdx::fromData() read gender ");
  parentName_ = pr.readString("m_parentName");
  LOGINFO("ParentPdx::fromData() m_parentName = %s ", parentName_.c_str());
  childPdx_ = pr.readObject("m_childPdx");
  LOGINFO("ParentPdx::fromData() start3...");

  char_ = pr.readChar("m_char");
  charArray_ = pr.readCharArray("m_charArray");

  LOGINFO("ParentPdx::fromData() end...");
}

std::string ParentPdx::toString() const {
  std::stringstream strm;

  strm << "ParentPdx: [m_parentId=" << parentId_
       << "][m_parentName = " << parentName_ << "][m_childPdx = " << childPdx_
       << "] ";
  return strm.str();
}

bool ParentPdx::equals(const ParentPdx& other, bool isPdxReadSerialized) const {
  if ((parentName_ == other.parentName_) && (parentId_ == other.parentId_) &&
      (enum_->getEnumOrdinal() == other.enum_->getEnumOrdinal()) &&
      (enum_->getEnumClassName() == other.enum_->getEnumClassName()) &&
      (enum_->getEnumName() == other.enum_->getEnumName()) &&
      char_ == other.char_ && charArray_ == other.charArray_) {
    if (!isPdxReadSerialized) {
      ChildPdx* ch1 = dynamic_cast<ChildPdx*>(childPdx_.get());
      ChildPdx* ch2 = dynamic_cast<ChildPdx*>(other.childPdx_.get());

      if (ch1->equals(*ch2)) {
        return true;
      }
    }
    return true;
  }

  return false;
}
}  // namespace testobject
