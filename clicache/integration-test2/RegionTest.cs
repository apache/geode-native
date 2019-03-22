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
using System.IO;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class RegionTest : TestBase
    {
        public RegionTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void PutOnOneCacheGetOnAnotherCache()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh
                    .create()
                    .region()
                    .withName("testRegion1")
                    .withType("PARTITION")
                    .execute());

                var cacheFactory = new CacheFactory()
                    .Set("log-level", "none");

                var cacheOne = cacheFactory.Create();
                try
                {
                    cluster.ApplyLocators(cacheOne.GetPoolFactory()).Create("default");

                    var cacheTwo = cacheFactory.Create();
                    try
                    {
                        cluster.ApplyLocators(cacheTwo.GetPoolFactory()).Create("default");

                        var regionFactory1 = cacheOne.CreateRegionFactory(RegionShortcut.PROXY)
                            .SetPoolName("default");
                        var regionFactory2 = cacheTwo.CreateRegionFactory(RegionShortcut.PROXY)
                            .SetPoolName("default");

                        var regionForCache1 = regionFactory1.Create<string, string>("testRegion1");
                        var regionForCache2 = regionFactory2.Create<string, string>("testRegion1");

                        const string key = "hello";
                        const string expectedResult = "dave";

                        regionForCache1.Put(key, expectedResult);
                        var actualResult = regionForCache2.Get(key);

                        Assert.Equal(expectedResult, actualResult);
                    }
                    finally
                    {
                        cacheTwo.Close();
                    }
                }
                finally
                {
                    cacheOne.Close();
                }
            }
        }
    }
}
