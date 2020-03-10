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

#include <ace/ACE.h>
#include <ace/Init_ACE.h>

#include "Utils.hpp"

// called during DLL initialization
void initLibDllEntry(void) {
  apache::geode::client::CppCacheLibrary::initLib();
}

extern "C" {
void DllMainGetPath(char* result, int maxLen);
}

namespace apache {
namespace geode {
namespace client {

// expect this to be called from key Library entry points, or automatically
// if we can... Probably safest to call from DistributedSystem factory method.
// impl type Unit tests may need to call this themselves to ensure the
// internals are prepared. fw_helper framework will handle this.
void CppCacheLibrary::initLib(void) { ACE::init(); }

// this closes ace and triggers the cleanup of the singleton CppCacheLibrary.
void CppCacheLibrary::closeLib(void) {
  // ACE::fini(); This should not happen..... Things might be using ace beyond
  // the life of
  // using geode.
}

// return the directory where the library/DLL resides
std::string CppCacheLibrary::getProductLibDir() {
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

std::string CppCacheLibrary::getProductDir() {
  // If the environment variable is set, use it.
  auto geodeNativeEnvironment = Utils::getEnv("GEODE_NATIVE_HOME");
  if (geodeNativeEnvironment.length() > 0) {
    return geodeNativeEnvironment;
  }

  // otherwise... get the DLL path, and work backwards from it.
  auto productLibraryDirectoryName = getProductLibDir();
  if (productLibraryDirectoryName.empty()) {
    fprintf(stderr,
            "Cannot determine location of product directory.\n"
            "Please set GEODE_NATIVE_HOME environment variable.\n");
    fflush(stderr);
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

}  // namespace client
}  // namespace geode
}  // namespace apache
