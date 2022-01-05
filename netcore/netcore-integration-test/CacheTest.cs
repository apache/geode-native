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
using System.Net.Cache;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests {
  [Collection("Geode .Net Core Collection")]
  public class CacheTest {
    [Fact]
    public void ClientCacheGetPdxReadSerialized() {
      using var cacheFactory =
          CacheFactory<object, object>.Create()
              .SetProperty("log-level", "debug")
              .SetProperty("log-file", "TestClientCacheGetPdxReadSerialized.log");
      try {
        cacheFactory.PdxReadSerialized = true;
        using var cache = cacheFactory.CreateCache();

        Assert.True(cache.GetPdxReadSerialized());

        cacheFactory.PdxReadSerialized = false;
        using var otherCache = cacheFactory.CreateCache();

        Assert.False(otherCache.GetPdxReadSerialized());
      } catch (Exception e) {
        Console.WriteLine(e);
        throw;
      }
    }

    [Fact]
    public void ClientCacheGetPdxIgnoreUnreadFields() {
      using var cacheFactory = CacheFactory<object, object>.Create()
                                   .SetProperty("log-level", "none")
                                   .SetProperty("log-file", "geode_native.log");
      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();

      Assert.True(cache.GetPdxIgnoreUnreadFields());

      cacheFactory.PdxIgnoreUnreadFields = false;
      using var otherCache = cacheFactory.CreateCache();
      Assert.False(otherCache.GetPdxIgnoreUnreadFields());
    }

    [Fact]
    public void ClientCacheGetPoolManagerObjectsNotNull() {
      using var cacheFactory = CacheFactory<object, object>.Create()
                                   .SetProperty("log-level", "none")
                                   .SetProperty("log-file", "geode_native.log");

      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();  // lgtm [cs/useless-assignment-to-local]
      Assert.NotNull(cache);
      using var poolManager = cache.PoolManager;
      Assert.NotNull(poolManager);
    }

    [Fact]
    public void ClientCacheCreateRegionFactoryObjectsNotNull() {
      using var cacheFactory = CacheFactory<object, object>.Create()
                                   .SetProperty("log-level", "none")
                                   .SetProperty("log-file", "geode_native.log");
      Assert.NotNull(cacheFactory);
      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);
      using var regionFactory = cache.CreateRegionFactory(
          RegionShortcut.Proxy);  // lgtm[cs / useless - assignment - to - local]
      Assert.NotNull(regionFactory);
    }

    [Fact]
    public void ClientCacheGetName() {
      using var cacheFactory = CacheFactory<object, object>.Create().SetProperty("log-level", "none");
      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);

      var cacheName = cache.Name;
      Assert.NotEqual(cacheName, String.Empty);
    }

    [Fact]
    public void ClientCacheClose() {
      using var cacheFactory = CacheFactory<object, object>.Create().SetProperty("log-level", "none");
      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);

      Assert.False(cache.Closed);
      cache.Close();
      Assert.True(cache.Closed);
    }

    [Fact]
    public void ClientCacheCloseWithKeepalive() {
      using var cacheFactory = CacheFactory<object, object>.Create().SetProperty("log-level", "none");
      cacheFactory.PdxIgnoreUnreadFields = true;
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);

      Assert.False(cache.Closed);
      cache.Close(true);
      Assert.True(cache.Closed);

      using var otherCache = cacheFactory.CreateCache();
      Assert.NotNull(otherCache);

      Assert.False(otherCache.Closed);
      otherCache.Close(false);
      Assert.True(otherCache.Closed);
    }
  }
}
