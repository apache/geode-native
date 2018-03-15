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

namespace Apache.Geode.Client.UnitTests
{

  [Trait("Category", "Integration")]
  public class MultipleCacheTests
  {
    Cache cacheOne;
    Cache cacheTwo;

    public MultipleCacheTests()
    {
      var cacheFactory = new CacheFactory();
      cacheOne = cacheFactory.Create();
      cacheTwo = cacheFactory.Create();
    }

    public void Dispose()
    {
      cacheOne.Close();
      cacheTwo.Close();
    }

    [Fact]
    public void RegisterSerializerForTwoCaches()
    {
      Assert.Equal(cacheOne, cacheTwo);

      var dummyPdxSerializer = new DummyPdxSerializer();
      cacheOne.TypeRegistry.PdxSerializer = dummyPdxSerializer;

      Assert.Same(dummyPdxSerializer, cacheOne.TypeRegistry.PdxSerializer);
      Assert.Null(cacheTwo.TypeRegistry.PdxSerializer);
    }

    [Fact]
    public void SetPdxTypeMapper()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      Assert.Same(dummyPdxTypeMapper, cacheOne.TypeRegistry.PdxTypeMapper);
      Assert.Null(cacheTwo.TypeRegistry.PdxTypeMapper);
    }

    [Fact]
    public void GetPdxTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var pdxName = cacheOne.TypeRegistry.GetPdxTypeName("bar");

      Assert.Equal("foo", pdxName);
      Assert.Equal("bar", cacheTwo.TypeRegistry.GetPdxTypeName("bar"));
    }

    [Fact]
    public void GetLocalTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var localName = cacheOne.TypeRegistry.GetLocalTypeName("foo");

      Assert.Equal("bar", localName);
      Assert.Equal("foo", cacheTwo.TypeRegistry.GetLocalTypeName("foo"));
    }

    [Fact]
    public void GetPut_withMultipleCaches()
    {
      var geodeServer = new GeodeServer();
      var cacheXml = new CacheXml(new FileInfo("cache.xml"), geodeServer);
      const int webTimeoutMinutes = 1;//needed?????????????

      var properties = new Properties<string, string>();
      properties.Insert("cache-xml-file", cacheXml.File.FullName);
      properties.Insert("geode-properties-file", "geode.properties");
      properties.Insert("web-timeout-minutes", webTimeoutMinutes.ToString());

      var cacheFactory = new CacheFactory(properties);

      cacheOne = cacheFactory.Create();
      cacheTwo = cacheFactory.Create();

      var regionOne = cacheOne.GetRegion<string, string>("testRegion1");
      var regionTwo = cacheTwo.GetRegion<string, string>("testRegion1");

      const string key = "hello";
      const string expectedRegionOneResult = "dave";
      const string expectedRegionTwoResult = "bob";
      regionOne.Put(key, expectedRegionOneResult, null);
      regionTwo.Put(key, expectedRegionTwoResult, null);

      var actualRegionOneResult = regionOne.Get(key, null);
      var actualRegionTwoResult = regionTwo.Get(key, null);

      Assert.Equal(expectedRegionOneResult, actualRegionOneResult);
      Assert.Equal(expectedRegionTwoResult, actualRegionTwoResult);

      cacheXml.Dispose();
      geodeServer.Dispose();
    }
  }

  internal class DummyPdxSerializer : IPdxSerializer
  {
    public object FromData(string classname, IPdxReader reader)
    {
      throw new NotImplementedException();
    }

    public bool ToData(object o, IPdxWriter writer)
    {
      throw new NotImplementedException();
    }
  }

  internal class DummyPdxTypeMapper : IPdxTypeMapper
  {
    public string FromPdxTypeName(string pdxTypeName)
    {
      return "foo".Equals(pdxTypeName) ? "bar" : null;
    }

    public string ToPdxTypeName(string localTypeName)
    {
      return "bar".Equals(localTypeName) ? "foo" : null;
    }
  }
}
