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

#ifndef GEODE_EXPIRATIONATTRIBUTES_H_
#define GEODE_EXPIRATIONATTRIBUTES_H_

#include <chrono>

#include "ExpirationAction.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {
/**
 * @class ExpirationAttributes ExpirationAttributes.hpp
 *
 * Immutable parameter object for accessing and setting the attributes
 * associated with
 * <code>timeToLive</code> and <code>idleTimeout</code>. If the expiration
 * action is not specified, it defaults to
 * <code>ExpirationAction.INVALIDATE</code>.
 * If the timeout is not specified, it defaults to zero (which means to never
 * time out).
 *
 * @see RegionAttributesFactory
 * @see RegionAttributes
 * @see AttributesMutator
 */
class APACHE_GEODE_EXPORT ExpirationAttributes {
  /**
   * @brief public methods
   */
 public:
  /**
   *@brief  constructors
   */

  /** Constructs a default <code>ExpirationAttributes</code>, which indicates no
   * expiration
   * will take place.
   */
  ExpirationAttributes();

  /** Constructs an <code>ExpirationAttributes</code> with the specified
   * expiration time and
   * expiration action.
   * @param expirationTime Duration live before it expires
   * @param expirationAction the action to take when the value expires
   * @throws IllegalArgumentException if expirationTime is nonpositive
   */
  explicit ExpirationAttributes(
      const std::chrono::seconds& expirationTime,
      const ExpirationAction expirationAction = ExpirationAction::INVALIDATE);

  /** Returns the duration before a region or value expires.
   *
   * @return the duration before a region or value expires or zero if it will
   * never expire
   */
  const std::chrono::seconds& getTimeout() const;
  void setTimeout(const std::chrono::seconds& timeout);

  /** Returns the action that should take place when this value or region
   * expires.
   *
   * @return the action to take when expiring
   */
  ExpirationAction getAction() const;
  void setAction(const ExpirationAction& action);

 private:
  ExpirationAction m_action;
  std::chrono::seconds m_timeout;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPIRATIONATTRIBUTES_H_
