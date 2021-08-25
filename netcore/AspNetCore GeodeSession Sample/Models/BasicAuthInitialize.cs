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
using Apache.Geode.Client;
using System;
using System.Collections.Generic;

namespace GemFireSessionState.Models
{
  public class BasicAuthInitialize : IAuthInitialize
  {
    private string _username;
    private string _password;

    public BasicAuthInitialize(string username, string password)
    {
      _username = username;
      _password = password;
    }

    public void Close()
    {
    }

    public Dictionary<string, string> GetCredentials()
    {
      Console.WriteLine("SimpleAuthInitialize::GetCredentials called");
      var credentials = new Dictionary<string, string>();
      credentials.Add("security-username", "root");
      credentials.Add("security-password", "root-password");
      return credentials;
    }
  }
}