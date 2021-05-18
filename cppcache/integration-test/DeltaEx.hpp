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

#ifndef GEODE_INTEGRATION_TEST_DELTAEX_H_
#define GEODE_INTEGRATION_TEST_DELTAEX_H_

#include "fw_dunit.hpp"
#include <ace/OS.h>
#include <memory>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxWriter.hpp>
#include <geode/PdxReader.hpp>
#include <geode/Delta.hpp>
#include <geode/DataSerializable.hpp>

#include "CacheHelper.hpp"

namespace {  // NOLINT(google-build-namespaces)

using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;
using apache::geode::client::Delta;
using apache::geode::client::InvalidDeltaException;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;

class DeltaEx : public DataSerializable, public Delta {
 public:
  int counter;
  bool isDelta;

 public:
  static int toDeltaCount;
  static int toDataCount;
  static int fromDeltaCount;
  static int fromDataCount;
  static int cloneCount;
  DeltaEx() : Delta(), counter(1), isDelta(false) {}
  explicit DeltaEx(int count) : Delta(), counter(count), isDelta(false) {}
  DeltaEx(const DeltaEx& rhs) = default;

  virtual bool hasDelta() const override { return isDelta; }
  virtual void toDelta(DataOutput& out) const override {
    out.writeInt(counter);
    toDeltaCount++;
  }

  virtual void fromDelta(DataInput& in) override {
    LOG("From delta gets called");
    int32_t val = in.readInt32();
    if (fromDeltaCount == 1) {
      fromDeltaCount++;
      LOG("Invalid Delta exception thrown");
      throw InvalidDeltaException("aaannn");
    }
    counter += val;
    fromDeltaCount++;
  }
  virtual void toData(DataOutput& output) const override {
    output.writeInt(counter);
    toDataCount++;
  }
  virtual void fromData(DataInput& input) override {
    counter = input.readInt32();
    fromDataCount++;
  }
  std::shared_ptr<Delta> clone() const override {
    cloneCount++;
    return std::make_shared<DeltaEx>(*this);
  }
  ~DeltaEx() noexcept override {}
  void setDelta(bool delta) { this->isDelta = delta; }
  static std::shared_ptr<Serializable> create() {
    return std::make_shared<DeltaEx>();
  }
};

class PdxDeltaEx : public PdxSerializable, public Delta {
 public:
  int m_counter;
  bool m_isDelta;

 public:
  static int m_toDeltaCount;
  static int m_toDataCount;
  static int m_fromDeltaCount;
  static int m_fromDataCount;
  static int m_cloneCount;
  PdxDeltaEx() : Delta(), m_counter(1), m_isDelta(false) {}
  explicit PdxDeltaEx(int count)
      : Delta(), m_counter(count), m_isDelta(false) {}
  PdxDeltaEx(const PdxDeltaEx& rhs)
      : Delta(), m_counter(rhs.m_counter), m_isDelta(rhs.m_isDelta) {}
  virtual bool hasDelta() const override { return m_isDelta; }
  virtual void toDelta(DataOutput& out) const override {
    out.writeInt(m_counter);
    m_toDeltaCount++;
  }

  virtual void fromDelta(DataInput& in) override {
    LOG("From delta gets called");
    int32_t val = in.readInt32();
    if (m_fromDeltaCount == 1) {
      m_fromDeltaCount++;
      LOG("Invalid Delta exception thrown");
      throw InvalidDeltaException("aaannn");
    }
    m_counter += val;
    m_fromDeltaCount++;
  }

  const std::string& getClassName() const override {
    static std::string className = "PdxTests.PdxDeltaEx";
    return className;
  }

  using PdxSerializable::toData;

  void toData(PdxWriter& pw) const override {
    pw.writeInt("counter", m_counter);
    m_toDataCount++;
  }

  using PdxSerializable::fromData;

  void fromData(PdxReader& pr) override {
    m_counter = pr.readInt("counter");
    m_fromDataCount++;
  }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<PdxDeltaEx>();
  }

  std::shared_ptr<Delta> clone() const override {
    m_cloneCount++;
    return std::make_shared<PdxDeltaEx>(*this);
  }

  ~PdxDeltaEx() noexcept override {}

  void setDelta(bool delta) { this->m_isDelta = delta; }

  std::string toString() const override {
    char idbuf[1024];
    sprintf(idbuf, "PdxDeltaEx :: [counter=%d]  [isDelta=%d]", m_counter,
            m_isDelta);
    return idbuf;
  }
};

}  // namespace

#endif  // GEODE_INTEGRATION_TEST_DELTAEX_H_
