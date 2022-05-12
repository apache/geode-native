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

#ifndef GEODE_PDXINSTANCEIMPL_H_
#define GEODE_PDXINSTANCEIMPL_H_

#include <map>
#include <vector>

#include <boost/thread.hpp>

#include <geode/PdxInstance.hpp>

namespace apache {
namespace geode {
namespace client {

class CachePerfStats;
class DataOutput;
class PdxFieldType;
class PdxReaderImpl;
class PdxType;
class PdxWriterImpl;
class PdxInstanceFactory;

class PdxInstanceImpl : virtual public PdxInstance {
 protected:
  using FieldsBuffer = std::vector<int8_t>;
  using Fields = std::vector<std::shared_ptr<Cacheable>>;

 public:
  PdxInstanceImpl(FieldsBuffer buffer, std::shared_ptr<PdxType> pdxType,
                  const CacheImpl& cacheImpl);

  PdxInstanceImpl(Fields fields, std::shared_ptr<PdxType> pdxType,
                  const CacheImpl& cacheImpl);

  PdxInstanceImpl(Fields fields, FieldsBuffer buffer,
                  std::shared_ptr<PdxType> pdxType, const CacheImpl& cacheImpl);

  PdxInstanceImpl(const PdxInstanceImpl& other) = delete;

  void operator=(const PdxInstanceImpl& other) = delete;

  ~PdxInstanceImpl() noexcept override;

  virtual std::shared_ptr<PdxSerializable> getObject() override;

  virtual bool hasField(const std::string& name) override;

  virtual bool getBooleanField(const std::string& name) const override;

  virtual int8_t getByteField(const std::string& name) const override;

  virtual int16_t getShortField(const std::string& name) const override;

  virtual int32_t getIntField(const std::string& name) const override;

  virtual int64_t getLongField(const std::string& name) const override;

  virtual float getFloatField(const std::string& name) const override;

  virtual double getDoubleField(const std::string& name) const override;

  virtual char16_t getCharField(const std::string& name) const override;

  virtual std::string getStringField(const std::string& name) const override;

  virtual std::vector<bool> getBooleanArrayField(
      const std::string& name) const override;

  virtual std::vector<int8_t> getByteArrayField(
      const std::string& name) const override;

  virtual std::vector<int16_t> getShortArrayField(
      const std::string& name) const override;

  virtual std::vector<int32_t> getIntArrayField(
      const std::string& name) const override;

  virtual std::vector<int64_t> getLongArrayField(
      const std::string& name) const override;

  virtual std::vector<float> getFloatArrayField(
      const std::string& name) const override;

  virtual std::vector<double> getDoubleArrayField(
      const std::string& name) const override;

  // charArray
  virtual std::vector<char16_t> getCharArrayField(
      const std::string& name) const override;

  virtual std::vector<std::string> getStringArrayField(
      const std::string& name) const override;

  virtual std::shared_ptr<CacheableDate> getCacheableDateField(
      const std::string& name) const override;

  virtual void getField(const std::string& name, int8_t*** value,
                        int32_t& arrayLength,
                        int32_t*& elementLength) const override;

  virtual std::shared_ptr<Cacheable> getCacheableField(
      const std::string& name) const override;

  virtual std::shared_ptr<CacheableObjectArray> getCacheableObjectArrayField(
      const std::string& name) const override;

  virtual bool isIdentityField(const std::string& name) override;

  virtual std::shared_ptr<WritablePdxInstance> createWriter() override;

  virtual int32_t hashcode() const override;

  virtual std::string toString() const override;

  virtual bool operator==(const CacheableKey& other) const override;

  virtual size_t objectSize() const override;

  virtual std::shared_ptr<CacheableStringArray> getFieldNames() override;

  // From PdxSerializable
  virtual void toData(PdxWriter& output) const override;

  virtual void fromData(PdxReader& input) override;

  virtual const std::string& getClassName() const override;

  virtual PdxFieldTypes getFieldType(
      const std::string& fieldname) const override;

  std::shared_ptr<PdxType> getPdxType() const { return pdxType_; }

 private:
  std::shared_ptr<Cacheable> getField(const std::string& name,
                                      PdxFieldTypes type) const;

  std::vector<int8_t> getFieldsBuffer() const;

