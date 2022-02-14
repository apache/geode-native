#pragma once

#ifndef GEODE_SECURITY_CREDENTIALGENERATOR_H_
#define GEODE_SECURITY_CREDENTIALGENERATOR_H_

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

/**
 * @file    CredentialGenerator.hpp
 * @since   1.0
 * @version 1.0
 * @see
 *
 */

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

#include <map>

#include <geode/Properties.hpp>

#include "typedefs.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

class CredentialGenerator;

/**
 * @class CredentialGenerator CredentialGenerator.hpp
 * Encapsulates valid or invalid credentials required for Client connection
 * and server command parameters to be supplied.
 * <p>
 * Different implementations are there for different kinds of authentication/
 * authorization schemes.
 * <p>
 *
 * for client connection credentials
 *   cg->getAuthInit(prop);
 *   cg->getValidCredentials(prop);
 *
 * for server command line security arguments
 *   str = cg->getServerCmdParams(":authenticator:authorizer");
 *
 */
class CredentialGenerator {
 public:
  typedef std::map<std::string, std::shared_ptr<CredentialGenerator>>
      registeredClassMap;

 private:
  ID m_id;
  std::string m_name;
  static registeredClassMap* generatormap;

  CredentialGenerator() : m_id(ID_NONE), m_name("NONE") {}

 protected:
  CredentialGenerator(ID id, std::string name) : m_id(id), m_name(name) {}
  CredentialGenerator(const CredentialGenerator& other)
      : m_id(other.m_id), m_name(other.m_name) {}

 public:
  CredentialGenerator& operator=(const CredentialGenerator& other) {
    if (this == &other) return *this;

    this->m_id = other.m_id;
    this->m_name = other.m_name;

    return *this;
  }
  virtual ~CredentialGenerator() {}
  bool operator==(const CredentialGenerator* other) {
    if (!other) {
      return false;
    }
    return (m_id == other->m_id && m_name == other->m_name);
  }

 public:
  /**
   * @brief create new Credential Generator
   * @param scheme can be one of the following
   *    DUMMY - simple username/password authentication
   *    LDAP  - LDAP server based authentication.
   *    PKCS  - Digital signature based authentication
   *    NONE  - Disable security altogether.
   * **/
  static std::shared_ptr<CredentialGenerator> create(std::string scheme);
  static bool registerScheme(std::shared_ptr<CredentialGenerator> scheme) {
    // if not already registered...
    if (generators().find(scheme->m_name) == generators().end()) {
      generators()[scheme->m_name] = scheme;
      return true;
    }
    return false;
  }
  static registeredClassMap& getRegisteredSchemes() { return generators(); }

  static registeredClassMap& generators() {
    if (!CredentialGenerator::generatormap) {
      CredentialGenerator::generatormap = new registeredClassMap;
    }
    return *CredentialGenerator::generatormap;
  }

  ID id() { return m_id; }
  std::string name() { return m_name; }

  std::string toString() { return std::to_string(m_id) + m_name; }
  static void dump();

  void hashCode() {}

  void getAuthInit(std::shared_ptr<Properties>& prop);

  std::string getPublickeyfile();

  std::string getServerCmdParams(std::string securityParams,
                                 std::string workingDir = "",
                                 bool userMode = false);

  virtual void getValidCredentials(std::shared_ptr<Properties>& p);

  virtual void getInvalidCredentials(std::shared_ptr<Properties>& p);

  virtual void getAllowedCredentialsForOps(opCodeList& opCodes,
                                           std::shared_ptr<Properties>& p,
                                           stringList* regionNames);

  virtual void getDisallowedCredentialsForOps(opCodeList& opCodes,
                                              std::shared_ptr<Properties>& p,
                                              stringList* regionNames);

  static registeredClassMap& getRegisterdSchemes() {
    if (generators().size() == 0) {
      // calling for registering the existing schemes.
      create("");
    }
    return generators();
  }

 public:
  virtual std::string getClientAuthInitLoaderFactory() { return ""; }
  virtual std::string getClientAuthInitLoaderLibrary() { return ""; }
  virtual std::string getInitArgs(std::string workingDir, bool userMode);
  virtual std::string getClientAuthenticator() { return ""; }
  virtual std::string getClientAuthorizer() { return ""; }
  virtual std::string getClientDummyAuthorizer() { return ""; }
};

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SECURITY_CREDENTIALGENERATOR_H_
