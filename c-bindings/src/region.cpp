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

// Standard headers
#include <memory>
#include <string>

#ifdef _WIN32
#include <objbase.h>
#endif  // _WIN32

// C++ client public headers
#include "geode/CacheableBuiltins.hpp"
#include "geode/CacheableString.hpp"
#include "geode/Region.hpp"
#include "geode/RegionShortcut.hpp"

// C client public headers
#include "geode/region.h"
#include "geode/region/factory.h"

// C client private headers
#include "data_serializable_raw.hpp"
#include "region.hpp"
#include "region/factory.hpp"

using namespace apache::geode::client;
using namespace apache::geode::client::internal;

RegionWrapper::RegionWrapper(
    std::shared_ptr<apache::geode::client::Region> region)
    : region_(region) {
  AddRecord(this, "RegionWrapper");

  // region_->getCache().createDataInput(buffer, len);
}

RegionWrapper::~RegionWrapper() { RemoveRecord(this); }

void RegionWrapper::PutString(const std::string& key,
                              const std::string& value) {
  region_->put(key, value);
}

void RegionWrapper::PutByteArray(const std::string& key, const char* value,
                                 size_t size) {
  region_->put(key,
               apache::geode::client::internal::DataSerializableRaw::create(
                   (const int8_t*)value, size));
}

// void RegionWrapper::PutByteArray(const char* key, size_t keySize,
//                                 const char* value, size_t valueSize) {
//  region_->put(apache::geode::client::internal::DataSerializableRaw::create(
//                   (const int8_t*)key, keySize),
//               apache::geode::client::internal::DataSerializableRaw::create(
//                   (const int8_t*)value, valueSize));
//}

void RegionWrapper::PutByteArray(const char* key, size_t keySize,
                                 const char* value, size_t valueSize) {
  // Get keyCode
  auto keyCode = *(DSCode*)(key);
  // auto primitive = std::dynamic_pointer_cast<DataSerializablePrimitive>(val);
  // std_sharedptr<CacheableKey> cacheableKey =
  //    GetUnmanagedValueGeneric(key, keyCode);)
  // auto keyValue = 0;
  switch (keyCode) {
    case DSCode::CacheableInt16: {
      int16_t keyValue16 = (int16_t)(*((int16_t*)(key + sizeof(DSCode))));
      region_->put(keyValue16, DataSerializableRaw::create((const int8_t*)value,
                                                           valueSize));
    } break;
    case DSCode::CacheableInt32: {
      int32_t keyValue32 = (int32_t)(*((int32_t*)(key + sizeof(DSCode))));
      region_->put(keyValue32, DataSerializableRaw::create((const int8_t*)value,
                                                           valueSize));
    } break;
  }
}

template <typename TKey, typename TVal>
void RegionWrapper::Put(TKey key, TVal val) {
  region_->put(key, val);
}

void RegionWrapper::PutByteArray(const int32_t key, const char* value,
                                 size_t size) {
  std::vector<int8_t> val(value, value + size);
  region_->put(key, apache::geode::client::CacheableBytes::create(val));
}

const char* RegionWrapper::GetString(const std::string& key) {
  auto value = region_->get(key);
  lastValue_ =
      std::dynamic_pointer_cast<apache::geode::client::CacheableString>(value)
          ->value();

  return lastValue_.c_str();
}

void RegionWrapper::GetByteArray(const std::string& key, char** value,
                                 size_t* size) {
  auto val = region_->get(key);
  auto primitive = std::dynamic_pointer_cast<
      apache::geode::client::internal::DataSerializablePrimitive>(val);

  if (val.get() == nullptr) return;

  std::shared_ptr<std::vector<int8_t>> bytes =
      std::make_shared<std::vector<int8_t>>();
  apache::geode::client::internal::DSCode dsCode = primitive->getDsCode();
  bytes->push_back((int8_t)((int)dsCode));
  bytes->push_back((int8_t)((int)dsCode >> 8));
  bytes->push_back((int8_t)((int)dsCode >> 16));
  bytes->push_back((int8_t)((int)dsCode >> 24));

  int32_t int32 = 0;
  int16_t int16 = 0;
  std::string str = "";

  switch (dsCode) {
    case apache::geode::client::internal::DSCode::CacheableASCIIString:
      str = std::dynamic_pointer_cast<apache::geode::client::CacheableString>(
                primitive)
                ->value();
      *size = str.length() + 4;
      std::copy(str.begin(), str.end(), std::back_inserter(*bytes));
      break;
    case apache::geode::client::internal::DSCode::CacheableInt32:
      int32 = std::dynamic_pointer_cast<apache::geode::client::CacheableInt32>(
                  primitive)
                  ->value();
      bytes->push_back((int8_t)(int32 >> 24));
      bytes->push_back((int8_t)(int32 >> 16));
      bytes->push_back((int8_t)(int32 >> 8));
      bytes->push_back((int8_t)(int32));
      *size = 8;
      break;
    case apache::geode::client::internal::DSCode::CacheableInt16:
      int16 = std::dynamic_pointer_cast<apache::geode::client::CacheableInt16>(
                  primitive)
                  ->value();
      bytes->push_back((int8_t)(int16 >> 8));
      bytes->push_back((int8_t)(int16));
      *size = 6;
      break;
  }

    // auto bytes =
    //    std::shared_ptr<apache::geode::client::internal::DataSerializableRaw>(
    //        val);
    // apache::geode::client::internal::DataSerializableRaw::create(
    //    (const int8_t*)val, size);

#if defined(_WIN32)
  int8_t* byteArray = static_cast<int8_t*>(CoTaskMemAlloc(*size));
#else
  int8_t* byteArray = static_cast<int8_t*>(malloc(*size));
#endif
  if (bytes) {
    memcpy(byteArray, bytes->data(), *size);
    *value = reinterpret_cast<char*>(byteArray);
  }
}

