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
using System.Net.Cache;
using Xunit;

namespace Apache.Geode.Client {
  [Collection("Geode .net Core Collection")]
  public class PoolFactoryTest {
    [Fact]
    public void PoolFactoryAddLocatorAllObjectsNotNull() {
      using var cacheFactory = CacheFactory.Create()
                                   .SetProperty("log-level", "none")
                                   .SetProperty("log-file", "geode_native.log");
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);
      using var poolManager = cache.PoolManager;
      Assert.NotNull(poolManager);
      using var poolFactory = poolManager.CreatePoolFactory();
      Assert.NotNull(poolFactory);

      poolFactory.AddLocator("localhost", 10334);
    }

    [Fact]
    public void PoolFactoryCreatePoolAllObjectsNotNull() {
      using var cacheFactory = CacheFactory.Create()
                                   .SetProperty("log-level", "none")
                                   .SetProperty("log-file", "geode_native.log");
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);
      using var poolManager = cache.PoolManager;
      Assert.NotNull(poolManager);
      using var poolFactory = poolManager.CreatePoolFactory();
      Assert.NotNull(poolFactory);

      poolFactory.AddLocator("localhost", 10334);
      using var pool =
          poolFactory.CreatePool("myPool");  // lgtm[cs / useless - assignment - to - local]
      Assert.NotNull(pool);
    }

    [Fact]
    public void CreatePoolWithoutPoolManagerAllObjectsNotNull() {
      using var cacheFactory = CacheFactory.Create();
      Assert.NotNull(cacheFactory);
      using var cache = cacheFactory.CreateCache();
      Assert.NotNull(cache);
      using var poolFactory = cache.PoolFactory;
      Assert.NotNull(poolFactory);

      poolFactory.AddLocator("localhost", 10334);
      using var pool =
          poolFactory.CreatePool("myPool");  // lgtm[cs / useless - assignment - to - local]
      Assert.NotNull(pool);
    }
  }
}
