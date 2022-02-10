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
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GarbageCollectCache : TestBase
    {
        public GarbageCollectCache(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void VerifyNoLeakedThreads()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh
                    .create()
                    .region()
                    .withName("testRegion")
                    .withType("PARTITION")
                    .execute());

                for (int i=0; i<25; i++)
                {
                    var ncThreadsBefore = Process.GetCurrentProcess().Threads.Count;

                    using (var cache = new CacheFactory()
                        .Set("log-level", "none")
                        .Create())
                    {

                        cluster.ApplyLocators(cache.GetPoolFactory()).Create("default");

                        var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                            .SetPoolName("default");

                        var region = regionFactory.Create<string, string>("testRegion");

                        const string key = "hello";
                        const string expectedResult = "dave";

                        region.Put(key, expectedResult);
                        var actualResult = region.Get(key);
                        Assert.Equal(expectedResult, actualResult);

                        cache.Close();
                    }

                    var ncThreadsAfter = Process.GetCurrentProcess().Threads.Count;
                    var ratio = ncThreadsBefore / (double)ncThreadsAfter;

                    // Because the number of threads in the process depends on
                    // the number of cores, debug vs release, whether the GC thread
                    // is running, etc., a robust test is to check whether the ratio
                    // of before and after threads is close to one. Also skipping the
                    // first couple of iterations avoids threads related to test 
                    // environment startup.
                    if (i > 5)
                    {
                        string error = "ncThreadsBefore = " + ncThreadsBefore.ToString() +
                            ", ncThreadsAfter = " + ncThreadsAfter.ToString();
                        Assert.True((.5 < ratio && ratio < 2.0), error);
                    }
                }
            }
        }
    }
}
