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
using System.Net.Cache;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests {
  public class SimpleAuthInitialize : IAuthInitialize {
    public Dictionary<string, string> GetCredentials() {
      Console.WriteLine("SimpleAuthInitialize::GetCredentials called");
      var credentials = new Dictionary<string, string>();
      credentials.Add("security-username", "server");
      credentials.Add("security-password", "server");
      return credentials;
    }

    public void Close() {
      Console.WriteLine("SimpleAuthInitialize::Close called");
    }
  }

  [Collection("Geode .Net Core Collection")]
  public class RegionFactoryTest : TestBase
  {
    public RegionFactoryTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
    {
    }

    private const string Username1 = "rtimmons";
    private const string Username2 = "scharles";
    private const string Username3 = "michael";
    private const string Username4 = "george";

    private void createPool(IGeodeCache<object, object> cache, int port) {
      using var poolManager = cache.PoolManager;
      using var poolFactory = poolManager.CreatePoolFactory().AddLocator("localhost", port);
      using var pool =
          poolFactory.CreatePool("myPool");  // lgtm[cs / useless - assignment - to - local]
    }

    private void doPutsAndGetsObject(IRegion<object, object> region)
    {
      // Key is a string

      var expectedFullName = "Robert Timmons";
      region.Put(Username1, expectedFullName);
      var actualFullName = region.Get(Username1);
      Assert.Equal(expectedFullName, actualFullName);

      Int16 expectedInt16 = 780;
      region.Put(Username4, expectedInt16);
      var actualInt16 = region.Get(Username4);
      Assert.Equal(expectedInt16, actualInt16);

      Int32 expectedInt32 = 779;
      region.Put(Username3, expectedInt32);
      var actualInt32 = region.Get(Username3);
      Assert.Equal(expectedInt32, actualInt32);

      // Key is an Int16

      var short5000 = (Int16)5000;
      region.Put((short)10000, short5000);
      var actual5000 = region.Get((short)10000);
      Assert.Equal(short5000, actual5000);

      // Key is a Byte

      var byteIndex= (byte)222;
      region.Put((byte)200, byteIndex);
      var actualByteIndex = region.Get((byte)200);
      Assert.Equal(byteIndex, actualByteIndex);

      //Int16 expectedInt16 = 780;
      //region.Put(Username4, expectedInt16);
      //var actualInt16 = region.Get(Username4);
      //Assert.Equal(expectedInt16, actualInt16);

      //Int32 expectedInt32 = 779;
      //region.Put(Username3, expectedInt32);
      //var actualInt32 = region.Get(Username3);
      //Assert.Equal(expectedInt32, actualInt32);
    }

    private void DoRemoves(IRegion<string, Int32> region) {
      //region.Remove(Username1);
      //region.Remove(Username2);

      //var hasUser1 = region.ContainsValueForKey(Username1);
      //var hasUser2 = region.ContainsValueForKey(Username2);

      //Assert.False(hasUser1);
      //Assert.False(hasUser2);
    }

    private void CreateRegionAndDoWorkObject(IGeodeCache<object, object> cache, string regionName,
                                       RegionShortcut regionType)
    {
      using var regionFactory = cache.CreateRegionFactory(regionType);
      using var region = regionFactory.CreateRegion(regionName);

      doPutsAndGetsObject(region);
      //DoRemoves(region);
    }

    [Fact]
    public void RegionFactoryCreateRegionStringPutGetWithAuthentication() {
      using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.True(cluster.Start());
        Assert.Equal(0, cluster.Gfsh
            .create()
            .region()
            .withName("authExampleRegion")
            .withType("PARTITION")
            .execute());

        using var cacheFactory = CacheFactory<object, object>.Create()
                                   .SetProperty("log-level", "debug")
                                   .SetProperty("log-file", "geode_native_with_auth.log");
      cacheFactory.AuthInitialize = new SimpleAuthInitialize();
      using var cache = cacheFactory.CreateCache();

      using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkObject(cache, "authExampleRegion", RegionShortcut.CachingProxy);
      }
    }

    [Fact]
    public void PutGetWhenRegionIsObject()
    {
      using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.True(cluster.Start());
        Assert.Equal(0, cluster.Gfsh
            .create()
            .region()
            .withName("exampleRegion")
            .withType("PARTITION")
            .execute());

        using var cacheFactory = CacheFactory<object, object>.Create()
                                     .SetProperty("log-level", "debug")
                                     .SetProperty("log-file", "geode_native.log");
        using var cache = cacheFactory.CreateCache();

        using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkObject(cache, "exampleRegion", RegionShortcut.Proxy);
      }
    }
  }
}
