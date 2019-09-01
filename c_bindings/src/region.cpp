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

// C++ client public headers
#include "geode/CacheableString.hpp"
#include "geode/Region.hpp"
#include "geode/RegionShortcut.hpp"

// C client public headers
#include "geode/region.h"
#include "geode/region/factory.h"

// C client private headers
#include "region.hpp"
#include "region/factory.hpp"

RegionWrapper::RegionWrapper(RegionFactoryWrapper &region_factory, std::shared_ptr<apache::geode::client::Region> region)
    : region_(region), region_factory{region_factory} {
      region_factory.AddRecord(this, "RegionWrapper");
    }

RegionWrapper::~RegionWrapper() { region_factory.RemoveRecord(this); }

void RegionWrapper::PutString(const std::string& key,
                              const std::string& value) {
  region_->put(key, value);
}

void RegionWrapper::PutByteArray(const std::string& key, const char* value,
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
  std::shared_ptr<apache::geode::client::Serializable> val = region_->get(key);

  if (val.get() == nullptr) return;

  auto bytes =
      std::dynamic_pointer_cast<apache::geode::client::CacheableBytes>(val);
  const int8_t* p = bytes->value().data();
  int valSize = val->objectSize();
#if defined(_WIN32)
  int8_t* byteArray = (int8_t*)CoTaskMemAlloc(valSize);
#else
  int8_t* byteArray = (int8_t*)malloc(valSize);
#endif
  memcpy(byteArray, bytes->value().data(), valSize);
  *value = (char*)byteArray;
  *size = valSize;
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

const char* apache_geode_Region_GetString(apache_geode_region_t* region,
                                          const char* key) {
  RegionWrapper* regionWrapper = reinterpret_cast<RegionWrapper*>(region);
  return regionWrapper->GetString(key);
}

void apache_geode_Region_GetByteArray(apache_geode_region_t* region,
                                             const char* key, char** value, size_t* size) {
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
