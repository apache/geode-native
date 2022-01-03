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
  public class RegionFactoryTest : TestBase, IClassFixture<NetCoreTestFixture>
  {
    NetCoreTestFixture fixture;

    public RegionFactoryTest(NetCoreTestFixture fixture, ITestOutputHelper testOutputHelper) : base(testOutputHelper)
    {
      this.fixture = fixture;
      this.fixture.Output = testOutputHelper;
      this.fixture.CreateCluster();
    }

    private const string Username1 = "rtimmons";
    private const string Username2 = "scharles";
    private const string Username3 = "michael";
    private const string Username4 = "george";

    private void createPool(IGeodeCache cache, int port) {
      using var poolManager = cache.PoolManager;
      using var poolFactory = poolManager.CreatePoolFactory().AddLocator("localhost", port);
      using var pool =
          poolFactory.CreatePool("myPool");  // lgtm[cs / useless - assignment - to - local]
    }

    private void doPutsAndGets(Region region) {
      IDictionary<object, object> dict = new Dictionary<object, object>
      {
        { "stringKey", "stringValue" },
        { "s32", 1 },
        { "s16", (short)1 },
        { "s64", (long)1 },
        { 77, 177 },
        { 78, (short)177 },
        { 79, (long)177 }
      };

      var array = new KeyValuePair<object, object>[dict.Count];
      dict.CopyTo(array, 0);

      var fullname1 = "Robert Timmons";
      var fullname2 = "Sylvia Charles";

      region.Put(Username1, fullname1);
      region.Put(Username2, fullname2);
      region.Put(Username3, 779);
      region.Put(Username4, (short)780);

      var user1 = region.GetString(Username1);
      var user2 = region.GetString(Username2);
      var value3 = region.Get<string, int>(Username3);
      var value4 = region.Get<string, short>(Username4);

      Assert.Equal(user1, fullname1);
      Assert.Equal(user2, fullname2);
      Assert.Equal(779, value3);
      Assert.Equal(780, value4);
    }

    private void DoRemoves(Region region) {
      region.Remove(Username1);
      region.Remove(Username2);

      var hasUser1 = region.ContainsValueForKey(Username1);
      var hasUser2 = region.ContainsValueForKey(Username2);

      Assert.False(hasUser1);
      Assert.False(hasUser2);
    }

    private void CreateRegionAndDoWork(IGeodeCache cache, string regionName,
                                       RegionShortcut regionType) {
      using var regionFactory = cache.CreateRegionFactory(regionType);
      using var region = regionFactory.CreateRegion(regionName);
      //using var region = regionFactory.CreateGenericRegion(regionName);
      //Region<int> region = regionFactory.CreateRegion<int>(regionName);

      doPutsAndGets(region);
      DoRemoves(region);
    }

    [Fact]
    public void RegionFactoryCreateProxyRegionStringPutGet() {
        using var cacheFactory = CacheFactory.Create()
                                     .SetProperty("log-level", "debug")
                                     .SetProperty("log-file", "geode_native.log");
        using var cache = cacheFactory.CreateCache();

        using var pool = fixture.cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWork(cache, "exampleRegion", RegionShortcut.Proxy);
    }

    [Fact]
    public void RegionFactoryCreateRegionStringPutGetWithAuthentication() {

        using var cacheFactory = CacheFactory.Create()
                                   .SetProperty("log-level", "debug")
                                   .SetProperty("log-file", "geode_native_with_auth.log");
      cacheFactory.AuthInitialize = new SimpleAuthInitialize();
      using var cache = cacheFactory.CreateCache();

      using var pool = fixture.cluster.ApplyLocators(cache.PoolFactory)
                    .CreatePool("myPool");

        CreateRegionAndDoWork(cache, "authExampleRegion", RegionShortcut.CachingProxy);
    }
  }
}