void RegionWrapper::GetByteArray(const char* key, size_t keyLength,
                                 char** value, size_t* valueLength) {
  auto keyCode = *(DSCode*)(key);

  auto val = region_->get(key);
  auto primitive = std::dynamic_pointer_cast<
      apache::geode::client::internal::DataSerializablePrimitive>(val);

  if (val.get() == nullptr) return;

  std::shared_ptr<std::vector<int8_t>> bytes =
      std::make_shared<std::vector<int8_t>>();
  apache::geode::client::internal::DSCode dsCode = primitive->getDsCode();
  bytes->push_back((int8_t)((int)dsCode));
  bytes->push_back((int8_t)((int)dsCode >> 8));
  bytes->push_back((int8_t)((int)dsCode >> 16));
  bytes->push_back((int8_t)((int)dsCode >> 24));

  int32_t int32 = 0;
  int16_t int16 = 0;
  std::string str = "";

  switch (keyCode) {
    case apache::geode::client::internal::DSCode::CacheableASCIIString:
      str = std::dynamic_pointer_cast<apache::geode::client::CacheableString>(
                primitive)
                ->value();
      *valueLength = str.length() + 4;
      std::copy(str.begin(), str.end(), std::back_inserter(*bytes));
      break;
    case apache::geode::client::internal::DSCode::CacheableInt32:
      int32 = std::dynamic_pointer_cast<apache::geode::client::CacheableInt32>(
                  primitive)
                  ->value();
      bytes->push_back((int8_t)(int32 >> 24));
      bytes->push_back((int8_t)(int32 >> 16));
      bytes->push_back((int8_t)(int32 >> 8));
      bytes->push_back((int8_t)(int32));
      *valueLength = 8;
      break;
    case apache::geode::client::internal::DSCode::CacheableInt16:
      int16 = std::dynamic_pointer_cast<apache::geode::client::CacheableInt16>(
                  primitive)
                  ->value();
      bytes->push_back((int8_t)(int16 >> 8));
      bytes->push_back((int8_t)(int16));
      *valueLength = 6;
      break;
  }

    // auto bytes =
    //    std::shared_ptr<apache::geode::client::internal::DataSerializableRaw>(
    //        val);
    // apache::geode::client::internal::DataSerializableRaw::create(
    //    (const int8_t*)val, size);

#if defined(_WIN32)
  int8_t* byteArray = static_cast<int8_t*>(CoTaskMemAlloc(*valueLength));
#else
  int8_t* byteArray = static_cast<int8_t*>(malloc(*valueLength));
#endif
  if (bytes) {
    memcpy(byteArray, bytes->data(), *valueLength);
    *value = reinterpret_cast<char*>(byteArray);
  }
}

// auto val = region_->get(key);
// auto primitive = std::dynamic_pointer_cast<
//    apache::geode::client::internal::DataSerializablePrimitive>(val);

// if (val.get() == nullptr) return;

// std::shared_ptr<std::vector<int8_t>> bytes =
//    std::make_shared<std::vector<int8_t>>();
// apache::geode::client::internal::DSCode dsCode = primitive->getDsCode();
// bytes->push_back((int8_t)primitive->getDsCode());

// int32_t int32 = 0;
// int16_t int16 = 0;
// std::string str = "";

// std::shared_ptr<apache::geode::client::CacheableKey>
// GetUnmanagedValueGeneric(
//    *key, dsCode);

