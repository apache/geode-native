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

    [Fact]
    public void PutOnOneCacheGetOnAnotherCache()
    {
      using (var geodeServer = new GeodeServer())
      {
        using (var cacheXml = new CacheXml(new FileInfo("cache.xml"), geodeServer))
        {
          var cacheFactory = new CacheFactory();

          var cacheOne = cacheFactory.Create();
          try
          {
            cacheOne.InitializeDeclarativeCache(cacheXml.File.FullName);

            var cacheTwo = cacheFactory.Create();
            try
            {
              cacheTwo.InitializeDeclarativeCache(cacheXml.File.FullName);

              var regionForCache1 = cacheOne.GetRegion<string, string>("testRegion1");
              var regionForCache2 = cacheTwo.GetRegion<string, string>("testRegion1");

              const string key = "hello";
              const string expectedResult = "dave";
              regionForCache1.Put(key, expectedResult, null);
              var actualResult = regionForCache2.Get(key, null);

              Assert.Equal(expectedResult, actualResult);
            }
            finally
            {
              cacheTwo.Close();
            }
          }
          finally
          {
            cacheOne.Close();
          }
        }
      }
    }
  }

}
