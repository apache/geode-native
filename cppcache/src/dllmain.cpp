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

#include <cstdio>
#include <cstdlib>
#include <string>

#include <geode/Exception.hpp>

#include "CppCacheLibrary.hpp"
#include "Utils.hpp"
#include "config.h"

void initLibDllEntry(void);

extern "C" {

static bool initgflib() {
  initLibDllEntry();
  return true;
}

static bool initgflibDone = initgflib();

#ifdef _WIN32
#include <windows.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_THREAD_DETACH: {
      break;
    }
    case DLL_PROCESS_DETACH: {
      break;
    }
    // Do not do *anything* in attach phase unless absolutely required
    // and absolutely sure that it follows all restrictions; this has
    // caused much grief with AccessViolations in the past.
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH: {
      break;
    }
  }
  return TRUE;
}

void DllMainGetPath(char *result, int maxLen) {
  if (!initgflibDone) {
    result[0] = '\0';
    return;
  }
  std::string cppLibName = PRODUCT_LIB_NAME;
  cppLibName += ".dll";
  std::string dotNetLibName = PRODUCT_DLL_NAME;
  dotNetLibName += ".dll";
  HMODULE module = GetModuleHandle(cppLibName.c_str());
  if (module == 0) {
    module = GetModuleHandle(dotNetLibName.c_str());
  }
  if (module == 0) {
    module = (HMODULE)&__ImageBase;
  }
  if (module != 0) {
    GetModuleFileName(module, result, maxLen);
  } else {
    result[0] = '\0';
  }
}

#else
#include <dlfcn.h>

void DllMainGetPath(char *result, int) {
  if (!initgflibDone) {
    result[0] = '\0';
    return;
  }
  Dl_info dlInfo;
  dladdr(reinterpret_cast<void *>(DllMainGetPath), &dlInfo);
  if (realpath(dlInfo.dli_fname, result) == nullptr) {
    result[0] = '\0';
  }
}

#endif /* WIN32 */

} /* extern "C" */
