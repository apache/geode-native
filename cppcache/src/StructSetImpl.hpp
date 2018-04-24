#pragma once

#ifndef GEODE_STRUCTSETIMPL_H_
#define GEODE_STRUCTSETIMPL_H_

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

#include <geode/internal/geode_globals.hpp>

#include <geode/StructSet.hpp>
#include <geode/Struct.hpp>
#include <geode/CacheableBuiltins.hpp>

#include <geode/SelectResultsIterator.hpp>

#include <string>
#include <map>

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class APACHE_GEODE_EXPORT StructSetImpl : public StructSet {
 public:
  StructSetImpl(const std::shared_ptr<CacheableVector>& values,
                const std::vector<std::string>& fieldNames);

  ~StructSetImpl() noexcept override = default;

  size_t size() const override;

  const std::shared_ptr<Serializable> operator[](size_t index) const override;

  size_t getFieldIndex(const std::string& fieldname) override;

  const std::string& getFieldName(int32_t index) override;

  virtual SelectResults::iterator begin() const override;

  virtual SelectResults::iterator end() const override;

 private:
  std::shared_ptr<CacheableVector> m_structVector;

  std::map<std::string, int32_t> m_fieldNameIndexMap;

  size_t m_nextIndex;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STRUCTSETIMPL_H_
