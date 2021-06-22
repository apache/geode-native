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
using Apache.Geode.NetCore;
using Xunit;

namespace GemfireDotNetTest
{
    [Collection("Geode .net Core Collection")]
    public class PoolFactoryUnitTests
    {
        [Fact]
        public void TestPoolFactoryAddLocator()
        {
            using var cacheFactory = CacheFactory.Create()
                    .SetProperty("log-level", "none")
                    .SetProperty("log-file", "geode_native.log");
            using var cache = cacheFactory.CreateCache();
            using var poolManager = cache.PoolManager;
            using var poolFactory = poolManager.CreatePoolFactory();

            poolFactory.AddLocator("localhost", 10334);
        }
        
        [Fact]
        public void TestPoolFactoryCreatePool()
        {
            using var cacheFactory = CacheFactory.Create()
                    .SetProperty("log-level", "none")
                    .SetProperty("log-file", "geode_native.log");
            using var cache = cacheFactory.CreateCache();
            using var poolManager = cache.PoolManager;
            using var poolFactory = poolManager.CreatePoolFactory();

            poolFactory.AddLocator("localhost", 10334);
            using var pool = poolFactory.CreatePool("myPool");
        }

        [Fact]
        public void TestCreatePoolWithoutPoolManager()
        {
            using var cacheFactory = CacheFactory.Create();
            using var cache = cacheFactory.CreateCache();
            using var poolFactory = cache.PoolFactory;
            
            poolFactory.AddLocator("localhost", 10334);
            using var pool = poolFactory.CreatePool("myPool");
        }
    }
}

