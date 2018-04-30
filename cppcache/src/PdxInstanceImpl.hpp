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

#include <vector>
#include <map>

#include <geode/PdxInstance.hpp>
#include <geode/WritablePdxInstance.hpp>
#include <geode/PdxSerializable.hpp>
#include <geode/PdxFieldTypes.hpp>

#include "PdxType.hpp"
#include "PdxLocalWriter.hpp"
#include "PdxTypeRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

typedef std::map<std::string, std::shared_ptr<Cacheable>> FieldVsValues;

class APACHE_GEODE_EXPORT PdxInstanceImpl : public WritablePdxInstance {
 public:
  ~PdxInstanceImpl() noexcept override;

  virtual std::shared_ptr<PdxSerializable> getObject() override;

  virtual bool hasField(const std::string& fieldname) override;

  virtual bool getBooleanField(const std::string& fieldname) const override;

  virtual int8_t getByteField(const std::string& fieldname) const override;

  virtual int16_t getShortField(const std::string& fieldname) const override;

  virtual int32_t getIntField(const std::string& fieldname) const override;

  virtual int64_t getLongField(const std::string& fieldname) const override;

  virtual float getFloatField(const std::string& fieldname) const override;

  virtual double getDoubleField(const std::string& fieldname) const override;

  virtual char16_t getCharField(const std::string& fieldName) const override;

  virtual std::string getStringField(
      const std::string& fieldName) const override;

  virtual std::vector<bool> getBooleanArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<int8_t> getByteArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<int16_t> getShortArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<int32_t> getIntArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<int64_t> getLongArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<float> getFloatArrayField(
      const std::string& fieldname) const override;

  virtual std::vector<double> getDoubleArrayField(
      const std::string& fieldname) const override;

  // charArray
  virtual std::vector<char16_t> getCharArrayField(
      const std::string& fieldName) const override;

  virtual std::vector<std::string> getStringArrayField(
      const std::string& fieldname) const override;

  virtual std::shared_ptr<CacheableDate> getCacheableDateField(
      const std::string& fieldname) const override;

  virtual void getField(const std::string& fieldName, int8_t*** value,
                        int32_t& arrayLength,
                        int32_t*& elementLength) const override;

  virtual std::shared_ptr<Cacheable> getCacheableField(
      const std::string& fieldname) const override;

  virtual std::shared_ptr<CacheableObjectArray> getCacheableObjectArrayField(
      const std::string& fieldname) const override;

  virtual void setField(const std::string& fieldName, bool value) override;

  virtual void setField(const std::string& fieldName,
                        signed char value) override;

  virtual void setField(const std::string& fieldName,
                        unsigned char value) override;

  virtual void setField(const std::string& fieldName, int16_t value) override;

  virtual void setField(const std::string& fieldName, int32_t value) override;

  virtual void setField(const std::string& fieldName, int64_t value) override;

  virtual void setField(const std::string& fieldName, float value) override;

  virtual void setField(const std::string& fieldName, double value) override;

