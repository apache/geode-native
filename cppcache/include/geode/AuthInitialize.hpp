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

#ifndef GEODE_AUTHINITIALIZE_H_
#define GEODE_AUTHINITIALIZE_H_

/**
 * @file
 */

#include <memory>
#include <string>

#include "internal/geode_globals.hpp"

namespace apache {
namespace geode {
namespace client {

class Properties;

/**
 * @class AuthInitialize AuthInitialize.hpp
 * Specifies the mechanism to obtain credentials for a client.
 * It is mandantory for clients when the server is running in secure
 * mode having a <code>security-client-authenticator</code> module specified.
 * Implementations should register the library path as
 * <code>security-client-auth-library</code> system property and factory
 * function (a zero argument function returning pointer to an
 * AuthInitialize object) as the <code>security-client-auth-factory</code>
 * system property.
 */
class APACHE_GEODE_EXPORT AuthInitialize {
 public:
  virtual ~AuthInitialize() noexcept = default;

  /**@brief initialize with the given set of security properties
   * and return the credentials for the client as properties.
   * @param securityprops the set of security properties provided to the
   * <code>DistributedSystem.connect</code> method
   * @param server it is the ID of the current endpoint.
   * The format expected is "host:port".
   * @returns the credentials to be used for the given <code>server</code>
   * @remarks This method can modify the given set of properties. For
   * example it may invoke external agents or even interact with the user.
   */
  virtual std::shared_ptr<Properties> getCredentials(
      const std::shared_ptr<Properties>& securityprops,
      const std::string& server) = 0;

  /**@brief Invoked before the cache goes down. */
  virtual void close() = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_AUTHINITIALIZE_H_
