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

// GENERATED FROM Config.cs.in DO NOT EDIT Config.cs

public class Config
{
  public static string GeodeGfsh
  {
    get { return @"C:/apache-geode-1.15.0/bin/gfsh.bat"; }
  }

  public static string JavaobjectJarPath
  {
    get { return @""; }
  }

  public static string SslServerKeyPath
  {
	get { return @"C:/temp/geode-native/netcore/shared/../../ssl_keys/server_keys"; }
  }

  public static string SslClientKeyPath
  {
	get { return @"C:/temp/geode-native/netcore/shared/../../ssl_keys/client_keys"; }
  }

  public static string SniConfigPath
  {
	get { return @"C:/temp/geode-native/netcore/shared/../../sni-test-config"; }
  }
}
