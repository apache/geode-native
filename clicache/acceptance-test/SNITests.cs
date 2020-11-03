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
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;

using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class SNITests : TestBase, IDisposable
    {
        string currentWorkingDirectory;
        private readonly Cache cache_;
        private int proxyPort = -1;

        public SNITests(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
            CleanupDocker();

            currentWorkingDirectory = Directory.GetCurrentDirectory();
            var clientTruststore = Config.SslClientKeyPath + @"/truststore_sni.pem";

            var cacheFactory = new CacheFactory();
            cacheFactory.Set("log-level", "none");
            cacheFactory.Set("log-file", "c:/temp/SNITest-csharp.log");
            cacheFactory.Set("ssl-enabled", "true");
            cacheFactory.Set("ssl-truststore", clientTruststore);

            cache_ = cacheFactory.Create();

            RunProcess("docker-compose",
                "-f " + Config.SniConfigPath + "/docker-compose.yml" +
                " up -d");
            RunProcess("docker",
                "exec -t geode " +
                "gfsh run --file=/geode/scripts/geode-starter.gfsh");
        }

        public void Dispose()
        {
            CleanupDocker();
        }

        private void CleanupDocker()
        {
            RunProcess("docker", "stop geode");
            RunProcess("docker", "stop haproxy");
            RunProcess("docker", "container prune -f");
        }

        private string RunProcess(string processFile, string processArgs)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.RedirectStandardOutput = true;
            startInfo.UseShellExecute = false;
            startInfo.FileName = processFile;
            startInfo.Arguments = processArgs;
            Process process = Process.Start(startInfo);
            String rVal = process.StandardOutput.ReadToEnd();
            process.WaitForExit();
            return rVal;
        }

        private int ParseProxyPort(string proxyString)
        {
            int colonPosition = proxyString.IndexOf(":");
            string portNumberString = proxyString.Substring(colonPosition + 1);
            return Int32.Parse(portNumberString);
        }

        [Fact]
        public void ConnectViaProxy()
        {
            var portString = RunProcess("docker", "port haproxy");
            proxyPort = ParseProxyPort(portString);

            cache_.GetPoolManager()
                .CreateFactory()
                .SetSniProxy("localhost", proxyPort)
                .AddLocator("locator-maeve", 10334)
                .Create("pool");

            var region = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                              .SetPoolName("pool")
                              .Create<string, string>("jellyfish");

            region.Put("1", "one");
            var value = region.Get("1");

            Assert.Equal("one", value);
            cache_.Close();
        }

        [Fact]
        public void ConnectionWithoutProxyFails()
        {
            cache_.GetPoolManager()
                .CreateFactory()
                .AddLocator("localhost", 10334)
                .Create("pool");

            var region = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                              .SetPoolName("pool")
                              .Create<string, string>("region");

            Assert.Throws<NotConnectedException>(() => region.Put("1", "one"));
        }

        [Fact]
        public void DropProxy()
        {
            var portString = RunProcess("docker", "port haproxy");
            proxyPort = ParseProxyPort(portString);

            cache_.GetPoolManager()
                .CreateFactory()
                .SetSniProxy("localhost", proxyPort)
                .AddLocator("locator-maeve", 10334)
                .Create("pool");

            var region = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                              .SetPoolName("pool")
                              .Create<string, string>("jellyfish");

            region.Put("1", "one");
            var value = region.Get("1");

            Assert.Equal("one", value);

            RunProcess("docker", "stop haproxy");
            RunProcess("docker", "container prune -f");

            Assert.Throws<NotConnectedException>(() =>
            {
                region.Put("2", "two");
                value = region.Get("2");
            });

            string startProxyArgs =
                "-f " + Config.SniConfigPath + "/docker-compose.yml " +
                "run -d --name haproxy " +
                "--publish " + proxyPort.ToString() + ":15443 haproxy";
            RunProcess("docker-compose", startProxyArgs);

            region.Put("3", "three");
            value = region.Get("3");
            Assert.Equal("three", value);

            cache_.Close();
        }
    }
}