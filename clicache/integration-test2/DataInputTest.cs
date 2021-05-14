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

                    var startingSize = Process.GetCurrentProcess().PrivateMemorySize64;

                    for (var i = 0; i < 1000; i++)
                    {
                        // Disable warning for 'useless assignment to local variable'.
                        // Declaring this variable in a using block will ensure the
                        // Dispose method is called when it goes out of scope, which
                        // is the whole point here, since that used to leak memory.
                        using (var di = new DataInput(buffer, __1M__, cache)) // lgtm[cs/useless-assignment-to-local]
                        {
                        }
                    }

                    // Disable warning for 'call to GC.Collect()'.  This test will fail
                    // with some regularity if we don't collect garbage prior to
                    // re-checking memory usage.
                    System.GC.Collect(); // lgtm[cs/call-to-gc]
                    System.GC.WaitForPendingFinalizers();
                    var endingSize = Process.GetCurrentProcess().PrivateMemorySize64;

                    //
                    // DataInput CLI object once had a leak of the internal buffer.  
                    // 1000 iterations each leaking 1MB should be a humongous leak, 
                    // so test for a < 10MB heap usage diff here, to allow some 
                    // wiggle room due to GC etc. and we're still certain we're not 
                    // leaking the buffer any more.
                    //
                    Assert.True(System.Math.Abs(endingSize - startingSize) < 10 * __1M__);

                    startingSize = Process.GetCurrentProcess().PrivateMemorySize64;

                    for (var i = 0; i < 1000; i++)
                    {
                        // Disable warning for 'useless assignment to local variable'.
                        // Declaring this variable in a using block will ensure the
                        // Dispose method is called when it goes out of scope, which
                        // is the whole point here, since that used to leak memory.
                        using (var di = new DataInput(buffer, cache)) // lgtm[cs/useless-assignment-to-local]
                        {
                        }
                    }

                    // Disable warning for 'call to GC.Collect()'.  This test will fail
                    // with some regularity if we don't collect garbage prior to
                    // re-checking memory usage.
                    System.GC.Collect(); // lgtm[cs/call-to-gc]
                    System.GC.WaitForPendingFinalizers();
                    endingSize = Process.GetCurrentProcess().PrivateMemorySize64;

                    //
                    // DataInput CLI object once had a leak of the internal buffer.  
                    // 1000 iterations each leaking 1MB should be a humongous leak, 
                    // so test for a < 10MB heap usage diff here, to allow some 
                    // wiggle room due to GC etc. and we're still certain we're not 
                    // leaking the buffer any more.
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
