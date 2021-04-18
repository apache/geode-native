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
                        Assert.True(.8 < ratio && ratio < 1.3);
                }
            }
        }

        [Fact(Skip = "Need a heuristic to filter out GC effect.")]
        public void VerifyNoLeakedMemoryPutAllGetAll()
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

                const int numEntries = 1000;
                const int numIterations = 100;
                const int stringLength = 1000;


                Int64[] workingSetBefore = new Int64[numIterations];
                Int64[] workingSetAfter = new Int64[numIterations];
                string value = new string('*', stringLength);

                for (int i = 0; i < numIterations; i++)
                {
                    workingSetBefore[i] = (Int64)Process.GetCurrentProcess().WorkingSet64;

                    using (var cache = new CacheFactory()
                        .Set("log-level", "none")
                        .Create())
                    {

                        cluster.ApplyLocators(cache.GetPoolFactory()).Create("default");

                        var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                            .SetPoolName("default");

                        var region = regionFactory.Create<object, object>("testRegion");

                        IDictionary<object, object> collection = new Dictionary<object, object>();

                        for (int n = 0; n < numEntries; n++)
                        {
                            collection.Add(n, value);
                        }

                        region.PutAll(collection);

                        List<Object> keys = new List<Object>();
                        for (int item = 0; item < numEntries; item++)
                        {
                            Object K = item;
                            keys.Add(K);
                        }
                        Dictionary<Object, Object> values = new Dictionary<Object, Object>();
                        region.GetAll(keys.ToArray(), values, null, true);

                        cache.Close();
                    }

                    // Because the are many small leaks in the native client, the
                    // heuristic below is based on empirical data on across many iterations
                    // to ensure there are no huge mmemory leaks. After a warmup,
                    // measurements are taken across several iterations
                    // to see the trend across many garbage collect cycles.
                
                    //if (i > 10)
                    //    Assert.True(.9 < ratio && ratio < 1.1);

                    workingSetAfter[i] = (Int64)Process.GetCurrentProcess().WorkingSet64;

                }

                Int64[] diff = new Int64[numIterations];
                for (int i=0; i<numIterations; i++)
                {
                    diff[i] = workingSetAfter[i] - workingSetBefore[i];
                }

                Console.WriteLine("diff[99] = {0}", diff[numIterations-1]);
            }
        }
    }
}