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
using System.Reflection;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class ClusterTest : TestBase, IDisposable
    {
        public ClusterTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        public void Dispose()
        {

        }

        [Fact]
        public void ClusterStartTest()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
            }
        }

        [Fact]
        public void ClusterStartWithTwoServersTest()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 2))
            {
                Assert.True(cluster.Start());
            }
        }

        [Fact]
        public void ClusterStartWithSslTest()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                cluster.UseSSL = true;

                Assert.True(cluster.Start());
            }
        }
    }
}
