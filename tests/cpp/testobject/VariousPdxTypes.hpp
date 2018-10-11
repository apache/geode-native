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

#ifndef GEODE_TESTOBJECT_VARIOUSPDXTYPES_H_
#define GEODE_TESTOBJECT_VARIOUSPDXTYPES_H_

#include <geode/DataSerializable.hpp>
#include <geode/PdxReader.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>

#include "testobject_export.h"

namespace PdxTests {

using apache::geode::client::Cacheable;
using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;

/************************************************************
 *  PdxTypes1
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes1 : public PdxSerializable {
 private:
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes1();

  ~PdxTypes1() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes1";
    return className;
  }
  int32_t getm_i1() { return m_i1; }
  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes1>();
  }
};

/************************************************************
 *  PdxTypes2
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes2 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes2();

  ~PdxTypes2() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes2";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes2>();
  }
};

/************************************************************
 *  PdxTypes3
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes3 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes3();

  ~PdxTypes3() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes3";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes3>();
  }
};

/************************************************************
 *  PdxTypes4
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes4 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes4();

  ~PdxTypes4() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes4";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes4>();
  }
};

/************************************************************
 *  PdxTypes5
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes5 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes5();

  ~PdxTypes5() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes5";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes5>();
  }
};

/************************************************************
 *  PdxTypes6
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes6 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  std::vector<int8_t> bytes128;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes6();

  ~PdxTypes6() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes6";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes6>();
  }
};

/************************************************************
 *  PdxTypes7
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes7 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  int32_t m_i1;
  std::vector<int8_t> bytes38000;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes7();

  ~PdxTypes7() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes7";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes7>();
  }
};

/************************************************************
 *  PdxTypes8
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes8 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  int32_t m_i1;
  std::vector<int8_t> bytes300;
  std::shared_ptr<Cacheable> _enum;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxTypes8();

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes8";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes8>();
  }
};

/************************************************************
 *  PdxTypes9
 * *********************************************************/
class TESTOBJECT_EXPORT PdxTypes9 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  std::string m_s3;
  std::vector<int8_t> m_bytes66000;
  std::string m_s4;
  std::string m_s5;

 public:
  PdxTypes9();

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes9";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes9>();
  }
};

/************************************************************
 *  PdxTypes10
 * *********************************************************/

class TESTOBJECT_EXPORT PdxTypes10 : public PdxSerializable {
 private:
  std::string m_s1;  //"one"
  std::string m_s2;
  std::string m_s3;
  std::vector<int8_t> m_bytes66000;
  std::string m_s4;
  std::string m_s5;

 public:
  PdxTypes10();

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxTypes10";
    return className;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxTypes10>();
  }
};

/************************************************************
 *  NestedPdx
 * *********************************************************/

class TESTOBJECT_EXPORT NestedPdx : public PdxSerializable {
 private:
  std::shared_ptr<PdxTypes1> m_pd1;
  std::shared_ptr<PdxTypes2> m_pd2;
  std::string m_s1;  //"one"
  std::string m_s2;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  NestedPdx();
  explicit NestedPdx(char* key);

  ~NestedPdx() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.NestedPdx";
    return className;
  }

  const std::string& getString() { return m_s1; }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<NestedPdx>();
  }
};

/************************************************************
 *  Mixed Version NestedPdx
 * *********************************************************/

class TESTOBJECT_EXPORT MixedVersionNestedPdx : public PdxSerializable {
 private:
  std::shared_ptr<PdxTypes1> m_pd1;
  std::shared_ptr<PdxTypes2> m_pd2;
  std::string m_s1;  //"one"
  std::string m_s2;
  std::string m_s3;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  MixedVersionNestedPdx();
  explicit MixedVersionNestedPdx(char* key);

  ~MixedVersionNestedPdx() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<PdxSerializable> obj);

  std::string toString() const override;

  using PdxSerializable::fromData;
  using PdxSerializable::toData;

  virtual void fromData(PdxReader& pr) override;

  virtual void toData(PdxWriter& pw) const override;

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.MixedVersionNestedPdx";
    return className;
  }

  const std::string& getString() { return m_s1; }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<MixedVersionNestedPdx>();
  }
};

/************************************************************
 *  PdxInsideIGeodeSerializable
 * *********************************************************/

class TESTOBJECT_EXPORT PdxInsideIGeodeSerializable : public DataSerializable {
 private:
  std::shared_ptr<NestedPdx> m_npdx;
  std::shared_ptr<PdxTypes3> m_pdx3;

  std::string m_s1;  //"one"
  std::string m_s2;
  int32_t m_i1;
  int32_t m_i2;
  int32_t m_i3;
  int32_t m_i4;

 public:
  PdxInsideIGeodeSerializable();

  ~PdxInsideIGeodeSerializable() noexcept override;

  int32_t getHashCode();

  bool equals(std::shared_ptr<Serializable> obj);

  std::string toString() const override;

  virtual void fromData(DataInput& input) override;

  void toData(DataOutput& output) const override;

  static std::shared_ptr<Serializable> createDeserializable() {
    return std::make_shared<PdxInsideIGeodeSerializable>();
  }
};

} /* namespace PdxTests */

#endif  // GEODE_TESTOBJECT_VARIOUSPDXTYPES_H_
