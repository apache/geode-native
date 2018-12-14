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
using System.IO;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
  [Trait("Category", "Integration")]
    public class FunctionExecutionTest : TestBase
    {
        [Fact]
        public void MultiGetFunctionExecutionWithFilter()
        {
            int expectedFilteredCount = 34;
            using (var cluster = new Cluster(CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.Equal(cluster.Start(), true);
                Assert.Equal(cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute(), 0);
                Assert.Equal(cluster.Gfsh
                    .create()
                    .region()
                    .withName("testRegion1")
                    .withType("PARTITION")
                    .execute(), 0);

                var cacheFactory = new CacheFactory();
                var cacheOne = cacheFactory.Create();
                cacheOne.GetPoolFactory()
                    .AddLocator(cluster.Gfsh.LocatorBindAddress, cluster.Gfsh.LocatorPort)
                    .Create("pool");
                var regionFactory = cacheOne.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("pool");
                var region = regionFactory.Create<object, object>("testRegion1");

                for (int i = 0; i < 230; i++)
                {
                  region["KEY--" + i] = "VALUE--" + i;
                }

                object args = true;
              

                Object[] oddKeyFilter = new Object[17];
                int j = 0;
                for (int i = 0; i < 34; i++)
                {
                  if (i % 2 == 0) continue;
                  oddKeyFilter[j] = "KEY--" + i;
                  j++;
                }
                Apache.Geode.Client.Execution<object> exc = Client.FunctionService<object>.OnRegion<object, object>(region);
                Client.IResultCollector<object> rc = exc.WithArgs<object>(args).WithFilter<object>(oddKeyFilter).Execute("MultiGetFunction");
                ICollection<object> executeFunctionResult = rc.GetResult();
                List<object> resultList = new List<object>();

                foreach (List<object> item in executeFunctionResult)
                {
                  foreach (object item2 in item)
                  {
                    resultList.Add(item2);
                  }
                }
                Assert.True(resultList.Count == expectedFilteredCount, "result count check failed");

            }
        }

    [Fact]
    public void MultiGetIFunctionExecutionWithArgs()
    {
      int expectedResultCount = 17;
      using (var cluster = new Cluster(CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.Equal(cluster.Start(), true);
        Assert.Equal(cluster.Gfsh.deploy()
            .withJar(Config.JavaobjectJarPath)
            .execute(), 0);
        Assert.Equal(cluster.Gfsh
            .create()
            .region()
            .withName("partition_region")
            .withType("PARTITION")
            .execute(), 0);

        var cacheFactory = new CacheFactory();
        var cacheOne = cacheFactory.Create();
        cacheOne.GetPoolFactory()
            .AddLocator(cluster.Gfsh.LocatorBindAddress, cluster.Gfsh.LocatorPort)
            .Create("pool");
        var regionFactory = cacheOne.CreateRegionFactory(RegionShortcut.PROXY)
            .SetPoolName("pool");
        var region = regionFactory.Create<object, object>("partition_region");

        for (int i = 0; i < 230; i++)
        {
          region["KEY--" + i] = "VALUE--" + i;
        }

        object args = true;
        Object[] oddKeyFilter = new Object[17];
        int j = 0;
        for (int i = 0; i < 34; i++)
        {
          if (i % 2 == 0) continue;
          oddKeyFilter[j] = "KEY--" + i;
          j++;
        }

        ArrayList oddKeyArgs = new ArrayList();
        for (int i = 0; i < oddKeyFilter.Length; i++)
        {
          oddKeyArgs.Add(oddKeyFilter[i]);
        }
        Apache.Geode.Client.Execution<object> exc = Client.FunctionService<object>.OnRegion<object, object>(region);
        Client.IResultCollector<object> rc = exc.WithArgs<ArrayList>(oddKeyArgs).Execute("MultiGetFunctionI");
        ICollection<object> executeFunctionResult = rc.GetResult();
        List<object> resultList = new List<object>();

        foreach (List<object> item in executeFunctionResult)
        {
          foreach (object item2 in item)
          {
            resultList.Add(item2);
          }
        }
        Assert.True(resultList.Count == expectedResultCount, "result count check failed");

      }
    }
  }
}
