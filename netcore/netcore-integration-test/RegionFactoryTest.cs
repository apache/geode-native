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

    private void doPutsAndGetsString(IRegion<string> region)
    {
      var fullname1 = "Robert Timmons";
      var fullname2 = "Sylvia Charles";

      region.Put(Username1, fullname1);
      region.Put(Username2, fullname2);

      var user1 = region.Get(Username1);
      var user2 = region.Get(Username2);

      Assert.Equal(user1, fullname1);
      Assert.Equal(user2, fullname2);
    }

    private void doPutsAndGetsInt32(IRegion<Int32> region)
    {
      region.Put(Username3, 779);
      var value3 = region.Get(Username3);

      Assert.Equal(779, value3);
    }

    private void doPutsAndGetsInt16(IRegion<Int16> region) {
      region.Put(Username4, (short)780);
      var value4 = region.Get(Username4);

      Assert.Equal(780, value4);
    }

    private void DoRemoves(IRegion<Int32> region) {
      region.Remove(Username1);
      region.Remove(Username2);

      //var hasUser1 = region.ContainsValueForKey(Username1);
      //var hasUser2 = region.ContainsValueForKey(Username2);

      //Assert.False(hasUser1);
      //Assert.False(hasUser2);
    }

    private void CreateRegionAndDoWorkString(IGeodeCache<string, string> cache, string regionName,
                                       RegionShortcut regionType)
    {
      using var regionFactory = cache.CreateRegionFactory(regionType);
      using var region = regionFactory.CreateRegion(regionName);

      doPutsAndGetsString(region);
      //DoRemoves(region);
    }

    private void CreateRegionAndDoWorkInt32(IGeodeCache<string, Int32> cache, string regionName,
                                       RegionShortcut regionType) {
      using var regionFactory = cache.CreateRegionFactory(regionType);
      using var region = regionFactory.CreateRegion(regionName);

      doPutsAndGetsInt32(region);
      //DoRemoves(region);
    }

    private void CreateRegionAndDoWorkInt16(IGeodeCache<string, Int16> cache, string regionName,
                                       RegionShortcut regionType)
    {
      using var regionFactory = cache.CreateRegionFactory(regionType);
      using var region = regionFactory.CreateRegion(regionName);

      doPutsAndGetsInt16(region);
      //DoRemoves(region);
    }

    [Fact]
    public void RegionFactoryCreateProxyRegionStringPutGetString()
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

        using var cacheFactory = CacheFactory<string, string>.Create()
                                     .SetProperty("log-level", "debug")
                                     .SetProperty("log-file", "geode_native.log");
        using var cache = cacheFactory.CreateCache();

        using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkString(cache, "exampleRegion", RegionShortcut.Proxy);
      }
    }

    [Fact]
    public void RegionFactoryCreateProxyRegionStringPutGetInt32() {
      using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.True(cluster.Start());
        Assert.Equal(0, cluster.Gfsh
            .create()
            .region()
            .withName("exampleRegion")
            .withType("PARTITION")
            .execute());

        using var cacheFactory = CacheFactory<string, Int32>.Create()
                                     .SetProperty("log-level", "debug")
                                     .SetProperty("log-file", "geode_native.log");
        using var cache = cacheFactory.CreateCache();

        using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkInt32(cache, "exampleRegion", RegionShortcut.Proxy);
      }
    }

    [Fact]
    public void RegionFactoryCreateProxyRegionStringPutInt16()
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

        using var cacheFactory = CacheFactory<string, Int16>.Create()
                                     .SetProperty("log-level", "debug")
                                     .SetProperty("log-file", "geode_native.log");
        using var cache = cacheFactory.CreateCache();

        using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkInt16(cache, "exampleRegion", RegionShortcut.Proxy);
      }
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

        using var cacheFactory = CacheFactory<string, Int32>.Create()
                                   .SetProperty("log-level", "debug")
                                   .SetProperty("log-file", "geode_native_with_auth.log");
      cacheFactory.AuthInitialize = new SimpleAuthInitialize();
      using var cache = cacheFactory.CreateCache();

      using var pool = cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWorkInt32(cache, "authExampleRegion", RegionShortcut.CachingProxy);
      }
    }
  }
}
