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
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{

    [Trait("Category", "Integration")]
    public class GfshTest : TestBase
    {
        public GfshTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void InstantiateGfshClassesTest()
        {
            var gfsh = new GfshExecute(output);
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
            var Gfsh = new GfshExecute(output);

            var locator = Gfsh
                .start()
                .locator();
            var s = locator.ToString();
            Assert.Equal("start locator", s);

            locator = Gfsh
                .start()
                .locator()
                .withName("name")
                .withDir("dir")
                .withBindAddress("address")
                .withPort(420)
                .withJmxManagerPort(1111)
                .withHttpServicePort(2222)
                .withLogLevel("fine")
                .withMaxHeap("someHugeAmount")
                .withConnect(false)
                .withSslEnableComponents("locator,jmx")
                .withSslKeyStore("some/path/keystore.jks")
                .withSslKeyStorePassword("password1")
                .withSslTrustStore("some/path/truststore.jks")
                .withSslTrustStorePassword("password2")
                .withDebugAgent("someAddress");
            s = locator.ToString();
            var withAttachPortRemoved = s.Substring(0, s.LastIndexOf(":", s.Length-1, 6));
            var startLocatorCommandWithoutDebugAgentPort =
                s.Substring(0, s.LastIndexOf(":", s.Length - 1, 6));
            Assert.Equal("start locator --name=name --dir=dir --bind-address=address --port=420 " +
              "--J=-Dgemfire.jmx-manager-port=1111 --http-service-port=2222 --log-level=fine --max-heap=someHugeAmount " +
              "--connect=false --J=-Dgemfire.ssl-enabled-components=locator,jmx " +
              "--J=-Dgemfire.ssl-keystore=some/path/keystore.jks --J=-Dgemfire.ssl-keystore-password=password1 " +
              "--J=-Dgemfire.ssl-truststore=some/path/truststore.jks --J=-Dgemfire.ssl-truststore-password=password2 " +
              "--J=-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address=someAddress",
              startLocatorCommandWithoutDebugAgentPort);
    }

        [Fact]
        public void StartServerStringsTest()
        {
            var Gfsh = new GfshExecute(output);

            var currentDir = Environment.CurrentDirectory;

            var server = Gfsh
                .start()
                .server();
            var s = server.ToString();
            Assert.Equal("start server", s);

            server = Gfsh
                .start()
                .server()
                .withName("server")
                .withDir("someDir")
                .withBindAddress("someAddress")
                .withPort(1234)
                .withLocators("someLocator")
                .withLogLevel("debug")
                .withMaxHeap("1.21gigabytes")
                .withSslEnableComponents("server,locator,jmx")
                .withSslKeyStore("some/path/keystore.jks")
                .withSslKeyStorePassword("password1")
                .withSslTrustStore("some/path/truststore.jks")
                .withSslTrustStorePassword("password2")
                .withDebugAgent("someAddress");
            s = server.ToString();
            var startServerCommandWithoutDebugAgentPort =
              s.Substring(0, s.LastIndexOf(":", s.Length-1, 6));
            Assert.Equal("start server --name=server " +
                "--dir=someDir " +
                "--bind-address=someAddress " +
                "--server-port=1234 " +
                "--locators=someLocator " +
                "--log-level=debug " +
                "--max-heap=1.21gigabytes " +
                "--J=-Dgemfire.ssl-enabled-components=server,locator,jmx " +
                "--J=-Dgemfire.ssl-keystore=some/path/keystore.jks " +
                "--J=-Dgemfire.ssl-keystore-password=password1 " +
                "--J=-Dgemfire.ssl-truststore=some/path/truststore.jks " +
                "--J=-Dgemfire.ssl-truststore-password=password2 " +
                "--J=-agentlib:jdwp=transport=dt_socket,server=y,suspend=n," +
                    "address=someAddress",
                startServerCommandWithoutDebugAgentPort);
        }

        [Fact]
        public void StopLocatorStringsTest()
        {
            var locator = new GfshExecute(output)
                .stop()
                .locator();
            var s = locator.ToString();
            Assert.Equal("stop locator", s);

            locator = new GfshExecute(output).stop().locator()
                .withName("name")
                .withDir("dir");
            s = locator.ToString();
            Assert.Equal("stop locator --name=name --dir=dir", s);
        }

        [Fact]
        public void StopServerStringsTest()
        {
            var server = new GfshExecute(output)
                .stop()
                .server();
            var s = server.ToString();
            Assert.Equal("stop server", s);

            server = new GfshExecute(output)
                .stop()
                .server()
                .withName("server")
                .withDir("someDir");
            s = server.ToString();
            Assert.Equal("stop server --name=server --dir=someDir", s);
        }

        [Fact]
        public void CreateRegionStringsTest()
        {
            var region = new GfshExecute(output)
                .create()
                .region();
            var s = region.ToString();
            Assert.Equal("create region", s);

            region = new GfshExecute(output)
                .create()
                .region()
                .withName("region")
                .withType("PARTITION");
            s = region.ToString();
            Assert.Equal("create region --name=region --type=PARTITION", s);
        }

        [Fact]
        public void ShutdownStringsTest()
        {
            var shutdown = new GfshExecute(output)
                .shutdown();
            var s = shutdown.ToString();
            Assert.Equal("shutdown", s);

            shutdown = new GfshExecute(output)
                .shutdown()
                .withIncludeLocators(true);
            s = shutdown.ToString();
            Assert.Equal("shutdown --include-locators=true", s);

            shutdown = new GfshExecute(output)
                .shutdown()
                .withIncludeLocators(false);
            s = shutdown.ToString();
            Assert.Equal("shutdown --include-locators=false", s);
        }

        [Fact]
        public void ConfigurePdxStringsTest()
        {
            var configurePdx = new GfshExecute(output)
                .configurePdx();
            var s = configurePdx.ToString();
            Assert.Equal("configure pdx", s);

            configurePdx = new GfshExecute(output)
                .configurePdx()
                .withReadSerialized(true);
            s = configurePdx.ToString();
            Assert.Equal( "configure pdx --read-serialized=true", s);

            configurePdx = new GfshExecute(output)
                .configurePdx()
                .withReadSerialized(false);
            s = configurePdx.ToString();
            Assert.Equal("configure pdx --read-serialized=false", s);
        }

        [Fact]
        public void ConnectStringsTest()
        {
            var Gfsh = new GfshExecute(output);

            var connect = Gfsh
                .connect();
            var s = connect.ToString();
            Assert.Equal(s, "connect");

            connect = Gfsh
                .connect()
                .withJmxManager("localhost", 1234)
                .withUseSsl(true)
                .withKeyStore("some/path/keystore.jks")
                .withKeyStorePassword("password1")
                .withTrustStore("some/path/truststore.jks")
                .withTrustStorePassword("password2");
            s = connect.ToString();
            Assert.Equal("connect --jmx-manager=localhost[1234] --use-ssl=true " +
                "--key-store=some/path/keystore.jks --key-store-password=password1 " +
                "--trust-store=some/path/truststore.jks --trust-store-password=password2", s);
        }
    }
}
