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

using System;
using System.Runtime.InteropServices;

public class Config
{
  public static string GeodeGfsh
  {
    //get { return @"C:/Users/pivotal/Src/Repos/geode/geode-assembly/build/install/apache-geode/bin/gfsh.bat"; }
     get {
       if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
         return Environment.GetEnvironmentVariable("GEODE_HOME") + "/bin/gfsh.bat";
       }
       else {
         return Environment.GetEnvironmentVariable("GEODE_HOME") + "/bin/gfsh";
       }
     }
  }

  public static string JavaobjectJarPath
  {
    //get { return @"C:/geode-native-develop/build/tests/javaobject/javaobject.jar"; }
    get { return Environment.GetEnvironmentVariable("GEODE_NATIVE_BUILD_DIR") + "/tests/javaobject/javaobject.jar"; }
  }

  public static string SslServerKeyPath
  {
	  //get { return @"C:/geode-native-develop/clicache/integration-test2/../../ssl_keys/server_keys"; }
    get { return "../../ssl_keys/server_keys"; }
  }

  public static string SslClientKeyPath
  {
	  //get { return @"C:/geode-native-develop/clicache/integration-test2/../../ssl_keys/client_keys"; }
    get { return "../../ssl_keys/client_keys"; }
  }
}
