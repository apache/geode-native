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
using System.Collections.Generic;
using Apache.Geode.Client;

namespace Apache.Geode.Examples.Transaction
{
  class Program
  {
    private static readonly List<string> keys = new List<string> {
      "Key1",
      "Key2",
      "Key3",
      "Key4",
      "Key5",
      "Key6",
      "Key7",
      "Key8",
      "Key9",
      "Key10"
    };

    private static readonly Random getRandom = new Random();

    private static int getValueFromExternalSystem() {
      var value = getRandom.Next(20);
      if (value == 0)
      {
        throw new Exception("failed to get from external system");
      }
      return value;
    }

    static void Main(string[] args)
    {
      var cache = new CacheFactory()
          .Set("log-level", "none")
          .Create();

      cache.GetPoolManager()
          .CreateFactory()
          .AddLocator("localhost", 10334)
          .Create("pool");

      Console.WriteLine("Created cache");

      var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
          .SetPoolName("pool");
      var region = regionFactory.Create<string, int>("exampleRegion");

      Console.WriteLine("Created region 'exampleRegion'");

      var retries = 5;
      while(retries-- > 0)
      {
        try 
        {
          cache.CacheTransactionManager.Begin();
          foreach(var key in keys)
          {
            var value = getValueFromExternalSystem();
            region.Put(key, value);
          }
          cache.CacheTransactionManager.Commit();
          Console.WriteLine("Committed transaction - exiting");
          break;
        } catch
        {
          cache.CacheTransactionManager.Rollback();
          Console.WriteLine("Rolled back transaction - retrying({0})", retries);
        }
      }
    }
  }
}
