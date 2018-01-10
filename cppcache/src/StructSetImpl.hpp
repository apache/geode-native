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

class _GEODE_EXPORT StructSetImpl
    : public StructSet,
      public std::enable_shared_from_this<StructSetImpl> {
 public:
  StructSetImpl(const std::shared_ptr<CacheableVector>& values,
                const std::vector<std::string>& fieldNames);

  bool isModifiable() const;

  int32_t size() const;

  const std::shared_ptr<Serializable> operator[](int32_t index) const;

  const int32_t getFieldIndex(const std::string& fieldname);

  const std::string& getFieldName(int32_t index);

  SelectResultsIterator getIterator();

  /** Get an iterator pointing to the start of vector. */
  virtual SelectResults::Iterator begin() const;

  /** Get an iterator pointing to the end of vector. */
  virtual SelectResults::Iterator end() const;

  virtual ~StructSetImpl();

 private:
  std::shared_ptr<CacheableVector> m_structVector;

  std::map<std::string, size_t> m_fieldNameIndexMap;

  int32_t m_nextIndex;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STRUCTSETIMPL_H_
