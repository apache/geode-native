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

namespace Apache.Geode.Client.IntegrationTests
{
    [Trait("Category", "Integration")]
    public class GfshExecuteTest : TestBase, IDisposable
    {
        public void Dispose()
        {

        }

        [Fact]
        public void GfshExecuteStartLocatorTest()
        {
            using (var gfsh = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withDir(testDir)
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh
                        .shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartServerTest()
        {
            using (var gfsh = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withDir(testDir)
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);

                    Assert.Equal(gfsh.start()
                        .server()
                        .withDir(testDir + "/server/0")
                        .withPort(0)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh
                        .shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartTwoServersTest()
        {
            using (var gfsh1 = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                try
                {
                    Assert.Equal(gfsh1.start()
                        .locator()
                        .withDir(testDir)
                        .withHttpServicePort(0)
                        .execute(), 0);

                    Assert.Equal(gfsh1.start()
                        .server()
                        .withDir(testDir + "/server/0")
                        .withPort(0)
                        .execute(), 0);

                    Assert.Equal(gfsh1.start()
                        .server()
                        .withDir(testDir + "/server/1")
                        .withPort(0)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh1.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartLocatorWithUseSslTest()
        {
            using (var gfsh = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                var sslPassword = "gemstone";
                var currentDir = Environment.CurrentDirectory;
                gfsh.Keystore = currentDir + "/ServerSslKeys/server_keystore.jks";
                gfsh.KeystorePassword = sslPassword;
                gfsh.Truststore = currentDir + "/ServerSslKeys/server_truststore.jks";
                gfsh.TruststorePassword = sslPassword;
                gfsh.UseSSL = true;

                try
                {
                    var locator = gfsh
                        .start()
                        .locator()
                        .withDir(testDir)
                        .withHttpServicePort(0)
                        .withUseSsl()
                        .withConnect(false);

                    Assert.Equal(locator.execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh
                        .shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartLocatorAndServerWithUseSslTest()
        {
            using (var gfsh = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                var sslPassword = "gemstone";
                var currentDir = Environment.CurrentDirectory;
                gfsh.Keystore = currentDir + "/ServerSslKeys/server_keystore.jks";
                gfsh.KeystorePassword = sslPassword;
                gfsh.Truststore = currentDir + "/ServerSslKeys/server_truststore.jks";
                gfsh.TruststorePassword = sslPassword;
                gfsh.UseSSL = true;

                try
                {
                    Assert.Equal(gfsh
                        .start()
                        .locator()
                        .withDir(testDir)
                        .withHttpServicePort(0)
                        .withUseSsl()
                        .withConnect(false)
                        .execute(), 0);

                    Assert.Equal(gfsh
                        .start()
                        .server()
                        .withDir(testDir + "/server/0")
                        .withPort(0)
                        .withUseSsl()
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh
                        .shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartLocatorAndVerifyPortTest()
        {
            using (var gfsh = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                Assert.Equal(gfsh.start()
                    .locator()
                    .withDir(testDir)
                    .withHttpServicePort(0)
                    .execute(), 0);

                Assert.NotEqual(0, gfsh.LocatorPort);

                Assert.Equal(gfsh.shutdown()
                    .withIncludeLocators(true)
                    .execute(), 0);
            }
        }

        [Fact]
        public void GfshExecuteStartTwoLocatorsTest()
        {
            using (var gfsh1 = new GfshExecute())
            {
                var testDir = CreateTestCaseDirectoryName();
                CleanTestCaseDirectory(testDir);

                try
                {
                    Assert.Equal(gfsh1.start()
                        .locator()
                        .withDir(testDir + "/locator/0")
                        .withHttpServicePort(0)
                        .execute(), 0);

                    using (var gfsh2 = new GfshExecute())
                    {
                        try
                        {
                            Assert.Equal(gfsh2.start()
                                .locator()
                                .withDir(testDir + "/locator/1")
                                .withHttpServicePort(0)
                                .execute(), 0);
                        }
                        finally
                        {
                            Assert.Equal(gfsh2.shutdown()
                                .withIncludeLocators(true)
                                .execute(), 0);
                        }
                    }
                }
                finally
                {
                    Assert.Equal(gfsh1.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }
    }
}
