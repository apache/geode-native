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

#ifndef GEODE_SERIALIZATIONREGISTRY_H_
#define GEODE_SERIALIZATIONREGISTRY_H_

#include <string>
#include <functional>
#include <typeinfo>
#include <iostream>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include <ace/Hash_Map_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Null_Mutex.h>

#include <geode/internal/geode_globals.hpp>
#include <geode/internal/DataSerializableInternal.hpp>
#include <geode/Serializable.hpp>
#include <geode/PdxSerializer.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Delta.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/DataSerializable.hpp>

#include "util/concurrent/spinlock_mutex.hpp"
#include "NonCopyable.hpp"
#include "MemberListForVersionStamp.hpp"
#include "config.h"

#if defined(_MACOSX)
namespace ACE_VERSIONED_NAMESPACE_NAME {
// TODO CMake check type int64_t
template <>
class ACE_Export ACE_Hash<int64_t> {
 public:
  inline unsigned long operator()(int64_t t) const {
    return static_cast<long>(t);
  }
};

}  // namespace ACE_VERSIONED_NAMESPACE_NAME
#endif

namespace apache {
namespace geode {
namespace client {

typedef ACE_Hash_Map_Manager<int64_t, TypeFactoryMethod, ACE_Null_Mutex>
    IdToFactoryMap;

typedef ACE_Hash_Map_Manager<std::string, TypeFactoryMethodPdx, ACE_Null_Mutex>
    StrToPdxFactoryMap;

class TheTypeMap : private NonCopyable {
 private:
  IdToFactoryMap* m_map;
  IdToFactoryMap* m_map2;  // to hold Fixed IDs since GFE 5.7.
  StrToPdxFactoryMap* m_pdxTypemap;
  mutable util::concurrent::spinlock_mutex m_mapLock;
  mutable util::concurrent::spinlock_mutex m_map2Lock;
  mutable util::concurrent::spinlock_mutex m_pdxTypemapLock;

  std::unordered_map<std::type_index, int32_t> typeToClassId;

 public:
  TheTypeMap() {
    m_map = new IdToFactoryMap();

    // second map to hold internal Data Serializable Fixed IDs - since GFE 5.7
    m_map2 = new IdToFactoryMap();

    // map to hold PDX types <string, funptr>.
    m_pdxTypemap = new StrToPdxFactoryMap();

    setup();
  }

  virtual ~TheTypeMap() {
    if (m_map != nullptr) {
      delete m_map;
    }

    if (m_map2 != nullptr) {
      delete m_map2;
    }

    if (m_pdxTypemap != nullptr) {
      delete m_pdxTypemap;
    }
  }

  void setup();

  void clear();

  void find(int64_t id, TypeFactoryMethod& func) const;

  void find2(int64_t id, TypeFactoryMethod& func) const;

  // int32_t findClassId(const std::type_info&& typeIndex) const;

  void bind(TypeFactoryMethod func, uint32_t id);

  inline void rebind(int64_t compId, TypeFactoryMethod func);

  inline void unbind(int64_t compId);

  inline void bind2(TypeFactoryMethod func);

  inline void rebind2(int64_t compId, TypeFactoryMethod func);

  inline void unbind2(int64_t compId);

  inline void bindPdxType(TypeFactoryMethodPdx func);

  inline void findPdxType(const std::string& objFullName,
                          TypeFactoryMethodPdx& func) const;

  inline void unbindPdxType(const std::string& objFullName);

  void rebindPdxType(std::string objFullName, TypeFactoryMethodPdx func);
};

class Pool;

/**
 * Used to register handlers for the PDX DsCode. .NET client extends this to
 * intercept PDX (de)serialization.
 */
class PdxTypeHandler {
 public:
  virtual ~PdxTypeHandler() noexcept = default;
  virtual void serialize(
      const std::shared_ptr<PdxSerializable>& pdxSerializable,
      DataOutput& dataOutput) const;
  virtual std::shared_ptr<PdxSerializable> deserialize(
      DataInput& dataInput) const;
};

class APACHE_GEODE_EXPORT SerializationRegistry {
 public:
  SerializationRegistry() : theTypeMap() {}

