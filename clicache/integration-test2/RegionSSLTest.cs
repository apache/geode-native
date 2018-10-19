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

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class RegionSSLTest : TestBase, IDisposable
    {
        private readonly Cache cacheOne_;

        public RegionSSLTest()
        {
            var pathvar = Environment.GetEnvironmentVariable("PATH");

            var openSslPath = Environment.CurrentDirectory + Config.OpenSSLPath;

            if (!Directory.Exists(openSslPath))
            {
                throw new DirectoryNotFoundException("OpenSSL is a prerequisite for integration tests and the directory was not found.");
            }

            pathvar += ";" + openSslPath;

            var cryptoImplPath = Environment.CurrentDirectory + Config.CryptoImplPath;

            if (!File.Exists(cryptoImplPath + "\\cryptoImpl.dll"))
            {
                throw new System.IO.FileNotFoundException("cryptoImpl.dll was not found at " + cryptoImplPath);
            }

            pathvar += ";" + cryptoImplPath;

            Environment.SetEnvironmentVariable("PATH", pathvar);

            var cacheFactory = new CacheFactory();
            cacheFactory.Set("ssl-enabled", "true");
            cacheFactory.Set("ssl-keystore", Environment.CurrentDirectory + "\\ClientSslKeys\\client_keystore.password.pem");
            cacheFactory.Set("ssl-keystore-password", "gemstone");
            cacheFactory.Set("ssl-truststore", Environment.CurrentDirectory + "\\ClientSslKeys\\client_truststore.pem");

            cacheOne_ = cacheFactory.Create();
        }

        public void Dispose()
        {
            cacheOne_.Close();
        }

        [Fact]
        public void SslPutGetTest()
        {
            using (var cluster = new Cluster(CreateTestCaseDirectoryName(), 1, 1))
            {
                cluster.UseSSL = true;
                Assert.True(cluster.Start());
                Assert.Equal(cluster.Gfsh
                    .create()
                    .region()
                    .withName("testRegion1")
                    .withType("PARTITION")
                    .execute(), 0);

                cacheOne_.GetPoolFactory()
                    .AddLocator(cluster.Gfsh.LocatorBindAddress, cluster.Gfsh.LocatorPort)
                    .Create("default");

                var regionFactory = cacheOne_.CreateRegionFactory(RegionShortcut.PROXY)
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
