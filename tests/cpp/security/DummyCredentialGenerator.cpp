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

#include "DummyCredentialGenerator.hpp"

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

std::string DummyCredentialGenerator::getInitArgs(std::string workingDir,
                                                  bool userMode) {
  std::string additionalArgs;
  char* buildDir = ACE_OS::getenv("BUILDDIR");
  if (buildDir && workingDir.length() == 0) {
    workingDir = std::string(buildDir);
    workingDir += std::string("/framework/xml/Security/");
  }

  BOOST_LOG_TRIVIAL(info) << "Inside dummy Credentials usermode is "
                          << userMode;

  if (userMode) {
    additionalArgs = std::string(" --J=-Dgemfire.security-authz-xml-uri=") +
                     std::string(workingDir) + std::string("authz-dummyMU.xml");
  } else {
    additionalArgs = std::string(" --J=-Dgemfire.security-authz-xml-uri=") +
                     std::string(workingDir) + std::string("authz-dummy.xml");
  }

  return additionalArgs;
}

void DummyCredentialGenerator::getValidCredentials(
    std::shared_ptr<Properties>& p) {
  p->insert("security-username", "user1");
  p->insert("security-password", "user1");

  BOOST_LOG_TRIVIAL(debug) << "inserted valid security-username "
                           << p->find("security-username")->value()
                           << " password "
                           << p->find("security-password")->value();
}

void DummyCredentialGenerator::getInvalidCredentials(
    std::shared_ptr<Properties>& p) {
  p->insert("security-username", "1user");
  p->insert("security-password", "user1");

  BOOST_LOG_TRIVIAL(debug) << "inserted invalid security-username "
                           << p->find("security-username")->value()
                           << " password "
                           << p->find("security-password")->value();
}

}  // namespace security
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
