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
using System.Collections.ObjectModel;
using System.IO;
using System.Threading;
using NUnit.Framework;
using Apache.Geode.DUnitFramework;
using Apache.Geode.Client;

namespace Apache.Geode.Client.UnitTests
{

  [TestFixture]
  public class ThinClientMultipleCacheTests
  {
    Cache cacheOne;
    Cache cacheTwo;

    [TestFixtureSetUp]
    public void SetUp()
    {
      var cacheFactory = new CacheFactory();
      cacheOne = cacheFactory.Create();
      cacheTwo = cacheFactory.Create();

    }

    [Test]
    public void RegisterSerializerForTwoCaches()
    {
      Assert.AreNotEqual(cacheOne, cacheTwo);
      
      var dummyPdxSerializer = new DummyPdxSerializer();
      cacheOne.TypeRegistry.PdxSerializer = dummyPdxSerializer;

      Assert.AreSame(dummyPdxSerializer, cacheOne.TypeRegistry.PdxSerializer);

      Assert.IsNull(cacheTwo.TypeRegistry.PdxSerializer);
    }

    [Test]
    public void SetPdxTypeMapper()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      Assert.AreSame(dummyPdxTypeMapper, cacheOne.TypeRegistry.PdxTypeMapper);
      Assert.IsNull(cacheTwo.TypeRegistry.PdxTypeMapper);
    }

    [Test]
    public void GetPdxTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var pdxName = cacheOne.TypeRegistry.GetPdxTypeName("bar");

      Assert.AreEqual("foo", pdxName);
      Assert.AreEqual("bar", cacheTwo.TypeRegistry.GetPdxTypeName("bar"));
    }

    [Test]
    public void GetLocalTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var localName = cacheOne.TypeRegistry.GetLocalTypeName("foo");

      Assert.AreEqual("bar", localName);
      Assert.AreEqual("foo", cacheTwo.TypeRegistry.GetLocalTypeName("foo"));
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
      if ("foo".Equals(pdxTypeName))
      {
        return "bar";
      }

      return null;
    }

    public string ToPdxTypeName(string localTypeName)
    {
      if ("bar".Equals(localTypeName))
      {
        return "foo";
      }

      return null;
    }
  }
}
