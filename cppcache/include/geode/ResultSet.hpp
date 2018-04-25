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

#ifndef GEODE_RESULTSET_H_
#define GEODE_RESULTSET_H_

#include "internal/geode_globals.hpp"
#include "SelectResults.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class ResultSet ResultSet.hpp
 * A ResultSet may be obtained after executing a Query which is obtained from a
 * QueryService which in turn is obtained from a Cache.
 */
class APACHE_GEODE_EXPORT ResultSet : public SelectResults {
 public:
  ~ResultSet() noexcept override = default;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_RESULTSET_H_
