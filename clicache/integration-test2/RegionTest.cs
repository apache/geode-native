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
  public class RegionTest : IDisposable
  {
    private readonly Cache _cacheOne;
    private readonly Cache _cacheTwo;

    public RegionTest()
    {
      var cacheFactory = new CacheFactory();
      _cacheOne = cacheFactory.Create();
      _cacheTwo = cacheFactory.Create();
    }

    public void Dispose()
    {
      _cacheOne.Close();
      _cacheTwo.Close();
    }

    [Fact]
    public void PutOnOneCacheGetOnAnotherCache()
    {
      var geodeServer = new GeodeServer();
      var cacheXml = new CacheXml(new FileInfo("cache.xml"), geodeServer);

      _cacheOne.InitializeDeclarativeCache(cacheXml.File.FullName);
      _cacheTwo.InitializeDeclarativeCache(cacheXml.File.FullName);

      var regionForCache1 = _cacheOne.GetRegion<string, string>("testRegion1");
      var regionForCache2 = _cacheTwo.GetRegion<string, string>("testRegion1");

      const string key = "hello";
      const string expectedResult = "dave";

      regionForCache1.Put(key, expectedResult, null);
      var actualResult = regionForCache2.Get(key, null);

      Assert.Equal(expectedResult, actualResult);

      cacheXml.Dispose();
      geodeServer.Dispose();
    }
  }
}