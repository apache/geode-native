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
      Assert.NotEqual(cacheOne, cacheTwo);

      var dummyPdxSerializerOne = new DummyPdxSerializer();
      cacheOne.TypeRegistry.PdxSerializer = dummyPdxSerializerOne;

      var dummyPdxSerializerTwo = new DummyPdxSerializer();
      cacheTwo.TypeRegistry.PdxSerializer = dummyPdxSerializerTwo;

      var cacheOnePdxSerializer = cacheOne.TypeRegistry.PdxSerializer;
      var cacheTwoPdxSerializer = cacheTwo.TypeRegistry.PdxSerializer;

      Assert.Same(dummyPdxSerializerOne, cacheOnePdxSerializer);
      Assert.Same(dummyPdxSerializerTwo, cacheTwoPdxSerializer);
      Assert.NotSame(cacheOnePdxSerializer, cacheTwoPdxSerializer);
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

    static bool pdxDelegate1Called = false;
    static IPdxSerializable delegateForPdx1()
    {
      pdxDelegate1Called = true;
      return Pdx1.CreateDeserializable();
    }
    static bool pdxDelegate2Called = false;
    static IPdxSerializable delegateForPdx2()
    {
      pdxDelegate2Called = true;
      return Pdx2.CreateDeserializable();
    }

    [Fact]
    public void RegisterPdxType()
    {
      cacheOne.TypeRegistry.RegisterPdxType(delegateForPdx1);
      cacheTwo.TypeRegistry.RegisterPdxType(delegateForPdx2);

      pdxDelegate1Called = false;
      var pdx1Type = cacheOne.TypeRegistry.GetPdxType(typeof(Pdx1).FullName);
      Assert.True(pdxDelegate1Called);

      pdxDelegate2Called = false;
      var pdx2Type = cacheOne.TypeRegistry.GetPdxType(typeof(Pdx2).FullName);
      Assert.False(pdxDelegate2Called);
    }

    [Fact]
    public void GetPut_withMultipleCaches()
    {
      var geodeServer = new GeodeServer();
      var cacheXml = new CacheXml(new FileInfo("cache.xml"), geodeServer);

      var cacheFactory = new CacheFactory();

      cacheOne = cacheFactory.Create();
      cacheOne.InitializeDeclarativeCache(cacheXml.File.FullName);
      cacheTwo = cacheFactory.Create();
      cacheTwo.InitializeDeclarativeCache(cacheXml.File.FullName);


      var regionForCache1 = cacheOne.GetRegion<string, string>("testRegion1");
      var regionForCache2 = cacheTwo.GetRegion<string, string>("testRegion1");

      const string key = "hello";
      const string expectedResult = "dave";
      regionForCache1.Put(key, expectedResult, null);
      var actualResult = regionForCache2.Get(key, null);

      Assert.Equal(expectedResult, actualResult);

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

  internal class Pdx1 : IPdxSerializable
  {
    // object fields
    private int m_id;
    private string m_pkid;
    private string m_type;
    private string m_status;
    private string[] m_names;
    private byte[] m_newVal;
    private DateTime m_creationDate;
    private byte[] m_arrayZeroSize;
    private byte[] m_arrayNull;

    public Pdx1() { }

    public Pdx1(int id, int size, string[] names)
    {
      m_names = names;
      m_id = id;
      m_pkid = id.ToString();
      m_status = (id % 2 == 0) ? "active" : "inactive";
      m_type = "type" + (id % 3);

      if (size > 0)
      {
        m_newVal = new byte[size];
        for (int index = 0; index < size; index++)
        {
          m_newVal[index] = (byte)'B';
        }
      }
      m_creationDate = DateTime.Now;
      m_arrayNull = null;
      m_arrayZeroSize = new byte[0];
    }

    public void FromData(IPdxReader reader)
    {
      m_id = reader.ReadInt("id");

      bool isIdentity = reader.IsIdentityField("id");

      if (isIdentity == false)
        throw new IllegalStateException("Pdx1 id is identity field");

      bool isId = reader.HasField("id");

      if (isId == false)
        throw new IllegalStateException("Pdx1 id field not found");

      bool isNotId = reader.HasField("ID");

      if (isNotId == true)
        throw new IllegalStateException("Pdx1 isNotId field found");

      m_pkid = reader.ReadString("pkid");
      m_type = reader.ReadString("type");
      m_status = reader.ReadString("status");
      m_names = reader.ReadStringArray("names");
      m_newVal = reader.ReadByteArray("newVal");
      m_creationDate = reader.ReadDate("creationDate");
      m_arrayNull = reader.ReadByteArray("arrayNull");
      m_arrayZeroSize = reader.ReadByteArray("arrayZeroSize");
    }

    public void ToData(IPdxWriter writer)
    {
      writer
        .WriteInt("id", m_id)
        //identity field
        .MarkIdentityField("id")
        .WriteString("pkid", m_pkid)
        .WriteString("type", m_type)
        .WriteString("status", m_status)
        .WriteStringArray("names", m_names)
        .WriteByteArray("newVal", m_newVal)
        .WriteDate("creationDate", m_creationDate)
        .WriteByteArray("arrayNull", m_arrayNull)
        .WriteByteArray("arrayZeroSize", m_arrayZeroSize);
    }

    public static IPdxSerializable CreateDeserializable()
    {
      return new Pdx1(777, 100, new string[] { "LEAF", "Volt", "Bolt" });
    }
  }

  internal class Pdx2 : IPdxSerializable
  {
    // object fields
    private int m_id;
    private string m_pkid;
    private string m_pkid2;
    private string m_type;
    private string m_status;
    private string m_status2;
    private string[] m_names;
    private string[] m_addresses;
    private byte[] m_newVal;
    private DateTime m_creationDate;
    private byte[] m_arrayZeroSize;
    private byte[] m_arrayNull;

    public Pdx2() { }

    public Pdx2(int id, int size, string[] names, string[] addresses)
    {
      m_names = names;
      m_addresses = addresses;
      m_id = id;
      m_pkid = id.ToString();
      m_pkid2 = id.ToString() + "two";
      m_status = (id % 2 == 0) ? "active" : "inactive";
      m_status2 = (id % 3 == 0) ? "red" : "green";
      m_type = "type" + (id % 3);

      if (size > 0)
      {
        m_newVal = new byte[size];
        for (int index = 0; index < size; index++)
        {
          m_newVal[index] = (byte)'B';
        }
      }
      m_creationDate = DateTime.Now;
      m_arrayNull = null;
      m_arrayZeroSize = new byte[0];
    }

    public void FromData(IPdxReader reader)
    {
      m_id = reader.ReadInt("id");

      bool isIdentity = reader.IsIdentityField("id");

      if (isIdentity == false)
        throw new IllegalStateException("Pdx1 id is identity field");

      bool isId = reader.HasField("id");

      if (isId == false)
        throw new IllegalStateException("Pdx1 id field not found");

      bool isNotId = reader.HasField("ID");

      if (isNotId == true)
        throw new IllegalStateException("Pdx1 isNotId field found");

      m_pkid = reader.ReadString("pkid");
      m_pkid2 = reader.ReadString("pkid2");
      m_type = reader.ReadString("type");
      m_status = reader.ReadString("status");
      m_status2 = reader.ReadString("status2");
      m_names = reader.ReadStringArray("names");
      m_addresses = reader.ReadStringArray("addresses");
      m_newVal = reader.ReadByteArray("newVal");
      m_creationDate = reader.ReadDate("creationDate");
      m_arrayNull = reader.ReadByteArray("arrayNull");
      m_arrayZeroSize = reader.ReadByteArray("arrayZeroSize");
    }

    public void ToData(IPdxWriter writer)
    {
      writer
        .WriteInt("id", m_id)
        //identity field
        .MarkIdentityField("id")
        .WriteString("pkid", m_pkid)
        .WriteString("pkid2", m_pkid2)
        .WriteString("type", m_type)
        .WriteString("status", m_status)
        .WriteString("status2", m_status2)
        .WriteStringArray("names", m_names)
        .WriteStringArray("addresses", m_addresses)
        .WriteByteArray("newVal", m_newVal)
        .WriteDate("creationDate", m_creationDate)
        .WriteByteArray("arrayNull", m_arrayNull)
        .WriteByteArray("arrayZeroSize", m_arrayZeroSize);
    }

    public static IPdxSerializable CreateDeserializable()
    {
      return new Pdx2(777, 100, new string[] { "Nissan", "Chevy", "Volvo" }, new string[] { "4451 Court St", "1171 Elgin Ave", "721 NW 173rd Pl" });
    }
  }
}