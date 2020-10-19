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
using System.Collections;
using System.Collections.Generic;
using Xunit.Abstractions;
using System.Threading.Tasks;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class SNITests : TestBase, IDisposable
    {
        string currentWorkingDirectory;
        Process dockerProcess;
        private readonly Cache cache_;

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

            var dc = Process.Start(@"docker-compose.exe", "-f " + Config.SniConfigPath + "/docker-compose.yml" + " up -d");
            dc.WaitForExit();

            var d = Process.Start(@"docker.exe", "exec -t geode gfsh run --file=/geode/scripts/geode-starter.gfsh");
            d.WaitForExit();
        }

        public void Dispose()
        {
            CleanupDocker();
        }

        private void CleanupDocker()
        {
            var dockerComposeProc = Process.Start(@"docker-compose.exe", "-f " + Config.SniConfigPath + "/docker-compose.yml" + " stop");
            dockerComposeProc.WaitForExit();

            var dockerProc = Process.Start(@"docker.exe", "system prune -f");
            dockerProc.WaitForExit();
        }

        private string RunDockerCommand(string dockerCommand)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.RedirectStandardOutput = true;
            startInfo.UseShellExecute = false;
            startInfo.FileName = @"docker.exe";
            startInfo.Arguments = dockerCommand;
            dockerProcess = Process.Start(startInfo);
            String rVal = dockerProcess.StandardOutput.ReadToEnd();
            dockerProcess.WaitForExit();
            return rVal;
        }

        private int ParseProxyPort(string proxyString)
        {
            int colonPosition = proxyString.IndexOf(":");
            string portNumberString = proxyString.Substring(colonPosition + 1);
            return Int32.Parse(portNumberString);
        }

        private Task PutAsync(IRegion<string, string> region, string key, string value)
        {
            return Task.Run(() => region.Put(key, value));
        }

        [Fact]
        public void ConnectViaProxy()
        {
            var portString = RunDockerCommand("port haproxy");
            var portNumber = ParseProxyPort(portString);

            cache_.GetPoolManager()
                .CreateFactory()
                .SetSniProxy("localhost", portNumber)
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
            var portString = RunDockerCommand("port haproxy");
            var portNumber = ParseProxyPort(portString);

            cache_.GetPoolManager()
                .CreateFactory()
                .SetSniProxy("localhost", portNumber)
                .AddLocator("locator-maeve", 10334)
                .Create("pool");

            var region = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                              .SetPoolName("pool")
                              .Create<string, string>("jellyfish");

            var rVal = RunDockerCommand("pause haproxy");

            Task putTask = PutAsync(region, "1", "one");

            // Insure the put times out (default is 15 seconds).
            System.Threading.Thread.Sleep(16 * 1000);

            rVal = RunDockerCommand("unpause haproxy");

            putTask.Wait();

            var value = region.Get("1");

            Assert.Equal("one", value);
            cache_.Close();
        }

        [Fact]
        public void NewDropProxy()
        {
            var portString = RunDockerCommand("port haproxy");
            var portNumber = ParseProxyPort(portString);

            cache_.GetPoolManager()
                .CreateFactory()
                .SetReadTimeout(new TimeSpan(0,0,5))
                .SetRetryAttempts(2)
                .SetSniProxy("localhost", portNumber)
                .AddLocator("locator-maeve", 10334)
                .Create("pool");

            var region = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                              .SetPoolName("pool")
                              .Create<string, string>("jellyfish");

            region.Put("1", "one");
            var value = region.Get("1");

            var rVal = RunDockerCommand("pause haproxy");


            Assert.ThrowsAny<Exception>(() =>
            {
                value = region.Get("1");
                Console.WriteLine("Shouldn't be able to retrieve any data while proxy is paused");
            });

            rVal = RunDockerCommand("unpause haproxy");
            value = region.Get("1");

            Assert.Equal("one", value);
            cache_.Close();
        }
    }
}


//It would be great to have this test:

    //bring up the system/cluster
    //do a cache ops (put/get)
    //drop the proxy
    //attempt cache ops - wait for exception (EXPECTEXCEPTION in gtest or similar...)
//bring proxy back up
//then do verify cache ops (put/get)