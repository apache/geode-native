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

#ifndef GEODE_EXPIRATIONACTION_H_
#define GEODE_EXPIRATIONACTION_H_

#include <iosfwd>
#include <string>

#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @enum ExpirationAction ExpirationAction.hpp
 * Enumerated type for expiration actions.
 *
 * @see ExpirationAttributes
 */
enum class ExpirationAction {
  /** When the region or cached object expires, it is invalidated. */
  INVALIDATE = 0,
  /** When expired, invalidated locally only. */
  LOCAL_INVALIDATE,
  /** When the region or cached object expires, it is destroyed. */
  DESTROY,
  /** When expired, destroyed locally only. */
  LOCAL_DESTROY,
  /** invalid type. */
  INVALID_ACTION
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPIRATIONACTION_H_
