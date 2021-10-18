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
using Xunit;
using Xunit.Abstractions;
using Apache.Geode.Client.IntegrationTests;

namespace Apache.Geode.Client.IntegrationTests
{
  public class NetCoreTestFixture : IDisposable
  {
    internal Cluster cluster;
    public ITestOutputHelper Output { get; set; }

    public NetCoreTestFixture SetOutPut(ITestOutputHelper output)
    {
      Output = output;
      return this;
    }

    public void CreateCluster()
    {
      if (cluster == null)
      {
        cluster = new Cluster(Output, "NetCoreTests", 1, 1);

        Assert.True(cluster.Start());
        Assert.Equal(0, cluster.Gfsh
            .create()
            .region()
            .withName("exampleRegion")
            .withType("PARTITION")
            .execute());

        Assert.Equal(0, cluster.Gfsh
            .create()
            .region()
            .withName("authExampleRegion")
            .withType("PARTITION")
            .execute());
      }
    }

    public void Dispose()
    {
      // At this point, the last test has executed and the ITestOutputHelper is
      // no longer available. Hence set the output to null in the cluster.
      cluster.Gfsh.Output = null;
      cluster.Dispose();
      Output = null;
    }
  }
}
