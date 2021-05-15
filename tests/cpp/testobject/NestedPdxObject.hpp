#pragma once

#ifndef GEODE_TESTOBJECT_NESTEDPDXOBJECT_H_
#define GEODE_TESTOBJECT_NESTEDPDXOBJECT_H_

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
 * NestedPdxObject.hpp
 *
 */

#include <util/Log.hpp>

#include <geode/CacheableEnum.hpp>
#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace testobject {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableEnum;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;

enum Gender { male, female, other };

class TESTOBJECT_EXPORT ChildPdx : public PdxSerializable {
 private:
  int m_childId;
  std::string m_childName;
  std::shared_ptr<CacheableEnum> m_enum;

 public:
  ChildPdx() {}

  explicit ChildPdx(int id) {
    m_childId = id;
    m_childName = "name-" + std::to_string(id);
    m_enum = CacheableEnum::create("Gender", "female", 5);
  }

  ~ChildPdx() override = default;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(ChildPdx);
    return objectSize;
  }

  int32_t getChildId() { return m_childId; }

  const std::string& getChildName() { return m_childName; }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void toData(PdxWriter& pw) const override;

  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;

  const std::string& getClassName() const override {
    static std::string className = "testobject.ChildPdx";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<ChildPdx>();
  }

  bool equals(ChildPdx& other) const;
};

class TESTOBJECT_EXPORT ParentPdx : public PdxSerializable {
 private:
  int m_parentId;
  std::string m_parentName;
  std::shared_ptr<Cacheable> m_childPdx;
  std::shared_ptr<CacheableEnum> m_enum;
  char16_t m_char;
  std::vector<char16_t> m_charArray;

 public:
  ParentPdx() {}

  explicit ParentPdx(int id) {
    m_parentId = id;
    m_parentName = "name-" + std::to_string(id);
    m_childPdx = std::make_shared<ChildPdx>(id /** 1393*/);
    m_enum = CacheableEnum::create("Gender", "male", 6);

    m_char = 'C';

    m_charArray = std::vector<char16_t>({'X', 'Y'});
  }

  ~ParentPdx() override = default;

  virtual size_t objectSize() const override {
    auto objectSize = sizeof(ParentPdx);
    return objectSize;
  }

  int32_t getParentId() { return m_parentId; }

  const std::string& getParentName() { return m_parentName; }

  std::shared_ptr<ChildPdx> getChildPdx() {
    return std::dynamic_pointer_cast<ChildPdx>(m_childPdx);
  }

  std::shared_ptr<CacheableEnum> getEnum() { return m_enum; }

  char16_t getChar() { return m_char; }

  std::vector<char16_t> getCharArray() { return m_charArray; }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void toData(PdxWriter& pw) const override;

  virtual void fromData(PdxReader& pr) override;

  std::string toString() const override;

  const std::string& getClassName() const override {
    static std::string className = "testobject.ParentPdx";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<ParentPdx>();
  }

  bool equals(const ParentPdx& other, bool isPdxReadSerialized) const;
};

enum enumQuerytest { id1, id2, id3 };

class TESTOBJECT_EXPORT PdxEnumTestClass : public PdxSerializable {
 private:
  int m_id;
  std::shared_ptr<CacheableEnum> m_enumid;

 public:
  int getID() { return m_id; }

  std::shared_ptr<CacheableEnum> getEnumID() { return m_enumid; }

  explicit PdxEnumTestClass(int id) {
    m_id = id;
    switch (m_id) {
      case 0:
        LOG_INFO("case 0 id1 = %d ", id1);
        m_enumid = CacheableEnum::create("enumQuerytest", "id1",
                                         /*enumQuerytest::*/ id1);
        break;
      case 1:
        LOG_INFO("case 1 id2 = %d ", id2);
        m_enumid = CacheableEnum::create("enumQuerytest", "id2",
                                         /*enumQuerytest::*/ id2);
        break;
      case 2:
        LOG_INFO("case 2 id3 = %d ", id3);
        m_enumid = CacheableEnum::create("enumQuerytest", "id3",
                                         /*enumQuerytest::*/ id3);
        break;
      default:
        LOG_INFO("case default id1 = %d ", id1);
        m_enumid = CacheableEnum::create("enumQuerytest", "id1",
                                         /*enumQuerytest::*/ id1);
        break;
    }
  }

  PdxEnumTestClass() {}

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeInt("m_id", m_id);
    pw.writeObject("m_enumid", m_enumid);
  }

  void fromData(PdxReader& pr) override {
    m_id = pr.readInt("m_id");
    m_enumid =
        std::dynamic_pointer_cast<CacheableEnum>(pr.readObject("m_enumid"));
  }

  std::string toString() const override { return "PdxEnumTestClass"; }

  const std::string& getClassName() const override {
    static std::string className = "testobject.PdxEnumTestClass";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxEnumTestClass>();
  }
};

class TESTOBJECT_EXPORT SerializePdx : public PdxSerializable {
 private:
  int i1;
  int i2;
  std::string s1;
  std::string s2;

 public:
  SerializePdx() {}

  explicit SerializePdx(bool init) {
    if (init) {
      i1 = 1;
      i2 = 2;
      s1 = "s1";
      s2 = "s2";
    } else {
      i1 = 0;
      i2 = 0;
      s1 = "";
      s2 = "";
    }
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<SerializePdx>(false);
  }

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeInt("i1", i1);
    pw.markIdentityField("i1");
    pw.writeInt("i2", i2);
    pw.writeString("s1", s1);
    pw.markIdentityField("s1");
    pw.writeString("s2", s2);
  }

  void fromData(PdxReader& pr) override {
    i1 = pr.readInt("i1");
    i2 = pr.readInt("i2");
    s1 = pr.readString("s1");
    s2 = pr.readString("s2");
  }

  std::string toString() const override { return "SerializePdx"; }

  const std::string& getClassName() const override {
    static std::string className = "SerializePdx";
    return className;
  }

  bool equals(SerializePdx& other) const {
    SerializePdx* ot = dynamic_cast<SerializePdx*>(&other);
    if (ot) {
      LOG_INFO("SerializePdx::equals1");
      return false;
    }

    if (ot->i1 != i1 && ot->i2 != i2 && ot->s1 != s1 && ot->s2 != s2) {
      LOG_INFO("SerializePdx::equals2");
      return false;
    }
    LOG_INFO("SerializePdx::equals3");
    return true;
  }
};
}  // namespace testobject

#endif  // GEODE_TESTOBJECT_NESTEDPDXOBJECT_H_