  /** write the length of the serialization, write the typeId of the object,
   * then write whatever the object's toData requires. The length at the
   * front is backfilled after the serialization.
   */
  inline void serialize(const Serializable* obj, DataOutput& output,
                        bool isDelta = false) const {
    if (obj == nullptr) {
      output.write(static_cast<int8_t>(DSCode::NullObj));
    } else if (const auto dataSerializableFixedId =
                   dynamic_cast<const DataSerializableFixedId*>(obj)) {
      serialize(dataSerializableFixedId, output);
    } else if (const auto dataSerializablePrimitive =
                   dynamic_cast<const DataSerializablePrimitive*>(obj)) {
      serialize(dataSerializablePrimitive, output);
    } else if (const auto dataSerializable =
                   dynamic_cast<const DataSerializable*>(obj)) {
      serialize(dataSerializable, output, isDelta);
    } else if (const auto dataSerializableInternal =
                   dynamic_cast<const DataSerializableInternal*>(obj)) {
      serialize(dataSerializableInternal, output);
    } else {
      throw UnsupportedOperationException(
          "SerializationRegistry::serialize: Serialization type not "
          "implemented.");
    }
  }

  inline void serialize(const std::shared_ptr<Serializable>& obj,
                        DataOutput& output, bool isDelta = false) const {
    if (obj == nullptr) {
      output.write(static_cast<int8_t>(DSCode::NullObj));
    } else if (auto&& pdxSerializable =
                   std::dynamic_pointer_cast<PdxSerializable>(obj)) {
      serialize(pdxSerializable, output);
    } else {
      serialize(obj.get(), output, isDelta);
    }
  }

  inline void serializeWithoutHeader(const Serializable* obj,
                                     DataOutput& output) const {
    if (const auto dataSerializableFixedId =
            dynamic_cast<const DataSerializableFixedId*>(obj)) {
      serializeWithoutHeader(dataSerializableFixedId, output);
    } else if (const auto dataSerializablePrimitive =
                   dynamic_cast<const DataSerializablePrimitive*>(obj)) {
      serializeWithoutHeader(dataSerializablePrimitive, output);
    } else if (const auto dataSerializable =
                   dynamic_cast<const DataSerializable*>(obj)) {
      serializeWithoutHeader(dataSerializable, output);
    } else if (const auto pdxSerializable =
                   dynamic_cast<const PdxSerializable*>(obj)) {
      serializeWithoutHeader(pdxSerializable, output);
    } else if (const auto dataSerializableInternal =
                   dynamic_cast<const DataSerializableInternal*>(obj)) {
      serializeWithoutHeader(dataSerializableInternal, output);
    } else {
      throw UnsupportedOperationException(
          "SerializationRegistry::serializeWithoutHeader: Serialization type "
          "not implemented.");
    }
  }

  inline void serializeWithoutHeader(const std::shared_ptr<Serializable>& obj,
                                     DataOutput& output) const {
    if (auto&& pdxSerializable =
            std::dynamic_pointer_cast<PdxSerializable>(obj)) {
      serializeWithoutHeader(pdxSerializable, output);
    } else {
      serializeWithoutHeader(obj.get(), output);
    }
  }

  inline void serialize(const std::shared_ptr<Serializable>& obj,
                        DataOutput& output) const {
    serialize(obj.get(), output);
  }

  /**
   * Read the length, typeid, and run the objs fromData. Returns the New
   * object.
   */
  std::shared_ptr<Serializable> deserialize(DataInput& input,
                                            int8_t typeId = -1) const;

  void addType(TypeFactoryMethod func, uint32_t id);

  void addType(int64_t compId, TypeFactoryMethod func);

  void addPdxType(TypeFactoryMethodPdx func);

  void setPdxSerializer(std::shared_ptr<PdxSerializer> pdxSerializer);

  std::shared_ptr<PdxSerializer> getPdxSerializer();

  void removeType(int64_t compId);

  // following for internal types with Data Serializable Fixed IDs  - since GFE
  // 5.7

  void addType2(TypeFactoryMethod func);

  void addType2(int64_t compId, TypeFactoryMethod func);

  void removeType2(int64_t compId);

  int32_t GetPDXIdForType(Pool* pool,
                          std::shared_ptr<Serializable> pdxType) const;

  std::shared_ptr<Serializable> GetPDXTypeById(Pool* pool,
                                               int32_t typeId) const;

  int32_t GetEnumValue(std::shared_ptr<Pool> pool,
                       std::shared_ptr<Serializable> enumInfo) const;

  std::shared_ptr<Serializable> GetEnum(std::shared_ptr<Pool> pool,
                                        int32_t val) const;

