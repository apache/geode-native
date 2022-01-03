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
#include "geode/CacheableString.hpp"
#include "geode/CacheableBuiltins.hpp"
#include "geode/Region.hpp"
#include "geode/RegionShortcut.hpp"

// C client public headers
#include "geode/region.h"
#include "geode/region/factory.h"

// C client private headers
#include "data_serializable_raw.hpp"
#include "region.hpp"
#include "region/factory.hpp"

RegionWrapper::RegionWrapper(
    std::shared_ptr<apache::geode::client::Region> region)
    : region_(region) {
  AddRecord(this, "RegionWrapper");
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
  auto primitive =
      std::dynamic_pointer_cast<apache::geode::client::internal::DataSerializablePrimitive>(val);

  if (val.get() == nullptr) return;

  std::shared_ptr<std::vector<int8_t>> bytes = std::make_shared<std::vector<int8_t>>();
  apache::geode::client::internal::DSCode dsCode = primitive->getDsCode();
  bytes->push_back((byte)primitive->getDsCode());

  int32_t int32 = 0;
  int16_t int16 = 0;

  switch (dsCode) {
    case apache::geode::client::internal::DSCode::CacheableInt32:
      int32 =
          std::dynamic_pointer_cast<apache::geode::client::CacheableInt32>
          (primitive)->value();
      bytes->push_back((int8_t)(int32 >> 24));
      bytes->push_back((int8_t)(int32 >> 16));
      bytes->push_back((int8_t)(int32 >> 8));
      bytes->push_back((int8_t)(int32));
      *size = 5;
      break;
    case apache::geode::client::internal::DSCode::CacheableInt16:
      int16 =
          std::dynamic_pointer_cast<apache::geode::client::CacheableInt16>(
              primitive)
              ->value();
      bytes->push_back((int8_t)(int16 >> 8));
      bytes->push_back((int8_t)(int16));
      *size = 3;
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
  int8_t* byteArray = static_cast<int8_t*>(malloc(size));
#endif
  if (bytes) {
    memcpy(byteArray, bytes->data(), *size);
    *value = reinterpret_cast<char*>(byteArray);
  }
}

void RegionWrapper::GetByteArray(const int32_t key, char** value,
                                 size_t* size) {
  std::shared_ptr<apache::geode::client::Serializable> val = region_->get(key);

  if (val.get() == nullptr) return;

  auto bytes = std::dynamic_pointer_cast<apache::geode::client::internal::
    DataSerializablePrimitive>(val);
  int valSize = val->objectSize();
#if defined(_WIN32)
  int8_t* byteArray = static_cast<int8_t*>(CoTaskMemAlloc(valSize));
#else
  int8_t* byteArray = static_cast<int8_t*>(malloc(valSize));
#endif
  //if (bytes) {
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

// void apache_geode_Region_Put(apache_geode_region_t* region, const char* key,
//                             size_t keyLength, int keyCode, const char* val,
//                             size_t valLength, int valCode) {
//  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
//  if (keyCode == 42)  // CacheableString
//  {
//    Cacheable
//  }
//  regionWrapper->Put(key, keyLength, keyCode, val, valLength, valCode);
//}

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