//  switch (dsCode) {
//    case apache::geode::client::internal::DSCode::CacheableASCIIString:
//      str =
//      std::dynamic_pointer_cast<apache::geode::client::CacheableString>(
//                primitive)
//                ->value();
//      *size = str.length() + 1;
//      std::copy(str.begin(), str.end(), std::back_inserter(*bytes));
//      break;
//    case apache::geode::client::internal::DSCode::CacheableInt32:
//      auto val = region_->get((int32_t)(*key));
//      int32 =
//      std::dynamic_pointer_cast<apache::geode::client::CacheableInt32>(
//                  primitive)
//                  ->value();
//      bytes->push_back((int8_t)(int32 >> 24));
//      bytes->push_back((int8_t)(int32 >> 16));
//      bytes->push_back((int8_t)(int32 >> 8));
//      bytes->push_back((int8_t)(int32));
//      *size = 5;
//      break;
//    case apache::geode::client::internal::DSCode::CacheableInt16:
//      int16 =
//      std::dynamic_pointer_cast<apache::geode::client::CacheableInt16>(
//                  primitive)
//                  ->value();
//      bytes->push_back((int8_t)(int16 >> 8));
//      bytes->push_back((int8_t)(int16));
//      *size = 3;
//      break;
//  }
//
//    // auto bytes =
//    //
//    std::shared_ptr<apache::geode::client::internal::DataSerializableRaw>(
//    //        val);
//    // apache::geode::client::internal::DataSerializableRaw::create(
//    //    (const int8_t*)val, size);
//
//#if defined(_WIN32)
//  int8_t* byteArray = static_cast<int8_t*>(CoTaskMemAlloc(*size));
//#else
//  int8_t* byteArray = static_cast<int8_t*>(malloc(*size));
//#endif
//  if (bytes) {
//    memcpy(byteArray, bytes->data(), *size);
//    *value = reinterpret_cast<char*>(byteArray);
//  }
//}

void RegionWrapper::GetByteArray(const int32_t key, char** value,
                                 size_t* size) {
  std::shared_ptr<apache::geode::client::Serializable> val = region_->get(key);

  if (val.get() == nullptr) return;

  auto bytes = std::dynamic_pointer_cast<
      apache::geode::client::internal::DataSerializablePrimitive>(val);
  int valSize = val->objectSize();
#if defined(_WIN32)
  int8_t* byteArray = static_cast<int8_t*>(CoTaskMemAlloc(valSize));
#else
  int8_t* byteArray = static_cast<int8_t*>(malloc(valSize));
#endif
  // if (bytes) {
  //  memcpy(byteArray, bytes->value().data(), valSize);
  //  *value = reinterpret_cast<char*>(byteArray);
  //  *size = valSize;
  //}
}

void RegionWrapper::Remove(const std::string& key) { region_->remove(key); }

bool RegionWrapper::ContainsValueForKey(const std::string& key) {
  return region_->containsValueForKey(key);
}

void apache_geode_DestroyRegion(apache_geode_region_t* region) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  delete regionWrapper;
}

void apache_geode_Region_PutString(apache_geode_region_t* region,
                                   const char* key, const char* value) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  regionWrapper->PutString(key, value);
}

void apache_geode_Region_PutByteArray(apache_geode_region_t* region,
                                      const char* key, const char* value,
                                      size_t size) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  regionWrapper->PutByteArray(key, value, size);
}

void apache_geode_Region_Put(apache_geode_region_t* region, const char* key,
                             size_t keyLength, const char* val,
                             size_t valLength) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  regionWrapper->PutByteArray(key, keyLength, val, valLength);
}

void apache_geode_Region_PutByteArrayForInt32Key(apache_geode_region_t* region,
                                                 const int32_t key,
                                                 const char* value,
                                                 size_t size) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  regionWrapper->PutByteArray(key, value, size);
}

const char* apache_geode_Region_GetString(apache_geode_region_t* region,
                                          const char* key) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->GetString(key);
}

void apache_geode_Region_GetByteArray(apache_geode_region_t* region,
                                      const char* key, char** value,
                                      size_t* size) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->GetByteArray(key, value, size);
}

void apache_geode_Region_Get(apache_geode_region_t* region, const char* key,
                             size_t keyLength, char** value,
                             size_t* valueLength) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  // GetUnmanagedValueGeneric(key, dsCode);
  return regionWrapper->GetByteArray(key, keyLength, value, valueLength);
}

void apache_geode_Region_GetByteArrayForInt32Key(apache_geode_region_t* region,
                                                 const int32_t key,
                                                 char** value, size_t* size) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->GetByteArray(key, value, size);
}

void apache_geode_Region_Remove(apache_geode_region_t* region,
                                const char* key) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->Remove(key);
}

bool apache_geode_Region_ContainsValueForKey(apache_geode_region_t* region,
                                             const char* key) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->ContainsValueForKey(key);
}

