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
using System.Diagnostics;
using System.IO;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class DataInputTest : TestBase
    {
        public DataInputTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        const int __1K__ = 1024;
        const int __1M__ = __1K__ * __1K__;

        [Fact]
        public void CreateDisposeAndCheckForMemoryLeaks()
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

                var cache = cacheFactory.Create();
                try
                {
                    cluster.ApplyLocators(cache.GetPoolFactory()).Create("default");
                    var buffer = new Byte[__1M__];

                    Process currentProc = Process.GetCurrentProcess();
                    var startingSize = currentProc.PrivateMemorySize64;

                    for (var i = 0; i < 1000; i++)
                    {
                        using (var di = new DataInput(buffer, cache))
                        {

                        }
                    }

                    var endingSize = currentProc.PrivateMemorySize64;
                    //
                    // DataInput CLI object once had a leak of the internal buffer if you
                    // used the 2-parameter ctor we used above.  1000 iterations each leaking
                    // 1MB should be a humongous leak, so test for a < 10MB heap usage diff
                    // here, to allow some wiggle room due to GC etc. and we're still certain
                    // we're not leaking the buffer any more.
                    //
                    Assert.True(System.Math.Abs(endingSize - startingSize) < 10 * __1M__);
                }
                finally
                {
                    cache.Close();
                }
            }
        }
    }
}
