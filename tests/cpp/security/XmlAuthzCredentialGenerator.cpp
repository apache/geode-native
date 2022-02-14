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

#include "XmlAuthzCredentialGenerator.hpp"

// Disable warning for "extra qualifications" here.  One of the boost log
// headers triggers this warning.  Note: use of disable pragma here is
// intentional - attempts to use push/pop as you ordinarily should just
// yielded a gripe from the MS tools that "warning number '4596' is not a
// valid compiler warning". re-enabling the warning after the include
// fails in the same way, so just leave it disabled for the rest of the
// file.  This is safe, since the warning can only trigger inside a class
// declaration, of which there are none in this file.
#ifdef WIN32
#pragma warning(disable : 4596)
#endif

#include <boost/log/trivial.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testframework {
namespace security {

void XmlAuthzCredentialGenerator::getAllowedDummyAuthz(ROLES role) {
  const char* adminUsers[] = {"admin", "root", "administrator"};
  const int adminUsrSz = (sizeof adminUsers / sizeof *adminUsers) - 1;
  std::string validity = "invalid";

  if (role == NO_ROLE) {
    role = getRequiredRole();
    validity = "valid";
  }

  std::string userName;
  switch (role) {
    case READER_ROLE:
      userName = "reader" + std::to_string(randomValue(2));
      break;
    case WRITER_ROLE:
      userName = "writer" + std::to_string(randomValue(2));
      break;
    case QUERY_ROLE:
      userName = "reader" + std::to_string(randomValue(1) + 3);
      break;
    case ADMIN_ROLE:
      userName = adminUsers[randomValue(adminUsrSz)];
      break;
    case NO_ROLE:
      userName = "user" + std::to_string(randomValue(2));
      break;
  }

  (*m_prop)->insert("security-username", userName);
  (*m_prop)->insert("security-password", userName);

  BOOST_LOG_TRIVIAL(info)
      << "inserted " << validity << " dummy security-username "
      << (*m_prop)->find("security-username")->value().c_str() << " password "
      << ((*m_prop)->find("security-password") != nullptr
              ? (*m_prop)->find("security-password")->value().c_str()
              : "not set");
}

std::string XmlAuthzCredentialGenerator::getAllowedUser(ROLES role) {
  const std::string userPrefix = "geode";
  const int readerIndices[] = {3, 4, 5};
  const int writerIndices[] = {6, 7, 8};
  const int queryIndices[] = {9, 10};
  const int adminIndices[] = {1, 2};
  const int readerIndSz = (sizeof readerIndices / sizeof *readerIndices) - 1;
  const int writerIndSz = (sizeof writerIndices / sizeof *writerIndices) - 1;
  const int queryIndSz = (sizeof queryIndices / sizeof *queryIndices) - 1;
  const int adminIndSz = (sizeof adminIndices / sizeof *adminIndices) - 1;

  std::string validity = "invalid";

  if (role == NO_ROLE) {
    role = getRequiredRole();
    validity = "valid";
  }
  std::string userName;
  switch (role) {
    case READER_ROLE:
      userName =
          userPrefix + std::to_string(readerIndices[randomValue(readerIndSz)]);
      break;
    case WRITER_ROLE:
      userName =
          userPrefix + std::to_string(writerIndices[randomValue(writerIndSz)]);
      break;
    case QUERY_ROLE:
      userName =
          userPrefix + std::to_string(queryIndices[randomValue(queryIndSz)]);
      break;
    case ADMIN_ROLE:
    case NO_ROLE:
      userName =
          userPrefix + std::to_string(adminIndices[randomValue(adminIndSz)]);
      break;
  }
  BOOST_LOG_TRIVIAL(info) << "inserted " << validity << " username "
                          << userName;
  return userName;
}

void XmlAuthzCredentialGenerator::getAllowedLdapAuthz(ROLES role) {
  const std::string userName = getAllowedUser(role);
  (*m_prop)->insert("security-username", userName.c_str());
  (*m_prop)->insert("security-password", userName.c_str());

  BOOST_LOG_TRIVIAL(info)
      << "inserted  ldap security-username "
      << (*m_prop)->find("security-username")->value() << " password "
      << ((*m_prop)->find("security-password") != nullptr
              ? (*m_prop)->find("security-password")->value()
              : "not set");
}

void XmlAuthzCredentialGenerator::getAllowedPkcsAuthz(ROLES role) {
  const std::string userName = getAllowedUser(role);
  (*m_prop)->insert("security-alias", userName.c_str());
  (*m_prop)->insert("security-keystorepass", "geode");

  BOOST_LOG_TRIVIAL(info)
      << "inserted  PKCS security-alias"
      << (*m_prop)->find("security-alias")->value() << " password "
      << ((*m_prop)->find("security-keystorepass") != nullptr
              ? (*m_prop)->find("security-keystorepass")->value()
              : "not set");
}

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
