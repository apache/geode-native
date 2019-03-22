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

#ifndef GEODE_STRUCTSET_H_
#define GEODE_STRUCTSET_H_

#include "CqResults.hpp"
#include "Struct.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class StructSet StructSet.hpp
 *
 * A StructSet may be obtained after executing a Query which is obtained from a
 * QueryService which in turn is obtained from a Cache.
 * It is the parent of a Struct which contains the field values.
 */
class APACHE_GEODE_EXPORT StructSet : public CqResults {
 public:
  ~StructSet() noexcept override = default;

  /**
   * Get the index number of the specified field name in the StructSet.
   *
   * @param fieldname the field name for which the index is required.
   * @returns the index number of the specified field name.
   * @throws std::invalid_argument if the field name is not found.
   */
  virtual int32_t getFieldIndex(const std::string& fieldname) = 0;

  /**
   * Get the field name of the StructSet from the specified index number.
   *
   * @param index the index number of the field name to get.
   * @returns the field name from the specified index number or nullptr if not
   * found.
   * @throws std::out_of_range if index is not found
   */
  virtual const std::string& getFieldName(int32_t index) = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STRUCTSET_H_
