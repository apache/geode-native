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
    public class GfshExecuteTest : IDisposable
    {
        public void Dispose()
        {

        }

        [Fact]
        public void GfshExecuteStartLocatorTest()
        {
            using (GfshExecute gfsh = new GfshExecute())
            {
                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }

        [Fact]
        public void GfshExecuteStartServerTest()
        {
            using (GfshExecute gfsh = new GfshExecute())
            {
                try
                {
                    Assert.Equal(gfsh.start()
                        .locator()
                        .withHttpServicePort(0)
                        .withPort(gfsh.LocatorPort)
                        .execute(), 0);
                    Assert.Equal(gfsh.start()
                        .server()
                        .withPort(0)
                        .execute(), 0);
                }
                finally
                {
                    Assert.Equal(gfsh.shutdown()
                        .withIncludeLocators(true)
                        .execute(), 0);
                }
            }
        }
    }
}
