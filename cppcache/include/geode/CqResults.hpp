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

#ifndef GEODE_CQRESULTS_H_
#define GEODE_CQRESULTS_H_

/**
 * @file
 */

#include "CacheableBuiltins.hpp"
#include "ExceptionTypes.hpp"
#include "SelectResults.hpp"
#include "Serializable.hpp"
#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class CqResults CqResults.hpp
 *
 * A CqResults is obtained by executing a Query on the server.
 * This will be a StructSet.
 */
class APACHE_GEODE_EXPORT CqResults : public SelectResults {};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQRESULTS_H_
