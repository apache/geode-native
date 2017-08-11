#pragma once

#ifndef GEODE_SERIALIZATIONREGISTRY_H_
#define GEODE_SERIALIZATIONREGISTRY_H_

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

#include <geode/geode_globals.hpp>

#include <geode/Serializable.hpp>
#include <geode/PdxSerializer.hpp>
#include <ace/Hash_Map_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Null_Mutex.h>
#include <geode/GeodeTypeIds.hpp>
#include "GeodeTypeIdsImpl.hpp"
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Delta.hpp>
#include <string>
#include "util/concurrent/spinlock_mutex.hpp"
#include "NonCopyable.hpp"
#include <geode/PdxSerializable.hpp>
#include "MemberListForVersionStamp.hpp"

#if defined(_MACOSX)
ACE_BEGIN_VERSIONED_NAMESPACE_DECL
// TODO CMake check type int64_t
template <>
class ACE_Export ACE_Hash<int64_t> {
 public:
  inline unsigned long operator()(int64_t t) const {
    return static_cast<long>(t);
  }
};

ACE_END_VERSIONED_NAMESPACE_DECL
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

  void bind(TypeFactoryMethod func);

  inline void rebind(int64_t compId, TypeFactoryMethod func);
  inline void unbind(int64_t compId);
  inline void bind2(TypeFactoryMethod func);

  inline void rebind2(int64_t compId, TypeFactoryMethod func);

  inline void unbind2(int64_t compId);
  inline void bindPdxType(TypeFactoryMethodPdx func);
  inline void findPdxType(char* objFullName, TypeFactoryMethodPdx& func);
  inline void unbindPdxType(char* objFullName);

  void rebindPdxType(char* objFullName, TypeFactoryMethodPdx func);
};

class CPPCACHE_EXPORT SerializationRegistry {
 public:
  SerializationRegistry() : theTypeMap() {}

  /** write the length of the serialization, write the typeId of the object,
   * then write whatever the object's toData requires. The length at the
   * front is backfilled after the serialization.
   */
  inline void serialize(const Serializable* obj, DataOutput& output,
                        bool isDelta = false) const {
    if (obj == nullptr) {
      output.write(static_cast<int8_t>(GeodeTypeIds::NullObj));
    } else {
      int8_t typeId = obj->typeId();
      switch (obj->DSFID()) {
        case GeodeTypeIdsImpl::FixedIDByte:
          output.write(static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDByte));
          output.write(typeId);  // write the type ID.
          break;
        case GeodeTypeIdsImpl::FixedIDShort:
          output.write(static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDShort));
          output.writeInt(static_cast<int16_t>(typeId));  // write the type ID.
          break;
        case GeodeTypeIdsImpl::FixedIDInt:
          output.write(static_cast<int8_t>(GeodeTypeIdsImpl::FixedIDInt));
          output.writeInt(static_cast<int32_t>(typeId));  // write the type ID.
          break;
        default:
          output.write(typeId);  // write the type ID.
          break;
      }

      if (static_cast<int32_t>(typeId) == GeodeTypeIdsImpl::CacheableUserData) {
        output.write(static_cast<int8_t>(obj->classId()));
      } else if (static_cast<int32_t>(typeId) ==
                 GeodeTypeIdsImpl::CacheableUserData2) {
        output.writeInt(static_cast<int16_t>(obj->classId()));
      } else if (static_cast<int32_t>(typeId) ==
                 GeodeTypeIdsImpl::CacheableUserData4) {
        output.writeInt(obj->classId());
      }
      if (isDelta) {
        const Delta* ptr = dynamic_cast<const Delta*>(obj);
        ptr->toDelta(output);
      } else {
        obj->toData(output);  // let the obj serialize itself.
      }
    }
  }

  inline void serialize(const SerializablePtr& obj, DataOutput& output) const {
    serialize(obj.get(), output);
  }

  /**
   * Read the length, typeid, and run the objs fromData. Returns the New
   * object.
   */
  SerializablePtr deserialize(DataInput& input, int8_t typeId = -1) const;

  void addType(TypeFactoryMethod func);

  void addType(int64_t compId, TypeFactoryMethod func);

  void addPdxType(TypeFactoryMethodPdx func);

  void setPdxSerializer(PdxSerializerPtr pdxSerializer);

  PdxSerializerPtr getPdxSerializer();

  void removeType(int64_t compId);

  // following for internal types with Data Serializable Fixed IDs  - since GFE
  // 5.7

  void addType2(TypeFactoryMethod func);

  void addType2(int64_t compId, TypeFactoryMethod func);

  void removeType2(int64_t compId);

  int32_t GetPDXIdForType(PoolPtr pool, SerializablePtr pdxType) const;

  SerializablePtr GetPDXTypeById(PoolPtr pool, int32_t typeId) const;

  int32_t GetEnumValue(PoolPtr pool, SerializablePtr enumInfo) const;
  SerializablePtr GetEnum(PoolPtr pool, int32_t val) const;

  PdxSerializablePtr getPdxType(char* className);

 private:
  PdxSerializerPtr m_pdxSerializer;
  TheTypeMap theTypeMap;
};

typedef std::shared_ptr<SerializationRegistry> SerializationRegistryPtr;
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SERIALIZATIONREGISTRY_H_
