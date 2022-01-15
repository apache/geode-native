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

#include "framework/TestConfig.h"

const char *getFrameworkString(FrameworkVariable name) {
  switch(name) {
    case FrameworkVariable::JavaObjectJarPath: return "C:/temp/geode-native/out/build/x64-Debug/tests/javaobject/javaobject.jar";
    case FrameworkVariable::GfShExecutable: return "C:/apache-geode-1.15.0/bin/gfsh.bat";
    case FrameworkVariable::TestCacheXmlDir: return "C:/temp/geode-native/cppcache/integration/framework/../../integration-test/resources";
    case FrameworkVariable::NewTestResourcesDir: return "C:/temp/geode-native/cppcache/integration/framework/../../integration/test/resources";
    case FrameworkVariable::TestClientSslKeysDir: return "C:/temp/geode-native/cppcache/integration/framework/../../../ssl_keys/client_keys";
    case FrameworkVariable::TestServerSslKeysDir: return "C:/temp/geode-native/cppcache/integration/framework/../../../ssl_keys/server_keys";
    case FrameworkVariable::TestSniConfigPath: return "C:/temp/geode-native/cppcache/integration/framework/../../../sni-test-config";
  }
  return "";
}
