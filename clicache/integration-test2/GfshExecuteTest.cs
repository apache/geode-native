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

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GfshExecuteTest : TestBase, IDisposable
    {
        public GfshExecuteTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        public void Dispose()
        {

        }

        [Fact]
        public void Start1Locator()
        {
            var gfsh = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            try
            {
                Assert.Equal(0, gfsh.start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(Framework.FreeTcpPort())
                    .execute());
            }
            finally
            {
                Assert.Equal(0, gfsh
                    .shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }

        [Fact]
        public void Start1Locator1Server()
        {
            var gfsh = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            try
            {
                Assert.Equal(0, gfsh.start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(Framework.FreeTcpPort())
                    .execute());

                Assert.Equal(0, gfsh.start()
                    .server()
                    .withDir(testDir + "/server/0")
                    .withPort(0)
                    .execute());
            }
            finally
            {
                Assert.Equal(0, gfsh
                    .shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }

        [Fact]
        public void Start1Locator2Servers()
        {
            var gfsh = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            try
            {
                Assert.Equal(0, gfsh.start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(Framework.FreeTcpPort())
                    .execute());

                Assert.Equal(0, gfsh.start()
                    .server()
                    .withDir(testDir + "/server/0")
                    .withPort(0)
                    .execute());

                Assert.Equal(0, gfsh.start()
                    .server()
                    .withDir(testDir + "/server/1")
                    .withPort(0)
                    .execute());
            }
            finally
            {
                Assert.Equal(0, gfsh.shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }

        [Fact]
        public void Start1LocatorWithSSL()
        {
            var gfsh = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            var sslPassword = "apachegeode";
            var keystore = Config.SslServerKeyPath + @"/server_keystore_chained.jks";
            var truststore = Config.SslServerKeyPath + @"/server_truststore_chained_root.jks";
            var jmxManagerPort = Framework.FreeTcpPort();

            try
            {
                Assert.Equal(0, gfsh
                    .start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(jmxManagerPort)
                    .withJmxManagerStart(true)
                    .withSslEnableComponents("locator,jmx")
                    .withSslKeyStore(keystore)
                    .withSslKeyStorePassword(sslPassword)
                    .withSslTrustStore(truststore)
                    .withSslTrustStorePassword(sslPassword)
                    .withConnect(false)
                    .execute());

                Assert.Equal(0, gfsh
                    .connect()
                    .withJmxManager("localhost", jmxManagerPort)
                    .withUseSsl(true)
                    .withKeyStore(keystore)
                    .withKeyStorePassword(sslPassword)
                    .withTrustStore(truststore)
                    .withTrustStorePassword(sslPassword)
                    .execute());
            }
            finally
            {
                Assert.Equal(0, gfsh
                    .shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }

        [Fact]
        public void Start1Locator1ServerWithSSL()
        {
            var gfsh = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            var sslPassword = "apachegeode";
            var keystore = Config.SslServerKeyPath + @"/server_keystore_chained.jks";
            var truststore = Config.SslServerKeyPath + @"/server_truststore_chained_root.jks";
            var jmxManagerPort = Framework.FreeTcpPort();

            try
            {
                Assert.Equal(0, gfsh
                    .start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(jmxManagerPort)
                    .withJmxManagerStart(true)
                    .withSslEnableComponents("locator,jmx")
                    .withSslKeyStore(keystore)
                    .withSslKeyStorePassword(sslPassword)
                    .withSslTrustStore(truststore)
                    .withSslTrustStorePassword(sslPassword)
                    .withConnect(false)
                    .execute());

                Assert.Equal(0, gfsh
                    .connect()
                    .withJmxManager("localhost", jmxManagerPort)
                    .withUseSsl(true)
                    .withKeyStore(keystore)
                    .withKeyStorePassword(sslPassword)
                    .withTrustStore(truststore)
                    .withTrustStorePassword(sslPassword)
                    .execute());

                Assert.Equal(0, gfsh
                    .start()
                    .server()
                    .withDir(testDir + "/server/0")
                    .withPort(0)
                    .withSslEnableComponents("server,locator,jmx")
                    .withSslKeyStore(keystore)
                    .withSslKeyStorePassword(sslPassword)
                    .withSslTrustStore(truststore)
                    .withSslTrustStorePassword(sslPassword)
                    .execute());
            }
            finally
            {
                Assert.Equal(0, gfsh
                    .shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }

        [Fact]
        public void Start2ClustersWith1Locator1ServerEach()
        {
            var gfsh1 = new GfshExecute(output);
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            try
            {
                Assert.Equal(0, gfsh1.start()
                    .locator()
                    .withDir(testDir + "/locator/0")
                    .withHttpServicePort(0)
                    .withPort(Framework.FreeTcpPort())
                    .withJmxManagerPort(Framework.FreeTcpPort())
                    .execute());

                var gfsh2 = new GfshExecute(output);
                try
                {
                    Assert.Equal(0, gfsh2.start()
                        .locator()
                        .withDir(testDir + "/locator/1")
                        .withHttpServicePort(0)
                        .withPort(Framework.FreeTcpPort())
                        .withJmxManagerPort(Framework.FreeTcpPort())
                        .execute());
                }
                finally
                {
                    Assert.Equal(0, gfsh2.shutdown()
                        .withIncludeLocators(true)
                        .execute());
                }

            }
            finally
            {
                Assert.Equal(0, gfsh1.shutdown()
                    .withIncludeLocators(true)
                    .execute());
            }
        }
    }
}
