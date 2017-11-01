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

#include "CredentialGenerator.hpp"
#include "DummyCredentialGenerator.hpp"
#include "DummyCredentialGenerator2.hpp"
#include "DummyCredentialGenerator3.hpp"
#include "LdapUserCredentialGenerator.hpp"
#include "PkcsCredentialGenerator.hpp"
#include "NoopCredentialGenerator.hpp"

using namespace apache::geode::client::testframework::security;

CredentialGenerator::registeredClassMap* CredentialGenerator::generatormap =
    NULL;

std::shared_ptr<CredentialGenerator> CredentialGenerator::create(std::string scheme) {
  if (generators().find(scheme) != generators().end()) {
    return generators()[scheme];

    // first call to create, nothing will be registered until now.
  } else if (generators().size() == 0) {
    registerScheme(std::shared_ptr<CredentialGenerator>(new CredentialGenerator()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator2()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new DummyCredentialGenerator3()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new LdapUserCredentialGenerator()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new PKCSCredentialGenerator()));
    registerScheme(std::shared_ptr<CredentialGenerator>(new NoopCredentialGenerator()));
    return create(scheme);

  } else {
    return std::shared_ptr<CredentialGenerator>(new CredentialGenerator());
  }
}
