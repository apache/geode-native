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

#include <string>
#include <memory>

#include "client.hpp"
#include "geode/Region.hpp"

class RegionFactoryWrapper;

class RegionWrapper : public ClientKeeper {
  std::shared_ptr<apache::geode::client::Region> region_;
  std::string lastValue_;

 public:
  RegionWrapper(std::shared_ptr<apache::geode::client::Region> region);
  ~RegionWrapper();

  void PutString(const std::string& key, const std::string& value);

  void PutByteArray(const std::string& key, const char* value, size_t size);

  const char* GetString(const std::string& key);

  void GetByteArray(const std::string& key, char** value, size_t* size);

  void Remove(const std::string& key);

  bool ContainsValueForKey(const std::string& key);
};
