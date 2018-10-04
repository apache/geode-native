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
using System.Collections.Generic;
using Xunit;

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GfshTest : IDisposable
    {
        public void Dispose()
        {
        
        }

        [Fact]
        public void InstantiateGfshClassesTest()
        {
            var gfsh = new GfshExecute();
            var start = gfsh.start();
            Assert.NotNull(start);

            var stop = gfsh.stop();
            Assert.NotNull(stop);

            var create = gfsh.create();
            Assert.NotNull(create);

            var shutdown = gfsh.shutdown();
            Assert.NotNull(shutdown);

            var configurePdx = gfsh.configurePdx();
            Assert.NotNull(configurePdx);
        }

        [Fact]
        public void StartLocatorStringsTest()
        {
            var locator = new GfshExecute()
                .start()
                .locator();
            var s = locator.ToString();
            Assert.True(s.Equals("Command: start locator"));

            locator = new GfshExecute().start().locator()
                .withName("name")
                .withDir("dir")
                .withBindAddress("address")
                .withPort(420)
                .withJmxManagerPort(1111)
                .withHttpServicePort(2222)
                .withLogLevel("fine")
                .withMaxHeap("someHugeAmount")
                .withConnect(false)
                .withSslEnabledComponents(new List<string> { "locator", "jmx" })
                .withSslKeystore("some/path/keystore.jks")
                .withSslKeystorePassword("password")
                .withSslTruststore("some/path/truststore.jks")
                .withSslTruststorePassword("password");
            s = locator.ToString();
            Assert.Equal(s, "Command: start locator --name=name --dir=dir " + 
                "--http-service-port=2222 --log-level=fine --max-heap=someHugeAmount " +
                "--connect=false --J=-Dgemfire.ssl-enabled-components=locator,jmx " +
                "--J=-Dgemfire.ssl-keystore=some/path/keystore.jks --J=-Dgemfire.ssl-keystore-password=password " +
                "--J=-Dgemfire.ssl-truststore=some/path/truststore.jks --J=-Dgemfire.ssl-truststore-password=password");
        }

        [Fact]
        public void StartServerStringsTest()
        {
            var server = new GfshExecute()
                .start()
                .server();
            var s = server.ToString();
            Assert.True(s.Equals("Command: start server"));

            server = new GfshExecute()
                .start()
                .server()
                .withName("server")
                .withDir("someDir")
                .withBindAddress("someAddress")
                .withPort(1234)
                .withLocators("someLocator")
                .withLogLevel("debug")
                .withMaxHeap("1.21gigabytes")
                .withSslEnabledComponents(new List<string> { "server", "locator", "jmx" })
                .withSslKeystore("some/path/keystore.jks")
                .withSslKeystorePassword("password")
                .withSslTruststore("some/path/truststore.jks")
                .withSslTruststorePassword("password");
            s = server.ToString();
            Assert.Equal(s, "Command: start server --name=server " +
                "--dir=someDir --server-port=1234 --locators=someLocator --log-level=debug " +
                "--max-heap=1.21gigabytes --J=-Dgemfire.ssl-enabled-components=server,locator,jmx " +
                "--J=-Dgemfire.ssl-keystore=some/path/keystore.jks --J=-Dgemfire.ssl-keystore-password=password " +
                "--J=-Dgemfire.ssl-truststore=some/path/truststore.jks --J=-Dgemfire.ssl-truststore-password=password");
        }

        [Fact]
        public void StopLocatorStringsTest()
        {
            var locator = new GfshExecute()
                .stop()
                .locator();
            var s = locator.ToString();
            Assert.True(s.Equals("Command: stop locator"));

            locator = new GfshExecute().stop().locator()
                .withName("name")
                .withDir("dir");
            s = locator.ToString();
            Assert.True(s.Equals("Command: stop locator --name=name --dir=dir"));
        }

        [Fact]
        public void StopServerStringsTest()
        {
            var server = new GfshExecute()
                .stop()
                .server();
            var s = server.ToString();
            Assert.True(s.Equals("Command: stop server"));

            server = new GfshExecute()
                .stop()
                .server()
                .withName("server")
                .withDir("someDir");
            s = server.ToString();
            Assert.True(s.Equals("Command: stop server --name=server --dir=someDir"));
        }

        [Fact]
        public void CreateRegionStringsTest()
        {
            var region = new GfshExecute()
                .create()
                .region();
            var s = region.ToString();
            Assert.True(s.Equals("Command: create region"));

            region = new GfshExecute()
                .create()
                .region()
                .withName("region")
                .withType("PARTITION");
            s = region.ToString();
            Assert.True(s.Equals("Command: create region --name=region --type=PARTITION"));
        }

        [Fact]
        public void ShutdownStringsTest()
        {
            var shutdown = new GfshExecute()
                .shutdown();
            var s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown"));

            shutdown = new GfshExecute()
                .shutdown()
                .withIncludeLocators(true);
            s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown --include-locators=true"));

            shutdown = new GfshExecute()
                .shutdown()
                .withIncludeLocators(false);
            s = shutdown.ToString();
            Assert.True(s.Equals("Command: shutdown --include-locators=false"));
        }

        [Fact]
        public void ConfigurePdxStringsTest()
        {
            var configurePdx = new GfshExecute()
                .configurePdx();
            var s = configurePdx.ToString();
            Assert.Equal(s, "Command: configure pdx");

            configurePdx = new GfshExecute()
                .configurePdx()
                .withReadSerialized(true);
            s = configurePdx.ToString();
            Assert.Equal(s, "Command: configure pdx --read-serialized=true");

            configurePdx = new GfshExecute()
                .configurePdx()
                .withReadSerialized(false);
            s = configurePdx.ToString();
            Assert.Equal(s, "Command: configure pdx --read-serialized=false");
        }
    }
}
