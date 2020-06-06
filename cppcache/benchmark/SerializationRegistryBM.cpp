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

#include <benchmark/benchmark.h>

#include <mutex>
#include <thread>

#include "SerializationRegistry.hpp"

using apache::geode::client::DataInput;
using apache::geode::client::DataOutput;
using apache::geode::client::DataSerializable;
using apache::geode::client::PdxReader;
using apache::geode::client::PdxSerializable;
using apache::geode::client::PdxWriter;
using apache::geode::client::TheTypeMap;
using apache::geode::client::TypeFactoryMethod;
using apache::geode::client::TypeFactoryMethodPdx;
using apache::geode::client::internal::DSCode;
using apache::geode::client::internal::DSFid;

class TestPdxClass : public PdxSerializable {
 public:
  TestPdxClass() = default;

  void fromData(PdxReader&) override {}

  void toData(PdxWriter&) const override {}

  const std::string& getClassName() const override { return className; }

  static std::shared_ptr<PdxSerializable> createDeserializable() {
    return std::make_shared<TestPdxClass>();
  }

 private:
  std::string className = "mypdxclass";
};

class TestDataSerializableClass : public DataSerializable {
 public:
  TestDataSerializableClass() = default;

  void fromData(DataInput&) override {}

  void toData(DataOutput&) const override {}

  static std::shared_ptr<DataSerializable> createInstance() {
    return std::make_shared<TestDataSerializableClass>();
  }
};

static void SerializationRegistryBM_findDataSerializablePrimitive(
    benchmark::State& state) {
  TheTypeMap theTypeMap;
  for (auto _ : state) {
    TypeFactoryMethod func;
    theTypeMap.findDataSerializablePrimitive(DSCode::CacheableString, func);
  }
}

static void SerializationRegistryBM_findDataSerializableFixedId(
    benchmark::State& state) {
  TheTypeMap theTypeMap;
  for (auto _ : state) {
    TypeFactoryMethod func;
    theTypeMap.findDataSerializableFixedId(DSFid::EventId, func);
  }
}

static void SerializationRegistryBM_findDataSerializable(
    benchmark::State& state) {
  TheTypeMap theTypeMap;
  theTypeMap.bindDataSerializable(TestDataSerializableClass::createInstance,
                                  1971);
  for (auto _ : state) {
    TypeFactoryMethod func;
    theTypeMap.findDataSerializable(1971, func);
  }
}

static void SerializationRegistryBM_findPdxSerializable(
    benchmark::State& state) {
  TheTypeMap theTypeMap;
  theTypeMap.bindPdxSerializable(TestPdxClass::createDeserializable);
  for (auto _ : state) {
    theTypeMap.findPdxSerializable("mypdxclass");
  }
}

const auto MAX_THREADS = std::thread::hardware_concurrency() * 8;

BENCHMARK(SerializationRegistryBM_findDataSerializablePrimitive)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();

BENCHMARK(SerializationRegistryBM_findDataSerializableFixedId)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();

BENCHMARK(SerializationRegistryBM_findDataSerializable)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();

BENCHMARK(SerializationRegistryBM_findPdxSerializable)
    ->ThreadRange(1, MAX_THREADS)
    ->UseRealTime();