  virtual void setField(const std::string& fieldName, char16_t value) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableDate> value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<bool>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int8_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int16_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int32_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<int64_t>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<float>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<double>& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::string& value) override;

  virtual void setField(const std::string& fieldName,
                        const std::vector<char16_t>& value) override;

  virtual void setField(const std::string& fieldName, std::string* value,
                        int32_t length) override;

  virtual void setField(const std::string& fieldName, int8_t** value,
                        int32_t arrayLength, int32_t* elementLength) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<Cacheable> value) override;

  virtual void setField(const std::string& fieldName,
                        std::shared_ptr<CacheableObjectArray> value) override;

  virtual bool isIdentityField(const std::string& fieldname) override;

  virtual std::shared_ptr<WritablePdxInstance> createWriter() override;

  virtual int32_t hashcode() const override;

  virtual std::string toString() const override;

  virtual void toData(DataOutput& output) const override {
    PdxInstance::toData(output);
  }

  virtual void fromData(DataInput& input) override {
    PdxInstance::fromData(input);
  }

  virtual bool operator==(const CacheableKey& other) const override;

  /** @return the size of the object in bytes
   * This is an internal method.
   * It is used in case of heap LRU property is set.
   */
  virtual size_t objectSize() const override;

  virtual std::shared_ptr<CacheableStringArray> getFieldNames() override;

  // From PdxSerializable
  virtual void toData(PdxWriter& output) const override;

  virtual void fromData(PdxReader& input) override;

  virtual const std::string& getClassName() const override;

  virtual PdxFieldTypes getFieldType(
      const std::string& fieldname) const override;

  void setPdxId(int32_t typeId);

 public:
  /**
   * @brief constructors
   */

  PdxInstanceImpl(uint8_t* buffer, int length, int typeId,
                  CachePerfStats& cacheStats, PdxTypeRegistry& pdxTypeRegistry,
                  const CacheImpl& cacheImpl, bool enableTimeStatistics);

  PdxInstanceImpl(FieldVsValues fieldVsValue, std::shared_ptr<PdxType> pdxType,
                  CachePerfStats& cacheStats, PdxTypeRegistry& pdxTypeRegistry,
                  const CacheImpl& cacheImpl, bool enableTimeStatistics);

  PdxInstanceImpl(const PdxInstanceImpl& other) = delete;

  void operator=(const PdxInstanceImpl& other) = delete;

  std::shared_ptr<PdxType> getPdxType() const;

  void updatePdxStream(uint8_t* newPdxStream, int len);

 private:
  uint8_t* m_buffer;
  int m_bufferLength;
  int m_typeId;
  std::shared_ptr<PdxType> m_pdxType;
  FieldVsValues m_updatedFields;
  CachePerfStats& m_cacheStats;
  PdxTypeRegistry& m_pdxTypeRegistry;
  const CacheImpl& m_cacheImpl;
  bool m_enableTimeStatistics;

  std::vector<std::shared_ptr<PdxFieldType>> getIdentityPdxFields(
      std::shared_ptr<PdxType> pt) const;

  int getOffset(DataInput& dataInput, std::shared_ptr<PdxType> pt,
                int sequenceId) const;

  int getRawHashCode(std::shared_ptr<PdxType> pt,
                     std::shared_ptr<PdxFieldType> pField,
                     DataInput& dataInput) const;

  int getNextFieldPosition(DataInput& dataInput, int fieldId,
                           std::shared_ptr<PdxType> pt) const;

  int getSerializedLength(DataInput& dataInput,
                          std::shared_ptr<PdxType> pt) const;

  bool hasDefaultBytes(std::shared_ptr<PdxFieldType> pField,
                       DataInput& dataInput, int start, int end) const;

  bool compareDefaultBytes(DataInput& dataInput, int start, int end,
                           int8_t* defaultBytes, int32_t length) const;

  void writeField(PdxWriter& writer, const std::string& fieldName,
                  PdxFieldTypes typeId, std::shared_ptr<Cacheable> value);

  void writeUnmodifieldField(DataInput& dataInput, int startPos, int endPos,
                             PdxLocalWriter& localWriter);

  void setOffsetForObject(DataInput& dataInput, std::shared_ptr<PdxType> pt,
                          int sequenceId) const;

  bool compareRawBytes(PdxInstanceImpl& other, std::shared_ptr<PdxType> myPT,
                       std::shared_ptr<PdxFieldType> myF,
                       DataInput& myDataInput, std::shared_ptr<PdxType> otherPT,
                       std::shared_ptr<PdxFieldType> otherF,
                       DataInput& otherDataInput) const;

  void equatePdxFields(std::vector<std::shared_ptr<PdxFieldType>>& my,
                       std::vector<std::shared_ptr<PdxFieldType>>& other) const;

  PdxTypeRegistry& getPdxTypeRegistry() const;

  void toDataMutable(PdxWriter& output);

  static int deepArrayHashCode(std::shared_ptr<Cacheable> obj);

  static int enumerateMapHashCode(std::shared_ptr<CacheableHashMap> map);

  static int enumerateVectorHashCode(std::shared_ptr<CacheableVector> vec);

  static int enumerateArrayListHashCode(
      std::shared_ptr<CacheableArrayList> arrList);

  static int enumerateLinkedListHashCode(
      std::shared_ptr<CacheableLinkedList> linkedList);

  static int enumerateObjectArrayHashCode(
      std::shared_ptr<CacheableObjectArray> objArray);

  static int enumerateSetHashCode(std::shared_ptr<CacheableHashSet> set);

  static int enumerateLinkedSetHashCode(
      std::shared_ptr<CacheableLinkedHashSet> linkedset);

  static int enumerateHashTableCode(
      std::shared_ptr<CacheableHashTable> hashTable);

  static bool deepArrayEquals(std::shared_ptr<Cacheable> obj,
                              std::shared_ptr<Cacheable> otherObj);

  static bool enumerateObjectArrayEquals(
      std::shared_ptr<CacheableObjectArray> Obj,
      std::shared_ptr<CacheableObjectArray> OtherObj);

  static bool enumerateVectorEquals(std::shared_ptr<CacheableVector> Obj,
                                    std::shared_ptr<CacheableVector> OtherObj);

  static bool enumerateArrayListEquals(
      std::shared_ptr<CacheableArrayList> Obj,
      std::shared_ptr<CacheableArrayList> OtherObj);

  static bool enumerateMapEquals(std::shared_ptr<CacheableHashMap> Obj,
                                 std::shared_ptr<CacheableHashMap> OtherObj);

  static bool enumerateSetEquals(std::shared_ptr<CacheableHashSet> Obj,
                                 std::shared_ptr<CacheableHashSet> OtherObj);

  static bool enumerateLinkedSetEquals(
      std::shared_ptr<CacheableLinkedHashSet> Obj,
      std::shared_ptr<CacheableLinkedHashSet> OtherObj);

  static bool enumerateHashTableEquals(
      std::shared_ptr<CacheableHashTable> Obj,
      std::shared_ptr<CacheableHashTable> OtherObj);

  DataInput getDataInputForField(
      const std::string& fieldname) const;

  static int8_t m_BooleanDefaultBytes[];
  static int8_t m_ByteDefaultBytes[];
  static int8_t m_CharDefaultBytes[];
  static int8_t m_ShortDefaultBytes[];
  static int8_t m_IntDefaultBytes[];
  static int8_t m_LongDefaultBytes[];
  static int8_t m_FloatDefaultBytes[];
  static int8_t m_DoubleDefaultBytes[];
  static int8_t m_DateDefaultBytes[];
  static int8_t m_StringDefaultBytes[];
  static int8_t m_ObjectDefaultBytes[];
  static int8_t m_NULLARRAYDefaultBytes[];
  static std::shared_ptr<PdxFieldType> m_DefaultPdxFieldType;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXINSTANCEIMPL_H_
