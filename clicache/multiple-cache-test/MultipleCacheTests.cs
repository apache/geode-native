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

namespace Apache.Geode.Client.UnitTests
{

  [Trait("Category", "Unit")]
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
