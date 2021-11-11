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

#ifndef GEODE_SECURITY_XMLAUTHZCREDENTIALGENERATOR_H_
#define GEODE_SECURITY_XMLAUTHZCREDENTIALGENERATOR_H_

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>

#include <geode/CacheableString.hpp>
#include <geode/Properties.hpp>

#include "typedefs.hpp"

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

template <typename T>
T randomValue(T maxValue) {
  static thread_local std::default_random_engine generator(
      std::random_device{}());
  return std::uniform_int_distribution<T>{0, maxValue}(generator);
}

const opCodeList::value_type RArr[] = {
    OP_GET,     OP_GETALL,      OP_REGISTER_INTEREST, OP_UNREGISTER_INTEREST,
    OP_KEY_SET, OP_CONTAINS_KEY};

const opCodeList::value_type WArr[] = {
    OP_CREATE,     OP_UPDATE,       OP_PUTALL,          OP_DESTROY,
    OP_INVALIDATE, OP_REGION_CLEAR, OP_EXECUTE_FUNCTION};

const opCodeList::value_type QArr[] = {OP_QUERY, OP_REGISTER_CQ};

const stringList::value_type QRArr[] = {"Portfolios", "Positions"};

class XmlAuthzCredentialGenerator;

class XmlAuthzCredentialGenerator {
 private:
  ID m_id;
  opCodeList* m_opCode;
  stringList* m_regionNames;
  std::shared_ptr<Properties>* m_prop;

  opCodeList Readers;
  opCodeList Writers;
  opCodeList Query;
  stringList QueryRegions;

 public:
  explicit XmlAuthzCredentialGenerator(ID id)
      : m_id(id),
        Readers(RArr, RArr + sizeof RArr / sizeof *RArr),
        Writers(WArr, WArr + sizeof WArr / sizeof *WArr),
        Query(QArr, QArr + sizeof QArr / sizeof *QArr),
        QueryRegions(QRArr, QRArr + sizeof QRArr / sizeof *QRArr) {
    m_opCode = nullptr;
    m_regionNames = nullptr;
    m_prop = nullptr;
    /* initialize random seed: */
    srand(static_cast<unsigned int>(time(nullptr)));
  }
  virtual ~XmlAuthzCredentialGenerator() {}

  virtual void getAllowedCredentials(opCodeList& opCode,
                                     std::shared_ptr<Properties>& prop,
                                     stringList* regionNames) {
    try {
      m_opCode = &opCode;
      m_regionNames = regionNames;
      m_prop = &prop;

      switch (m_id) {
        case ID_DUMMY:
          getAllowedDummyAuthz(NO_ROLE);
          break;
        case ID_LDAP:
          getAllowedLdapAuthz(NO_ROLE);
          break;
        case ID_PKI:
          getAllowedPkcsAuthz(NO_ROLE);
          break;
        case ID_NONE:
        case ID_NOOP:
        case ID_DUMMY2:
        case ID_DUMMY3:
          break;
      }

    } catch (...) {
      reset();
    }

    reset();
  }

  void reset() {
    m_opCode = nullptr;
    m_regionNames = nullptr;
    m_prop = nullptr;
  }

  virtual void getDisallowedCredentials(opCodeList& opCode,
                                        std::shared_ptr<Properties>& prop,
                                        stringList* regionNames) {
    try {
      m_opCode = &opCode;
      m_regionNames = regionNames;
      m_prop = &prop;
      ROLES role = getRequiredRole();
      switch (role) {
        case READER_ROLE:
          role = WRITER_ROLE;
          break;
        case WRITER_ROLE:
          role = READER_ROLE;
          break;
        case QUERY_ROLE:
          role = WRITER_ROLE;
          break;
        case ADMIN_ROLE:
          role = QUERY_ROLE;
          break;
        case NO_ROLE:
          /* UNNECESSARY role = role*/ break;
      }

      switch (m_id) {
        case ID_DUMMY:
          getAllowedDummyAuthz(role);
          break;
        case ID_LDAP:
          getAllowedLdapAuthz(role);
          break;
        case ID_PKI:
          getAllowedPkcsAuthz(role);
          break;
        case ID_NONE:
        case ID_NOOP:
        case ID_DUMMY2:
        case ID_DUMMY3:
          break;
      }

    } catch (...) {
      reset();
    }

    reset();
  }

 private:
  void getAllowedDummyAuthz(ROLES role);
  std::string getAllowedUser(ROLES role);
  void getAllowedLdapAuthz(ROLES role);
  void getAllowedPkcsAuthz(ROLES role);

  ROLES getRequiredRole() {
    bool requireReaders = true, requireWriters = true, requireQuery = true;
    ROLES role = ADMIN_ROLE;

    for (opCodeList::iterator it = m_opCode->begin(); it != m_opCode->end();
         it++) {
      if (requireReaders &&
          std::find(Readers.begin(), Readers.end(), (*it)) == Readers.end()) {
        requireReaders = false;
      }

      if (requireWriters &&
          std::find(Writers.begin(), Writers.end(), (*it)) == Writers.end()) {
        requireWriters = false;
      }

      if (requireQuery &&
          std::find(Query.begin(), Query.end(), (*it)) == Query.end()) {
        requireQuery = false;
      }
    }

    if (requireReaders) {
      role = READER_ROLE;
    } else if (requireWriters) {
      role = WRITER_ROLE;
    } else if (requireQuery) {
      role = QUERY_ROLE;
    }

    return role;
  }  // end of requireRole
};

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SECURITY_XMLAUTHZCREDENTIALGENERATOR_H_