  std::vector<int8_t> serialize() const;

  void serialize(PdxWriterImpl& writer) const;

  void deserialize();

 protected:
  friend class PdxInstanceFactory;

  static std::shared_ptr<Cacheable> toCacheableField(bool value);

  static std::shared_ptr<Cacheable> toCacheableField(int8_t value);

  static std::shared_ptr<Cacheable> toCacheableField(uint8_t value);

  static std::shared_ptr<Cacheable> toCacheableField(int16_t value);

  static std::shared_ptr<Cacheable> toCacheableField(int32_t value);

  static std::shared_ptr<Cacheable> toCacheableField(int64_t value);

  static std::shared_ptr<Cacheable> toCacheableField(float value);

  static std::shared_ptr<Cacheable> toCacheableField(double value);

  static std::shared_ptr<Cacheable> toCacheableField(char value);

  static std::shared_ptr<Cacheable> toCacheableField(char16_t value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<bool>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<int8_t>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<int16_t>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<int32_t>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<int64_t>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<float>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<double>& value);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<char16_t>& value);

  static std::shared_ptr<Cacheable> toCacheableField(const std::string& value);

  static std::shared_ptr<Cacheable> toCacheableField(int8_t** value,
                                                     int32_t arrayLength,
                                                     int32_t* elementLength);

  static std::shared_ptr<Cacheable> toCacheableField(
      const std::vector<std::string>& value);

  static std::shared_ptr<Cacheable> toCacheableField(std::string* value,
                                                     int32_t length);

 private:
  static void writeField(PdxWriterImpl& writer, PdxFieldTypes type,
                         std::shared_ptr<Cacheable> value);


  static bool isDefaultFieldValue(PdxFieldTypes type, Cacheable *value);

  static int32_t getRawFieldHashCode(PdxReaderImpl& reader, int32_t fieldIdx);

  static int32_t enumerateMapHashCode(std::shared_ptr<CacheableHashMap> map);

  static int32_t enumerateVectorHashCode(std::shared_ptr<CacheableVector> vec);

  static int32_t enumerateArrayListHashCode(
      std::shared_ptr<CacheableArrayList> arrList);

  static int32_t enumerateLinkedListHashCode(
      std::shared_ptr<CacheableLinkedList> linkedList);

  static int32_t enumerateObjectArrayHashCode(
      std::shared_ptr<CacheableObjectArray> objArray);

  static int32_t enumerateSetHashCode(std::shared_ptr<CacheableHashSet> set);

  static int32_t enumerateLinkedSetHashCode(
      std::shared_ptr<CacheableLinkedHashSet> linkedSet);

  static int32_t enumerateHashTableCode(
      std::shared_ptr<CacheableHashTable> hashTable);

  static int32_t deepHashCode(std::shared_ptr<Cacheable> object);

  static bool enumerateObjectArrayEquals(CacheableObjectArray* array,
                                         CacheableObjectArray* otherArray);

  static bool enumerateVectorEquals(CacheableVector* array,
                                    CacheableVector* otherArray);

  static bool enumerateArrayListEquals(CacheableArrayList* array,
                                       CacheableArrayList* otherArray);

  static bool enumerateLinkedListEquals(CacheableLinkedList* list,
                                       CacheableLinkedList* otherList);

  static bool enumerateMapEquals(CacheableHashMap* map,
                                 CacheableHashMap* otherMap);

  static bool enumerateHashTableEquals(CacheableHashTable* map,
                                       CacheableHashTable* otherMap);

  static bool enumerateSetEquals(CacheableHashSet* set,
                                 CacheableHashSet* otherSet);

  static bool enumerateLinkedSetEquals(CacheableLinkedHashSet* set,
                                       CacheableLinkedHashSet* otherSet);

  static bool deepArrayEquals(Cacheable* object, Cacheable* otherObject);

 protected:
  Fields fields_;
  mutable FieldsBuffer buffer_;

  std::shared_ptr<PdxType> pdxType_;
  mutable boost::shared_mutex mutex_;

  CachePerfStats& cacheStats_;
  const CacheImpl& cache_;
  bool enableTimeStatistics_;

 private:
  // For UT purposes
  friend class PdxInstanceImplTest;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCEIMPL_H_
