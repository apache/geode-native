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

using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class FunctionExecutionTest : TestBase
    {
        public FunctionExecutionTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void MultiGetFunctionExecutionWithFilter()
        {
            const int expectedFilteredCount = 34;
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute());
                Assert.Equal(0, cluster.Gfsh.create().region()
                    .withName("testRegion1")
                    .withType("PARTITION")
                    .execute());

                var cache = cluster.CreateCache();
                var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("default")
                    .Create<object, object>("testRegion1");

                for (var i = 0; i < 230; i++)
                {
                  region["KEY--" + i] = "VALUE--" + i;
                }

                const bool args = true;

                var oddKeyFilter = new object[17];
                var j = 0;
                for (var i = 0; i < 34; i++)
                {
                  if (i % 2 == 0) continue;
                  oddKeyFilter[j] = "KEY--" + i;
                  j++;
                }

                var exc = FunctionService<List<object>>.OnRegion(region);
                var rc = exc.WithArgs(args).WithFilter(oddKeyFilter).Execute("MultiGetFunction");
                var executeFunctionResult = rc.GetResult();
                var resultList = executeFunctionResult.SelectMany(item => item).ToList();

                Assert.Equal(expectedFilteredCount, resultList.Count);

            }
        }

        [Fact]
        public void MultiGetIFunctionExecutionWithArgs()
        {
            const int expectedResultCount = 17;
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute());
                Assert.Equal(0, cluster.Gfsh.create().region()
                    .withName("partition_region")
                    .withType("PARTITION")
                    .execute());

                var cache = cluster.CreateCache();

                var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("default")
                    .Create<object, object>("partition_region");

                for (var i = 0; i < 230; i++)
                {
                    region["KEY--" + i] = "VALUE--" + i;
                }

                var oddKeyArgs = new ArrayList();
                for (var i = 0; i < 34; i++)
                {
                    if (i % 2 == 0) continue;
                    oddKeyArgs.Add("KEY--" + i);
                }

                var exc = FunctionService<List<object>>.OnRegion(region);
                var rc = exc.WithArgs(oddKeyArgs).Execute("MultiGetFunctionI");
                var executeFunctionResult = rc.GetResult();
                var resultList = executeFunctionResult.SelectMany(item => item).ToList();

                Assert.Equal(expectedResultCount, resultList.Count);
            }
        }

    [Fact(Skip = "Waiting for a fix from Geode server.")]
    public void FunctionReturnsObjectWhichCantBeDeserializedOnServer()
    {
      using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 2))
      {
        Assert.True(cluster.Start());
        Assert.Equal(0, cluster.Gfsh.create().region()
            .withName("region")
            .withType("REPLICATE")
            .execute());
        Assert.Equal(0, cluster.Gfsh.deploy()
            .withJar(Config.JavaobjectJarPath)
            .execute());

        var cache = cluster.CreateCache();
        cache.GetPoolFactory().AddLocator("localhost", 10334).Create("pool");
        var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
            .SetPoolName("pool")
            .Create<object, object>("region");

        var exc = FunctionService<List<object>>.OnRegion(region);
        Assert.Throws<FunctionExecutionException>(() => exc.Execute("executeFunction_SendObjectWhichCantBeDeserialized"));
      }
    }
  }
}
