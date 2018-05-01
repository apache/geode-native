#pragma once

#ifndef GEODE_INTEGRATION_TEST_BUILTINCACHEABLEWRAPPERS_H_
#define GEODE_INTEGRATION_TEST_BUILTINCACHEABLEWRAPPERS_H_

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

#include <limits.h>
#include <cstdlib>
#include <wchar.h>

#include <ace/Date_Time.h>

#include "CacheHelper.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheableWrapper.hpp"
#include "CacheImpl.hpp"

#include <geode/CacheableFileName.hpp>
#include <geode/CacheableUndefined.hpp>
#include <geode/CacheableObjectArray.hpp>

using namespace apache::geode::client;

namespace CacheableHelper {
const uint32_t m_crc32Table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

inline double random(double maxValue) {
  // Random number generator is initialized in registerBuiltins()
  return (maxValue * static_cast<double>(rand())) /
         (static_cast<double>(RAND_MAX) + 1.0);
}

template <typename TPRIM>
inline TPRIM random(TPRIM maxValue) {
  return (TPRIM)random((double)maxValue);
}

// This returns an array allocated on heap
// which should be freed by the user.
template <typename TPRIM>
inline std::vector<TPRIM> randomArray(int32_t size, TPRIM maxValue) {
  ASSERT(size > 0, "The size of the array should be greater than zero.");
  std::vector<TPRIM> array;
  array.reserve(size);

  for (int32_t index = 0; index < size; index++) {
    array.push_back(random(maxValue));
  }
  return array;
}

// Taken from GsRandom::getAlphanumericString
inline void randomString(int32_t size, std::string& randStr) {
  ASSERT(size > 0, "The size of the string should be greater than zero.");

  static const char chooseFrom[] =
      "0123456789 abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const int32_t chooseSize = sizeof(chooseFrom) - 1;

  randStr.resize(size);
  for (int32_t index = 0; index < size; index++) {
    randStr[index] = chooseFrom[random(chooseSize)];
  }
}

inline void randomString(int32_t size, std::wstring& randStr,
                         bool useASCII = false) {
  ASSERT(size > 0, "The size of the string should be greater than zero.");

  static const char chooseFrom[] =
      "0123456789 abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const int32_t chooseSize = sizeof(chooseFrom) - 1;

  randStr.resize(size);
  for (int32_t index = 0; index < size; index++) {
    if (useASCII) {
      randStr[index] = static_cast<wchar_t>(chooseFrom[random(chooseSize)]);
    } else {
      randStr[index] = static_cast<wchar_t>(
          static_cast<uint16_t>(random(UCHAR_MAX)) + 0x0901);
    }
  }
}

inline uint32_t crc32(const uint8_t* buffer, size_t bufLen) {
  if (buffer == nullptr || bufLen == 0) {
    return 0;
  }

  uint32_t crc = 0xffffffff;

  for (size_t index = 0; index < bufLen; index++) {
    crc =
        ((crc >> 8) & 0x00ffffff) ^ m_crc32Table[(crc ^ buffer[index]) & 0xff];
  }
  return ~crc;
}

inline uint32_t crc32(const int8_t* buffer, size_t bufLen) {
  return crc32(reinterpret_cast<const uint8_t*>(buffer), bufLen);
}

template <typename TPRIM>
inline uint32_t crc32(TPRIM value) {
  auto output = CacheHelper::getHelper().getCache()->createDataOutput();
  apache::geode::client::serializer::writeObject(output, value);
  return crc32(output.getBuffer(), output.getBufferLength());
}

template <typename TPRIM>
inline uint32_t crc32Array(const std::vector<TPRIM> arr) {
  auto output = CacheHelper::getHelper().getCache()->createDataOutput();
  for (auto obj : arr) {
    apache::geode::client::serializer::writeObject(output, obj);
  }
  return crc32(output.getBuffer(), output.getBufferLength());
}

inline bool isContainerTypeId(int8_t typeId) {
  return (typeId == GeodeTypeIds::CacheableObjectArray) ||
         (typeId == GeodeTypeIds::CacheableVector) ||
         (typeId == GeodeTypeIds::CacheableHashMap) ||
         (typeId == GeodeTypeIds::CacheableHashSet) ||
         (typeId == GeodeTypeIds::CacheableStack) ||
         (typeId == GeodeTypeIds::CacheableArrayList) ||
         (typeId == GeodeTypeIds::CacheableHashTable) ||
         (typeId == GeodeTypeIds::CacheableIdentityHashMap) ||
         (typeId == GeodeTypeIds::CacheableLinkedHashSet) ||
         (typeId == GeodeTypeIds::CacheableLinkedList);
}
}  // namespace CacheableHelper

