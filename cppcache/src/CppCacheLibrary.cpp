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

#include "CppCacheLibrary.hpp"

#include <iostream>

#include <ace/ACE.h>
#include <ace/Init_ACE.h>

#include "Utils.hpp"

void initLibDllEntry(void) {
  apache::geode::client::CppCacheLibrary::initLib();
}

extern "C" {
void DllMainGetPath(char* result, int maxLen);
}

namespace apache {
namespace geode {
namespace client {

void CppCacheLibrary::initLib(void) { ACE::init(); }

void CppCacheLibrary::closeLib(void) {
  // DO NOT CALL ACE::fini() HERE!
  // Things might be using ace beyond the life of Geode.
}

std::string CppCacheLibrary::initProductLibDir() {
  // otherwise... get the DLL path, and work backwards from it.
  char buffer[PATH_MAX + 1];
  buffer[0] = '\0';
  DllMainGetPath(buffer, PATH_MAX);

  std::string path(buffer);

#ifdef WIN32
  std::replace(path.begin(), path.end(), '\\', '/');
#endif

  const auto pos = path.rfind('/');
  if (std::string::npos != pos) {
    return path.substr(0, pos);
  }

  return std::string();
}

const std::string& CppCacheLibrary::getProductLibDir() {
  static const std::string productLibDir = initProductLibDir();
  return productLibDir;
}

std::string CppCacheLibrary::initProductDir() {
  // If the environment variable is set, use it.
  auto geodeNativeEnvironment = Utils::getEnv("GEODE_NATIVE_HOME");
  if (geodeNativeEnvironment.length() > 0) {
    return geodeNativeEnvironment;
  }

  // otherwise... get the DLL path, and work backwards from it.
  auto productLibraryDirectoryName = getProductLibDir();
  if (productLibraryDirectoryName.empty()) {
    std::cerr << "Cannot determine location of product directory.\n"
              << "Please set GEODE_NATIVE_HOME environment variable.\n"
              << std::flush;
    throw apache::geode::client::IllegalStateException(
        "Product installation directory not found. Please set "
        "GEODE_NATIVE_HOME environment variable.");
  }

  // check if bin on windows, and go back one...
#ifdef WIN32
  std::string libpart = "bin";
#else
  std::string libpart = "lib";
#endif
  if (productLibraryDirectoryName.substr(productLibraryDirectoryName.length() -
                                         3) == libpart) {
    return productLibraryDirectoryName.substr(
        0, productLibraryDirectoryName.length() - 4);
  } else {
    return productLibraryDirectoryName;
  }
}

const std::string& CppCacheLibrary::getProductDir() {
  static const std::string productDir = initProductDir();
  return productDir;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
