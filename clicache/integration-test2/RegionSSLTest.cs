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
    public class RegionSSLTest : TestBase, IDisposable
    {
        private readonly Cache cache_;

        public RegionSSLTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
            var cacheFactory = new CacheFactory();
            cacheFactory.Set("log-level", "none");
            cacheFactory.Set("ssl-enabled", "true");
            cacheFactory.Set("ssl-keystore", Config.SslClientKeyPath + @"/client_keystore_chained.pem");
            cacheFactory.Set("ssl-keystore-password", "apachegeode");
            cacheFactory.Set("ssl-truststore", Config.SslClientKeyPath + @"/client_truststore_chained_root.pem");

            cache_ = cacheFactory.Create();
        }

        public void Dispose()
        {
            cache_.Close();
        }

        [Fact]
        public void SslPutGetTest()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                cluster.UseSSL = true;
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh
                    .create()
                    .region()
                    .withName("testRegion1")
                    .withType("PARTITION")
                    .execute());

                cluster.ApplyLocators(cache_.GetPoolFactory()).Create("default");

                var regionFactory = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                            .SetPoolName("default");

                var region = regionFactory.Create<string, string>("testRegion1");

                const string key = "hello";
                const string expectedResult = "dave";

                region.Put(key, expectedResult);
                var actualResult = region.Get(key);

                Assert.Equal(expectedResult, actualResult);
            }
        }
    }
}