// Cacheable types that can be used as keys

class CacheableBooleanWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  inline CacheableBooleanWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableBooleanWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return 2; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableBoolean::create(keyIndex % 2);
  }

  void initRandomValue(int32_t) override {
    m_cacheableObject = CacheableBoolean::create(
        CacheableHelper::random<uint8_t>(UCHAR_MAX) % 2);
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableBoolean* obj =
        dynamic_cast<const CacheableBoolean*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<uint8_t>(obj->value() ? 1 : 0);
  }
};

class CacheableByteWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  inline CacheableByteWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableByteWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return UCHAR_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableByte::create(static_cast<uint8_t>(keyIndex));
  }

  void initRandomValue(int32_t) override {
    m_cacheableObject =
        CacheableByte::create(CacheableHelper::random<uint8_t>(UCHAR_MAX));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableByte* obj = dynamic_cast<const CacheableByte*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<uint8_t>(obj->value());
  }
};

class CacheableDoubleWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableDoubleWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableDoubleWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return INT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableDouble::create(static_cast<double>(keyIndex));
  }

  void initRandomValue(int32_t maxSize) override {
    m_cacheableObject = CacheableDouble::create(
        CacheableHelper::random(static_cast<double>(maxSize)));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableDouble* obj =
        dynamic_cast<const CacheableDouble*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<double>(obj->value());
  }
};

class CacheableDateWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableDateWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableDateWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return INT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    // Seconds since epoch for June 26, 16:44
    time_t offset = 1182901465;
    m_cacheableObject = CacheableDate::create(offset + keyIndex);
  }

  void initRandomValue(int32_t) override {
    int32_t rnd = CacheableHelper::random<int32_t>(INT_MAX);
    time_t timeofday = 0;

    const ACE_Time_Value currentTime = ACE_OS::gettimeofday();
    timeofday = currentTime.sec();
    time_t epoctime = (time_t)(timeofday + (rnd * (rnd % 2 == 0 ? 1 : -1)));

    m_cacheableObject = CacheableDate::create(epoctime);
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableDate* obj = dynamic_cast<const CacheableDate*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<int64_t>(obj->milliseconds());
  }
};

class CacheableFileNameWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableFileNameWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableFileNameWrapper(); }

  // CacheableWrapper members

  virtual int32_t maxKeys() const { return INT_MAX; }

  virtual void initKey(int32_t keyIndex, int32_t maxSize) {
    maxSize %= (0xFFFF + 1);
    if (maxSize < 11) {
      maxSize = 11;
    }
    std::string baseStr(maxSize - 10, 'A');
    char indexStr[15];
    sprintf(indexStr, "%10d", keyIndex);
    baseStr.append(indexStr);
// make first caharacter as a '/' so java does not change the path
// taking it to be a relative path
#ifdef WIN32
    baseStr[0] = L'C';
    baseStr[1] = L':';
    baseStr[2] = L'\\';
#else
    baseStr[0] = L'/';
#endif
    m_cacheableObject = CacheableFileName::create(baseStr);
  }

  virtual void initRandomValue(int32_t maxSize) {
    maxSize %= (0xFFFF + 1);
    std::string randStr;
    CacheableHelper::randomString(maxSize, randStr);
    // make first caharacter as a '/' so java does not change the path
    // taking it to be a relative path
    randStr[0] = '/';
    m_cacheableObject = CacheableFileName::create(randStr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& obj = std::dynamic_pointer_cast<CacheableFileName>(object);
    return (obj ? CacheableHelper::crc32((uint8_t*)obj->value().c_str(),
                                         obj->length())
                : 0);
  }
};

class CacheableFloatWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableFloatWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableFloatWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return INT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableFloat::create(static_cast<float>(keyIndex));
  }

  void initRandomValue(int32_t maxSize) override {
    m_cacheableObject = CacheableFloat::create(
        CacheableHelper::random(static_cast<float>(maxSize)));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableFloat* obj =
        dynamic_cast<const CacheableFloat*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<float>(obj->value());
  }
};

class CacheableInt16Wrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt16Wrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt16Wrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return SHRT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableInt16::create(static_cast<int16_t>(keyIndex));
  }

  void initRandomValue(int32_t) override {
    m_cacheableObject =
        CacheableInt16::create(CacheableHelper::random<int16_t>(SHRT_MAX));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableInt16* obj =
        dynamic_cast<const CacheableInt16*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<int16_t>(obj->value());
  }
};

class CacheableInt32Wrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt32Wrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt32Wrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return INT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableInt32::create(keyIndex);
  }

  void initRandomValue(int32_t) override {
    m_cacheableObject =
        CacheableInt32::create(CacheableHelper::random<int32_t>(INT_MAX));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableInt32* obj =
        dynamic_cast<const CacheableInt32*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<int32_t>(obj->value());
  }
};

class CacheableInt64Wrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt64Wrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt64Wrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return INT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableInt64::create(static_cast<int64_t>(keyIndex));
  }

  void initRandomValue(int32_t) override {
    int64_t rnd = CacheableHelper::random<int64_t>(INT_MAX);
    rnd = (rnd << 32) + CacheableHelper::random<int64_t>(INT_MAX);
    m_cacheableObject = CacheableInt64::create(rnd);
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const CacheableInt64* obj =
        dynamic_cast<const CacheableInt64*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<int64_t>(obj->value());
  }
};

class CacheableStringWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableStringWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableStringWrapper(); }

  // CacheableWrapper members

  virtual int32_t maxKeys() const { return INT_MAX; }

  virtual void initKey(int32_t keyIndex, int32_t maxSize) {
    maxSize %= (0xFFFF + 1);
    if (maxSize < 11) {
      maxSize = 11;
    }
    std::string baseStr(maxSize - 10, 'A');
    char indexStr[15];
    sprintf(indexStr, "%10d", keyIndex);
    baseStr.append(indexStr);
    m_cacheableObject = CacheableString::create(baseStr);
  }

  virtual void initRandomValue(int32_t maxSize) {
    maxSize %= (0xFFFF + 1);
    std::string randStr;
    CacheableHelper::randomString(maxSize, randStr);
    m_cacheableObject = CacheableString::create(randStr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const CacheableString* obj =
        dynamic_cast<const CacheableString*>(object.get());
    return (obj != nullptr ? CacheableHelper::crc32(
                                 (uint8_t*)obj->value().c_str(), obj->length())
                           : 0);
  }
};

class CacheableHugeStringWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableHugeStringWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableHugeStringWrapper(); }

  // CacheableWrapper members

  virtual int32_t maxKeys() const { return INT_MAX; }

  virtual void initKey(int32_t keyIndex, int32_t maxSize) {
    if (maxSize <= 0xFFFF)  // ensure its larger than 64k
    {
      maxSize += (0xFFFF + 1);
    }
    std::string baseStr(maxSize - 10, 'A');
    char indexStr[15];
    sprintf(indexStr, "%10d", keyIndex);
    baseStr.append(indexStr);
    m_cacheableObject = CacheableString::create(baseStr);
  }

  virtual void initRandomValue(int32_t maxSize) {
    if (maxSize <= 0xFFFF)  // ensure its larger than 64k
    {
      maxSize += (0xFFFF + 1);
    }
    std::string randStr;
    CacheableHelper::randomString(maxSize, randStr);
    m_cacheableObject = CacheableString::create(randStr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const CacheableString* obj =
        dynamic_cast<const CacheableString*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32((uint8_t*)obj->value().c_str(),
                                  obj->length());
  }
};

class CacheableHugeUnicodeStringWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableHugeUnicodeStringWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableHugeUnicodeStringWrapper();
  }

  // CacheableWrapper members

  virtual int32_t maxKeys() const { return INT_MAX; }

  virtual void initKey(int32_t keyIndex, int32_t maxSize) {
    if (maxSize <= 0xFFFF)  // ensure its larger than 64k
    {
      maxSize += (0xFFFF + 1);
    }
    std::wstring baseStr(maxSize - 10, L'\x0905');
    wchar_t indexStr[15];
    swprintf(indexStr, 14, L"%10d", keyIndex);
    baseStr.append(indexStr);
    m_cacheableObject = CacheableString::create(baseStr);
  }

  virtual void initRandomValue(int32_t maxSize) {
    if (maxSize <= 0xFFFF)  // ensure its larger than 64k
    {
      maxSize += (0xFFFF + 1);
    }
    std::wstring randStr;
    CacheableHelper::randomString(maxSize, randStr);
    m_cacheableObject = CacheableString::create(randStr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& obj = std::dynamic_pointer_cast<CacheableString>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32(
        (uint8_t*)obj->value().c_str(),
        obj->length() * sizeof(std::string::value_type));
  }
};

class CacheableUnicodeStringWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableUnicodeStringWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableUnicodeStringWrapper();
  }

  // CacheableWrapper members

  virtual int32_t maxKeys() const { return INT_MAX; }

  virtual void initKey(int32_t keyIndex, int32_t maxSize) {
    maxSize %= 21800;  // so that encoded length is within 64k
    if (maxSize < 11) {
      maxSize = 11;
    }
    std::wstring baseStr(maxSize - 10, L'\x0905');
    wchar_t indexStr[15];
    swprintf(indexStr, 14, L"%10d", keyIndex);
    baseStr.append(indexStr);
    m_cacheableObject = CacheableString::create(baseStr);
  }

  virtual void initRandomValue(int32_t maxSize) {
    maxSize %= 21800;  // so that encoded length is within 64k
    std::wstring randStr;
    CacheableHelper::randomString(maxSize, randStr);
    m_cacheableObject = CacheableString::create(randStr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& obj = std::dynamic_pointer_cast<CacheableString>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32(
        (uint8_t*)obj->value().c_str(),
        obj->length() * sizeof(std::string::value_type));
  }
};

class CacheableWideCharWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableWideCharWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableWideCharWrapper(); }

  // CacheableWrapper members

  int32_t maxKeys() const override { return SHRT_MAX; }

  void initKey(int32_t keyIndex, int32_t) override {
    m_cacheableObject = CacheableCharacter::create(keyIndex);
  }

  void initRandomValue(int32_t) override {
    m_cacheableObject =
        CacheableCharacter::create(CacheableHelper::random<char16_t>(SHRT_MAX));
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    auto obj = std::dynamic_pointer_cast<const CacheableCharacter>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32<char16_t>(obj->value());
  }
};

// Other cacheable types that cannot be used as keys

