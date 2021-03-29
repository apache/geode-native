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

#ifndef GEODE_INTEGRATION_TEST_BBNAMINGCONTEXT_H_
#define GEODE_INTEGRATION_TEST_BBNAMINGCONTEXT_H_

// This these classes should have been in the framework libary. If we ever use
// these, instead of the ACE context, onto windows and Linux then we should
// move them there.
// This will avoid pulling in a lot of framework headers to cause compilation
// grieve, especially with the stl stuff.
#include <cstdlib>
#include <string>

namespace apache {
namespace geode {
namespace client {
namespace testframework {

class BBNamingContextClientImpl;
class BBNamingContextClient {
  BBNamingContextClientImpl* m_impl;

 public:
  BBNamingContextClient();
  ~BBNamingContextClient();
  void open();
  void close();
  int rebind(const char* key, const char* value, char* type = nullptr);
  void dump();
  int resolve(const std::string& key, std::string& value, char* type = nullptr);
};
class BBNamingContextServerImpl;
class BBNamingContextServer {
 public:
  BBNamingContextServerImpl* m_impl;
  BBNamingContextServer();
  ~BBNamingContextServer();
};

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_BBNAMINGCONTEXT_H_
