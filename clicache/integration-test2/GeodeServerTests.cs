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

using Xunit;

[Trait("Category", "Integration")]
public class GemFireServerTest
{
    [Fact]
    public void Start()
    {
        using (var geodeServer = new GeodeServer())
        {
            Assert.NotNull(geodeServer);
            Assert.NotEqual(0, geodeServer.LocatorPort);
        }
    }

    [Fact]
    public void StartTwo()
    {
        using (var geodeServer1 = new GeodeServer())
        {
            Assert.NotNull(geodeServer1);
            Assert.NotEqual(0, geodeServer1.LocatorPort);

            using (var geodeServer2 = new GeodeServer())
            {
                Assert.NotNull(geodeServer2);
                Assert.NotEqual(0, geodeServer2.LocatorPort);
                Assert.NotEqual(geodeServer1.LocatorPort, geodeServer2.LocatorPort);
            }
        }
    }
}