template <typename HMAPTYPE>
class CacheableHashMapTypeWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableHashMapTypeWrapper<HMAPTYPE>() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableHashMapTypeWrapper<HMAPTYPE>();
  }

  // CacheableWrapper members

  void initRandomValue(int32_t maxSize) override {
    m_cacheableObject = HMAPTYPE::create(maxSize);
    HMAPTYPE* chmp = dynamic_cast<HMAPTYPE*>(m_cacheableObject.get());
    ASSERT(chmp != nullptr, "initRandomValue: null object.");
    auto keyTypeIds = CacheableWrapperFactory::getRegisteredKeyTypes();
    auto valTypeIds = CacheableWrapperFactory::getRegisteredValueTypes();

    for (std::vector<int8_t>::iterator keyIter = keyTypeIds.begin();
         keyIter != keyTypeIds.end(); keyIter++) {
      int item = 0;

      for (std::vector<int8_t>::iterator valIter = valTypeIds.begin();
           valIter != valTypeIds.end(); valIter++) {
        if (CacheableHelper::isContainerTypeId(*valIter)) {
          continue;
        }
        if ((*valIter == GeodeTypeIds::CacheableASCIIStringHuge ||
             *valIter == GeodeTypeIds::CacheableStringHuge) &&
            !(*keyIter == GeodeTypeIds::CacheableBoolean)) {
          continue;
        }
        if ((*keyIter == GeodeTypeIds::CacheableASCIIStringHuge ||
             *keyIter == GeodeTypeIds::CacheableStringHuge) &&
            !(*valIter == GeodeTypeIds::CacheableBoolean)) {
          continue;
        }
        // null object does not work on server side during deserialization
        if (*valIter == GeodeTypeIds::CacheableNullString) {
          continue;
        }

        const auto keyWrapper = std::unique_ptr<CacheableWrapper>(
            CacheableWrapperFactory::createInstance(*keyIter));
        ASSERT(keyWrapper != nullptr,
               "initRandomValue: keyWrapper null object.");
        if (item > keyWrapper->maxKeys()) {
        }
        const auto valWrapper = std::unique_ptr<CacheableWrapper>(
            CacheableWrapperFactory::createInstance(*valIter));
        ASSERT(valWrapper != nullptr,
               "initRandomValue: valWrapper null object.");
        keyWrapper->initKey(((*keyIter) << 8) + item, maxSize);
        valWrapper->initRandomValue(maxSize);
        chmp->emplace(
            std::dynamic_pointer_cast<CacheableKey>(keyWrapper->getCacheable()),
            valWrapper->getCacheable());
        item++;
      }
    }
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    const HMAPTYPE* obj = dynamic_cast<const HMAPTYPE*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    uint32_t chksum = 0;

    for (const auto& iter : *obj) {
      const auto cwpKey = std::unique_ptr<CacheableWrapper>(
          CacheableWrapperFactory::createInstance(iter.first->typeId()));
      const auto& cwpObj = iter.second;
      uint32_t cwpObjCkSum = 0;
      if (cwpObj != nullptr) {
        const auto cwpVal = std::unique_ptr<CacheableWrapper>(
            CacheableWrapperFactory::createInstance(cwpObj->typeId()));
        cwpObjCkSum = cwpVal->getCheckSum(cwpObj);
      }
      chksum ^= (cwpKey->getCheckSum(iter.first) ^ cwpObjCkSum);
    }

    return chksum;
  }
};

typedef CacheableHashMapTypeWrapper<CacheableHashMap> CacheableHashMapWrapper;

typedef CacheableHashMapTypeWrapper<CacheableHashTable>
    CacheableHashTableWrapper;

typedef CacheableHashMapTypeWrapper<CacheableIdentityHashMap>
    CacheableIdentityHashMapWrapper;

template <typename HSETTYPE>
class CacheableHashSetTypeWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableHashSetTypeWrapper<HSETTYPE>() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableHashSetTypeWrapper<HSETTYPE>();
  }

  // CacheableWrapper members

  void initRandomValue(int32_t maxSize) override {
    auto set =
        std::static_pointer_cast<HSETTYPE>(HSETTYPE::createDeserializable());
    auto keyTypeIds = CacheableWrapperFactory::getRegisteredKeyTypes();
    size_t sizeOfTheVector = keyTypeIds.size();
    maxSize = maxSize / static_cast<int32_t>(sizeOfTheVector) + 1;
    for (size_t i = 0; i < sizeOfTheVector; i++) {
      int8_t keyTypeId = keyTypeIds[i];
      auto wrapper = CacheableWrapperFactory::createInstance(keyTypeId);
      wrapper->initRandomValue(maxSize);
      auto cptr = wrapper->getCacheable();
      set->insert(
          std::dynamic_pointer_cast<CacheableKey>(wrapper->getCacheable()));
      delete wrapper;
    }
    m_cacheableObject = set;
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    auto&& obj = std::dynamic_pointer_cast<HSETTYPE>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    uint32_t checkSum = 0;

    for (const auto& iter : *obj) {
      const auto typeId = iter->typeId();
      const auto wrapper = std::unique_ptr<CacheableWrapper>(
          CacheableWrapperFactory::createInstance(typeId));
      checkSum ^= wrapper->getCheckSum(iter);
    }
    return checkSum;
  }
};

typedef CacheableHashSetTypeWrapper<CacheableHashSet> CacheableHashSetWrapper;

typedef CacheableHashSetTypeWrapper<CacheableLinkedHashSet>
    CacheableLinkedHashSetWrapper;

class CacheableBytesWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableBytesWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableBytesWrapper(); }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    auto randArr =
        CacheableHelper::randomArray<uint8_t>(maxSize, UCHAR_MAX);
    m_cacheableObject = CacheableBytes::create(
        std::vector<int8_t>(std::begin(randArr), std::end(randArr)));
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const CacheableBytes* obj =
        dynamic_cast<const CacheableBytes*>(object.get());
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32(obj->value().data(), obj->length());
  }
};

class CacheableDoubleArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableDoubleArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableDoubleArrayWrapper();
  }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    maxSize = maxSize / sizeof(double) + 1;
    auto randArr =
        CacheableHelper::randomArray(maxSize, static_cast<double>(INT_MAX));
    m_cacheableObject = CacheableDoubleArray::create(randArr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const auto obj =
        std::dynamic_pointer_cast<const CacheableDoubleArray>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32Array(obj->value());
  }
};

class CacheableFloatArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableFloatArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableFloatArrayWrapper(); }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    maxSize = maxSize / sizeof(float) + 1;
    auto randArr =
        CacheableHelper::randomArray(maxSize, static_cast<float>(INT_MAX));
    m_cacheableObject = CacheableFloatArray::create(randArr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const auto obj =
        std::dynamic_pointer_cast<const CacheableFloatArray>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32Array(obj->value());
  }
};

class CacheableInt16ArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt16ArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt16ArrayWrapper(); }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    maxSize = maxSize / sizeof(int16_t) + 1;
    auto randArr = CacheableHelper::randomArray<int16_t>(maxSize, SHRT_MAX);
    m_cacheableObject = CacheableInt16Array::create(randArr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const auto obj =
        std::dynamic_pointer_cast<const CacheableInt16Array>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32Array(obj->value());
  }
};

class CacheableInt32ArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt32ArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt32ArrayWrapper(); }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    maxSize = maxSize / sizeof(int32_t) + 1;
    auto randArr = CacheableHelper::randomArray<int32_t>(maxSize, INT_MAX);
    m_cacheableObject = CacheableInt32Array::create(randArr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const auto obj =
        std::dynamic_pointer_cast<const CacheableInt32Array>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32Array(obj->value());
  }
};

class CacheableInt64ArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableInt64ArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableInt64ArrayWrapper(); }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    maxSize = maxSize / sizeof(int64_t) + 1;
    auto randArr = CacheableHelper::randomArray<int64_t>(maxSize, INT_MAX);
    m_cacheableObject = CacheableInt64Array::create(randArr);
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    const auto obj =
        std::dynamic_pointer_cast<const CacheableInt64Array>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    return CacheableHelper::crc32Array(obj->value());
  }
};

class CacheableNullStringWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableNullStringWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableNullStringWrapper(); }

  // CacheableWrapper members

  void initRandomValue(int32_t) override {
    m_cacheableObject = CacheableString::create((char*)nullptr);
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const override {
    ASSERT(object == nullptr, "getCheckSum: expected null object");
    return 0;
  }
};

class CacheableStringArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableStringArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableStringArrayWrapper();
  }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    int32_t arraySize = 16;
    maxSize = maxSize / arraySize;
    if (maxSize < 2) {
      maxSize = 2;
    }

    std::shared_ptr<CacheableString>* randArr =
        new std::shared_ptr<CacheableString>[arraySize];
    for (int32_t arrayIndex = 0; arrayIndex < arraySize; arrayIndex++) {
      if (arrayIndex % 2 == 0) {
        std::string randStr;
        CacheableHelper::randomString(maxSize, randStr);
        randArr[arrayIndex] = CacheableString::create(randStr);
      } else {
        std::wstring randStr;
        CacheableHelper::randomString(maxSize, randStr);
        randArr[arrayIndex] = CacheableString::create(randStr);
      }
    }
    m_cacheableObject = CacheableStringArray::create(
      std::vector<std::shared_ptr<CacheableString>>(
        randArr, randArr + arraySize));
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& obj = std::dynamic_pointer_cast<CacheableStringArray>(object);
    ASSERT(obj != nullptr, "getCheckSum: null object.");
    uint32_t checkSum = 0;
    std::shared_ptr<CacheableString> str;
    for (int32_t index = 0; index < obj->length(); index++) {
      str = obj->operator[](index);
      checkSum ^= CacheableHelper::crc32(
          (uint8_t*)str->value().c_str(),
          str->length() * sizeof(std::string::value_type));
    }
    return checkSum;
  }
};

class CacheableUndefinedWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableUndefinedWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() { return new CacheableUndefinedWrapper(); }

  // CacheableWrapper members

  void initRandomValue(int32_t) override {
    m_cacheableObject = std::shared_ptr<Serializable>(
        CacheableUndefined::createDeserializable());
  }

  uint32_t getCheckSum(const std::shared_ptr<Cacheable>) const override {
    return 0;
  }
};

template <typename VECTTYPE>
class CacheableVectorTypeWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableVectorTypeWrapper<VECTTYPE>() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableVectorTypeWrapper<VECTTYPE>();
  }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    auto vec =
        std::static_pointer_cast<VECTTYPE>(VECTTYPE::createDeserializable());
    auto valueTypeIds = CacheableWrapperFactory::getRegisteredValueTypes();
    size_t sizeOfTheVector = valueTypeIds.size();
    maxSize = maxSize / static_cast<int32_t>(sizeOfTheVector) + 1;
    for (size_t i = 0; i < sizeOfTheVector; i++) {
      int8_t valueTypeId = valueTypeIds[i];
      if (!CacheableHelper::isContainerTypeId(valueTypeId)) {
        auto wrapper = CacheableWrapperFactory::createInstance(valueTypeId);
        wrapper->initRandomValue(maxSize);
        vec->push_back(wrapper->getCacheable());
        delete wrapper;
      }
    }
    m_cacheableObject = vec;
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& vec = std::dynamic_pointer_cast<VECTTYPE>(object);
    ASSERT(vec != nullptr, "getCheckSum: null object.");
    uint32_t checkSum = 0;
    for (uint32_t index = 0; index < (uint32_t)vec->size(); ++index) {
      auto obj = vec->at(index);
      if (obj == nullptr) {
        continue;
      }
      int8_t typeId = obj->typeId();
      auto wrapper = CacheableWrapperFactory::createInstance(typeId);
      checkSum ^= wrapper->getCheckSum(obj);
      delete wrapper;
    }
    return checkSum;
  }
};

typedef CacheableVectorTypeWrapper<CacheableVector> CacheableVectorWrapper;

typedef CacheableVectorTypeWrapper<CacheableArrayList>
    CacheableArrayListWrapper;

typedef CacheableVectorTypeWrapper<CacheableLinkedList>
    CacheableLinkedListWrapper;

typedef CacheableVectorTypeWrapper<CacheableStack> CacheableStackWrapper;

class CacheableObjectArrayWrapper : public CacheableWrapper {
 public:
  // Constructor and factory function

  CacheableObjectArrayWrapper() : CacheableWrapper(nullptr) {}

  static CacheableWrapper* create() {
    return new CacheableObjectArrayWrapper();
  }

  // CacheableWrapper members

  virtual void initRandomValue(int32_t maxSize) {
    auto arr = std::static_pointer_cast<CacheableObjectArray>(
        CacheableObjectArray::createDeserializable());
    auto valueTypeIds = CacheableWrapperFactory::getRegisteredValueTypes();
    size_t sizeOfTheVector = valueTypeIds.size();
    maxSize = maxSize / static_cast<int32_t>(sizeOfTheVector) + 1;
    for (size_t i = 0; i < sizeOfTheVector; i++) {
      int8_t valueTypeId = valueTypeIds[i];
      if (!CacheableHelper::isContainerTypeId(valueTypeId)) {
        auto wrapper = CacheableWrapperFactory::createInstance(valueTypeId);
        wrapper->initRandomValue(maxSize);
        arr->push_back(wrapper->getCacheable());
        delete wrapper;
      }
    }
    m_cacheableObject = arr;
  }