template <class TKey>
std::shared_ptr<apache::geode::client::CacheableKey> GetUnmanagedValueGeneric(
    TKey key, DSCode dsCode) {
  switch (dsCode) {
    // case native::internal::DSCode::CacheableByte: {
    //  return Serializable::getCacheableByte((Byte)key);
    //}
    // case native::internal::DSCode::CacheableBoolean:
    //  return Serializable::getCacheableBoolean((bool)key);
    // case native::internal::DSCode::CacheableCharacter:
    //  return Serializable::getCacheableWideChar((Char)key);
    // case native::internal::DSCode::CacheableDouble:
    //  return Serializable::getCacheableDouble((double)key);
    // case native::internal::DSCode::CacheableASCIIString:
    //  return Serializable::GetCacheableString((String ^) key);
    // case native::internal::DSCode::CacheableFloat:
    //  return Serializable::getCacheableFloat((float)key);
    case DSCode::CacheableInt16: {
      // return (int16_t)key);
      return GetNativeCacheableKeyWrapperForManagedISerializable(
          Apache::Geode::Client::CacheableInt16Array::Create((array<Int16> ^)
                                                                 key));
    }
    case DSCode::CacheableInt32: {
      return (int32_t)key;
    }
    // case native::internal::DSCode::CacheableInt64: {
    //  return Serializable::getCacheableInt64((System::Int64)key);
    //}
    // case native::internal::DSCode::CacheableBytes: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableBytes::Create((array<Byte> ^) key));
    //}
    // case native::internal::DSCode::CacheableDoubleArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableDoubleArray::Create((array<Double>
    //      ^)
    //                                                              key));
    //}
    // case native::internal::DSCode::CacheableFloatArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableFloatArray::Create((array<float> ^)
    //                                                             key));
    //}
    // case native::internal::DSCode::CacheableInt16Array: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableInt16Array::Create((array<Int16> ^)
    //                                                             key));
    //}
    // case native::internal::DSCode::CacheableInt32Array: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableInt32Array::Create((array<Int32> ^)
    //                                                             key));
    //}
    // case native::internal::DSCode::CacheableInt64Array: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableInt64Array::Create((array<Int64> ^)
    //                                                             key));
    //}
    // case native::internal::DSCode::CacheableStringArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableStringArray::Create(
    //          (array<String ^> ^) key));
    //}
    // case native::internal::DSCode::CacheableFileName: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableFileName ^) key);
    //}
    // case native::internal::DSCode::CacheableHashTable:  //
    // collection::hashtable
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableHashTable::Create(
    //          (System::Collections::Hashtable ^) key));
    //}
    // case native::internal::DSCode::CacheableHashMap:  // generic dictionary
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableHashMap::Create(
    //          (System::Collections::IDictionary ^) key));
    //}
    // case native::internal::DSCode::CacheableVector:  // collection::arraylist
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      CacheableVector::Create((System::Collections::IList ^) key));
    //}
    // case native::internal::DSCode::CacheableArrayList:  // generic ilist
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableArrayList::Create(
    //          (System::Collections::IList ^) key));
    //}
    // case native::internal::DSCode::CacheableLinkedList:  // generic linked
    // list
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableLinkedList::Create(
    //          (System::Collections::Generic::LinkedList<Object ^> ^) key));
    //}
    // case native::internal::DSCode::CacheableStack: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableStack::Create(
    //          (System::Collections::ICollection ^) key));
    //}
    // case native::internal::InternalId::CacheableManagedObject: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableObject ^) key);
    //}
    // case native::internal::InternalId::CacheableManagedObjectXml: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableObjectXml ^) key);
    //}
    // case native::internal::DSCode::CacheableObjectArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableObjectArray ^) key);
    //}
    // case native::internal::DSCode::CacheableIdentityHashMap: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableIdentityHashMap::Create(
    //          (System::Collections::IDictionary ^) key));
    //}
    // case native::internal::DSCode::CacheableHashSet:  // no need of it,
    // default
    //                                                  // case should work
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableHashSet ^) key);
    //}
    // case native::internal::DSCode::
    //    CacheableLinkedHashSet:  // no need of it, default case should work
    //{
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      (Apache::Geode::Client::CacheableLinkedHashSet ^) key);
    //}
    // case native::internal::DSCode::CacheableDate: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CacheableDate::Create((System::DateTime)key));
    //}
    // case native::internal::DSCode::BooleanArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::BooleanArray::Create((array<bool> ^) key));
    //}
    // case native::internal::DSCode::CharArray: {
    //  return GetNativeCacheableKeyWrapperForManagedISerializable(
    //      Apache::Geode::Client::CharArray::Create((array<Char> ^) key));
    //}
    default: {
      std::shared_ptr<native::Cacheable> kPtr(
          GetNativeWrapperForManagedObject(key));
      return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
    }
  }
}  //
