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

#include <gtest/gtest.h>

#include <geode/CacheFactory.hpp>
#include <PdxType.hpp>
#include <PdxLocalReader.hpp>
#include <PdxLocalWriter.hpp>
#include <PdxTypeRegistry.hpp>
#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"

using namespace apache::geode::client;

class MyPdxClass : public PdxSerializable {
 public:
  MyPdxClass();
  ~MyPdxClass();
  virtual void toData(std::shared_ptr<PdxWriter> output);
  virtual void fromData(std::shared_ptr<PdxReader> input);
  virtual void setAString(std::string a_string);
  virtual std::string getAString();
  virtual const char *getClassName() const;

  static PdxSerializable *CreateDeserializable();

 private:
  std::string _a_string;
};

MyPdxClass::MyPdxClass() { _a_string = ""; }

void MyPdxClass::setAString(std::string a_string) { _a_string = a_string; }

std::string MyPdxClass::getAString() { return _a_string; }

MyPdxClass::~MyPdxClass() {}

void MyPdxClass::toData(std::shared_ptr<PdxWriter> output) {
  output->writeString("name", _a_string.c_str());
}

void MyPdxClass::fromData(std::shared_ptr<PdxReader> input) {
  _a_string = input->readString("name");
}
const char *MyPdxClass::getClassName() const { return "MyPdxClass"; }

PdxSerializable *MyPdxClass::CreateDeserializable() { return new MyPdxClass(); }

class DISABLED_PdxLocalReaderTest : public ::testing::Test {
 public:
  void SetUp() {
    auto factory = CacheFactory::createCacheFactory();
    cache.reset(new Cache(factory->create()));
  }

 protected:
  std::shared_ptr<Cache> cache;
};

TEST_F(DISABLED_PdxLocalReaderTest, testSerializationOfPdxType) {
  MyPdxClass expected, actual;
  DataOutputInternal stream(cache.get());
  int length = 0;

  expected.setAString("the_expected_string");

  // TODO: Refactor static singleton patterns in PdxTypeRegistry so that
  // tests will not interfere with each other.

  auto pdxTypeRegistry =
      CacheRegionHelper::getCacheImpl(cache.get())->getPdxTypeRegistry();

  // C++ Client does not require pdxDomainClassName as it is only needed
  // for reflection purposes, which we do not support in C++. We pass in
  // getClassName() for consistency reasons only.
  auto pdx_type_ptr = std::make_shared<PdxType>(pdxTypeRegistry,
                                                expected.getClassName(), false);

  // Here we construct a serialized stream of bytes representing MyPdxClass.
  // The stream is later deserialization and validated for consistency.
  auto writer =
      std::make_shared<PdxLocalWriter>(stream, pdx_type_ptr, pdxTypeRegistry);
  expected.toData(writer);
  writer->endObjectWriting();
  uint8_t *raw_stream = writer->getPdxStream(length);

  DataInputInternal input(raw_stream, length, cache.get());
  auto reader = std::make_shared<PdxLocalReader>(input, pdx_type_ptr, length,
                                                 pdxTypeRegistry);

  actual.fromData(reader);

  EXPECT_EQ(actual.getAString(), "the_expected_string");
}