  virtual uint32_t getCheckSum(const std::shared_ptr<Cacheable> object) const {
    auto&& arr = std::dynamic_pointer_cast<CacheableObjectArray>(object);
    ASSERT(arr != nullptr, "getCheckSum: null object.");
    uint32_t checkSum = 0;
    for (uint32_t index = 0; index < static_cast<uint32_t>(arr->size());
         ++index) {
      const auto obj = arr->at(index);
      if (obj == nullptr) {
        continue;
      }
      int8_t typeId = obj->typeId();
      auto wrapper = CacheableWrapperFactory::createInstance(typeId);
      checkSum ^= wrapper->getCheckSum(obj);
      delete wrapper;
    }
    return checkSum;
  }
};

namespace CacheableHelper {

void registerBuiltins(bool isRegisterFileName = false) {
  // Initialize the random number generator.
  srand(getpid() + static_cast<int>(time(0)));

  // Register the builtin cacheable keys
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableBoolean,
                                        "CacheableBoolean",
                                        CacheableBooleanWrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableByte,
                                        "CacheableByte",
                                        CacheableByteWrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableDouble,
                                        "CacheableDouble",
                                        CacheableDoubleWrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableDate,
                                        "CacheableDate",
                                        CacheableDateWrapper::create, true);
  if (isRegisterFileName) {
#ifdef _WIN32
    // TODO: windows requires some serious tweaking to get this to work
    CacheableWrapperFactory::registerType(
        GeodeTypeIds::CacheableFileName, "CacheableFileName",
        CacheableFileNameWrapper::create, true);
#endif
  }
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableFloat,
                                        "CacheableFloat",
                                        CacheableFloatWrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableInt16,
                                        "CacheableInt16",
                                        CacheableInt16Wrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableInt32,
                                        "CacheableInt32",
                                        CacheableInt32Wrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableInt64,
                                        "CacheableInt64",
                                        CacheableInt64Wrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableASCIIString,
                                        "CacheableString",
                                        CacheableStringWrapper::create, true);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableString, "CacheableUnicodeString",
      CacheableUnicodeStringWrapper::create, true);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableASCIIStringHuge, "CacheableHugeString",
      CacheableHugeStringWrapper::create, true);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableStringHuge, "CacheableHugeUnicodeString",
      CacheableHugeUnicodeStringWrapper::create, true);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableCharacter,
                                        "CacheableCharacter",
                                        CacheableWideCharWrapper::create, true);

  // Register other builtin cacheables
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableHashMap,
                                        "CacheableHashMap",
                                        CacheableHashMapWrapper::create, false);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableHashSet,
                                        "CacheableHashSet",
                                        CacheableHashSetWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableHashTable, "CacheableHashTable",
      CacheableHashTableWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableIdentityHashMap, "CacheableIdentityHashMap",
      CacheableIdentityHashMapWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableLinkedHashSet, "CacheableLinkedHashSet",
      CacheableLinkedHashSetWrapper::create, false);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableBytes,
                                        "CacheableBytes",
                                        CacheableBytesWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableDoubleArray, "CacheableDoubleArray",
      CacheableDoubleArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableFloatArray, "CacheableFloatArray",
      CacheableFloatArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableInt16Array, "CacheableInt16Array",
      CacheableInt16ArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableInt32Array, "CacheableInt32Array",
      CacheableInt32ArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableInt64Array, "CacheableInt64Array",
      CacheableInt64ArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableNullString, "CacheableNullString",
      CacheableNullStringWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableObjectArray, "CacheableObjectArray",
      CacheableObjectArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableStringArray, "CacheableStringArray",
      CacheableStringArrayWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableUndefined, "CacheableUndefined",
      CacheableUndefinedWrapper::create, false);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableVector,
                                        "CacheableVector",
                                        CacheableVectorWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableArrayList, "CacheableArrayList",
      CacheableArrayListWrapper::create, false);
  CacheableWrapperFactory::registerType(
      GeodeTypeIds::CacheableLinkedList, "CacheableLinkedList",
      CacheableLinkedListWrapper::create, false);
  CacheableWrapperFactory::registerType(GeodeTypeIds::CacheableStack,
                                        "CacheableStack",
                                        CacheableStackWrapper::create, false);
}
}  // namespace CacheableHelper

#endif  // GEODE_INTEGRATION_TEST_BUILTINCACHEABLEWRAPPERS_H_