  std::shared_ptr<PdxSerializable> getPdxType(
      const std::string& className) const;

  void setPdxTypeHandler(PdxTypeHandler* handler) {
    this->pdxTypeHandler = std::unique_ptr<PdxTypeHandler>(handler);
  }

 private:
  std::unique_ptr<PdxTypeHandler> pdxTypeHandler;
  std::shared_ptr<PdxSerializer> pdxSerializer;
  TheTypeMap theTypeMap;

  inline void serialize(const DataSerializableFixedId* obj,
                        DataOutput& output) const {
    auto id = static_cast<int32_t>(obj->getDSFID());
    if (id <= std::numeric_limits<int8_t>::max() &&
        id >= std::numeric_limits<int8_t>::min()) {
      output.write(static_cast<int8_t>(DSCode::FixedIDByte));
      output.write(static_cast<int8_t>(id));
    } else if (id <= std::numeric_limits<int16_t>::max() &&
               id >= std::numeric_limits<int16_t>::min()) {
      output.write(static_cast<int8_t>(DSCode::FixedIDShort));
      output.writeInt(static_cast<int16_t>(id));
    } else {
      output.write(static_cast<int8_t>(DSCode::FixedIDInt));
      output.writeInt(static_cast<int32_t>(id));
    }

    serializeWithoutHeader(obj, output);
  }

  inline void serializeWithoutHeader(const DataSerializableFixedId* obj,
                                     DataOutput& output) const {
    obj->toData(output);
  }

  inline void serialize(const DataSerializablePrimitive* obj,
                        DataOutput& output) const {
    auto id = obj->getDsCode();
    output.write(static_cast<int8_t>(id));

    serializeWithoutHeader(obj, output);
  }

  inline void serializeWithoutHeader(const DataSerializablePrimitive* obj,
                                     DataOutput& output) const {
    obj->toData(output);
  }

  inline void serialize(const DataSerializable* obj, DataOutput& output,
                        bool isDelta) const {
    // auto id = obj->getClassId();
    auto&& type = obj->getType();
    // int32_t id = theTypeMap.findClassId(type);
    int32_t id = typeToClassId[type];
    // std::string objname = typeid(obj).name();
    // std::cout << "obj has type: " << dsClass.name() << std::endl;

    if (isDelta) {
      const Delta* ptr = dynamic_cast<const Delta*>(obj);
      ptr->toDelta(output);
    } else {
      serializeWithoutHeader(obj, output);
    }
  }

  inline void serializeWithoutHeader(const DataSerializable* obj,
                                     DataOutput& output) const {
    obj->toData(output);
  }

  inline void serialize(const std::shared_ptr<PdxSerializable>& obj,
                        DataOutput& output) const {
    output.write(static_cast<int8_t>(DSCode::PDX));

    serializeWithoutHeader(obj, output);
  }

  void serializeWithoutHeader(const std::shared_ptr<PdxSerializable>& obj,
                              DataOutput& output) const;

  inline void serialize(const DataSerializableInternal* obj,
                        DataOutput& output) const {
    serializeWithoutHeader(obj, output);
  }

  inline void serializeWithoutHeader(const DataSerializableInternal* obj,
                                     DataOutput& output) const {
    obj->toData(output);
  }

 public:
  static inline DSCode getSerializableDataDsCode(int32_t classId) {
    if (classId <= std::numeric_limits<int8_t>::max() &&
        classId >= std::numeric_limits<int8_t>::min()) {
      return DSCode::CacheableUserData;
    } else if (classId <= std::numeric_limits<int16_t>::max() &&
               classId >= std::numeric_limits<int16_t>::min()) {
      return DSCode::CacheableUserData2;
    } else {
      return DSCode::CacheableUserData4;
    }
  }

 private:
  void deserialize(DataInput& input, std::shared_ptr<Serializable> obj) const;

  void deserialize(DataInput& input,
                   std::shared_ptr<DataSerializableInternal> obj) const;

  void deserialize(DataInput& input,
                   std::shared_ptr<DataSerializableFixedId> obj) const;

  void deserialize(DataInput& input,
                   std::shared_ptr<DataSerializablePrimitive> obj) const;

  void deserialize(DataInput& input,
                   std::shared_ptr<DataSerializable> obj) const;

  [[noreturn]] void deserialize(DataInput& input,
                                std::shared_ptr<PdxSerializable> obj) const;
};  // namespace client

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZATIONREGISTRY_H_
