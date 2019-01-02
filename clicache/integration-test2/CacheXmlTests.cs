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

using System.IO;
using System.Threading;
using Xunit;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{

    [Trait("Category", "Integration")]
    public class CacheXmlTests : TestBase
    {
        public CacheXmlTests(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void ConstructAndGenerate()
        {
            string testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            var template = new FileInfo("cache.xml");
            var cacheXml = new CacheXml(template, 1234);
            Assert.NotNull(cacheXml.File);
            Assert.True(cacheXml.File.Exists);

            using (var input = cacheXml.File.OpenText())
            {
                var content = input.ReadToEnd();
                Assert.True(content.Contains(1234.ToString()));
            }
        }

        [Fact]
        public void DisposeAndCleanup()
        {
            var testDir = CreateTestCaseDirectoryName();
            CleanTestCaseDirectory(testDir);

            FileInfo file;

            var template = new FileInfo("cache.xml");
            using (var cacheXml = new CacheXml(template, 1234))
            {
                Assert.NotNull(cacheXml.File);
                file = cacheXml.File;
                Assert.True(file.Exists);
            }

            file.Refresh();

            // File deletion via File.Delete (inside the file.Refresh() call)
            // is not synchronous so we need to potentially wait until the file 
            // has been deleted here
            Assert.True(SpinWait.SpinUntil(() => !file.Exists, 10000));
        }
    }
}
