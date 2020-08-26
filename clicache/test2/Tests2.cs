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
  [Trait("Category", "UnitTests")]
  public class Tests2 : IDisposable
  {
    private readonly Cache _cacheOne;
    private readonly Cache _cacheTwo;

    public Tests2()
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
    public void RegisterSerializerForTwoCaches()
    {
      Assert.NotEqual(_cacheOne, _cacheTwo);

      var dummyPdxSerializerOne = new DummyPdxSerializer();
      _cacheOne.TypeRegistry.PdxSerializer = dummyPdxSerializerOne;

      var dummyPdxSerializerTwo = new DummyPdxSerializer();
      _cacheTwo.TypeRegistry.PdxSerializer = dummyPdxSerializerTwo;

      var cacheOnePdxSerializer = _cacheOne.TypeRegistry.PdxSerializer;
      var cacheTwoPdxSerializer = _cacheTwo.TypeRegistry.PdxSerializer;

      Assert.Same(dummyPdxSerializerOne, cacheOnePdxSerializer);
      Assert.Same(dummyPdxSerializerTwo, cacheTwoPdxSerializer);
      Assert.NotSame(cacheOnePdxSerializer, cacheTwoPdxSerializer);
    }

    [Fact]
    public void SetPdxTypeMapper()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      _cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      Assert.Same(dummyPdxTypeMapper, _cacheOne.TypeRegistry.PdxTypeMapper);
      Assert.Null(_cacheTwo.TypeRegistry.PdxTypeMapper);
    }

    [Fact]
    public void GetPdxTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      _cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var pdxName = _cacheOne.TypeRegistry.GetPdxTypeName("bar");

      Assert.Equal("foo", pdxName);
      Assert.Equal("bar", _cacheTwo.TypeRegistry.GetPdxTypeName("bar"));
    }

    [Fact]
    public void GetLocalTypeName()
    {
      var dummyPdxTypeMapper = new DummyPdxTypeMapper();
      _cacheOne.TypeRegistry.PdxTypeMapper = dummyPdxTypeMapper;

      var localName = _cacheOne.TypeRegistry.GetLocalTypeName("foo");

      Assert.Equal("bar", localName);
      Assert.Equal("foo", _cacheTwo.TypeRegistry.GetLocalTypeName("foo"));
    }

    private static bool _pdxDelegate1Called;

    private static IPdxSerializable DelegateForPdx1()
    {
      _pdxDelegate1Called = true;
      return Pdx1.CreateDeserializable();
    }

    private static bool _pdxDelegate2Called;

    private static IPdxSerializable DelegateForPdx2()
    {
      _pdxDelegate2Called = true;
      return Pdx2.CreateDeserializable();
    }

    [Fact]
    public void RegisterPdxType()
    {
      _cacheOne.TypeRegistry.RegisterPdxType(DelegateForPdx1);
      _cacheTwo.TypeRegistry.RegisterPdxType(DelegateForPdx2);

      _pdxDelegate1Called = false;
      var pdx1Type = _cacheOne.TypeRegistry.GetPdxType(typeof(Pdx1).FullName);
      Assert.True(_pdxDelegate1Called);

      _pdxDelegate2Called = false;
      var pdx2Type = _cacheOne.TypeRegistry.GetPdxType(typeof(Pdx2).FullName);
      Assert.False(_pdxDelegate2Called);
    }

    [Fact]
    public void SetSniProxy()
    {
        PoolFactory poolFactory = _cacheOne.GetPoolFactory()
                .AddLocator("localhost", 10334)
                .SetSniProxy("haproxy", 7777);

        Pool pool = poolFactory.Create("testPool");

        string sniProxyHost = pool.SniProxyHost;
        int sniProxyPort = pool.SniProxyPort;

        Assert.Equal(sniProxyHost, "haproxy");
        Assert.Equal(sniProxyPort, 7777);
    }

    private class DummyPdxSerializer : IPdxSerializer
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

    private class DummyPdxTypeMapper : IPdxTypeMapper
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

    private class Pdx1 : IPdxSerializable
    {
      // object fields
      private int _mId;
      private string _mPkid;
      private string _mType;
      private string _mStatus;
      private string[] _mNames;
      private byte[] _mNewVal;
      private DateTime _mCreationDate;
      private byte[] _mArrayZeroSize;
      private byte[] _mArrayNull;

      public Pdx1()
      {
      }

      public Pdx1(int id, int size, string[] names)
      {
        _mNames = names;
        _mId = id;
        _mPkid = id.ToString();
        _mStatus = (id % 2 == 0) ? "active" : "inactive";
        _mType = "type" + (id % 3);

        if (size > 0)
        {
          _mNewVal = new byte[size];
          for (var index = 0; index < size; index++)
          {
            _mNewVal[index] = (byte) 'B';
          }
        }

        _mCreationDate = DateTime.Now;
        _mArrayNull = null;
        _mArrayZeroSize = new byte[0];
      }

      public void FromData(IPdxReader reader)
      {
        _mId = reader.ReadInt("id");

        var isIdentity = reader.IsIdentityField("id");

        if (isIdentity == false)
          throw new IllegalStateException("Pdx1 id is identity field");

        var isId = reader.HasField("id");

        if (isId == false)
          throw new IllegalStateException("Pdx1 id field not found");

        var isNotId = reader.HasField("ID");

        if (isNotId)
          throw new IllegalStateException("Pdx1 isNotId field found");

        _mPkid = reader.ReadString("pkid");
        _mType = reader.ReadString("type");
        _mStatus = reader.ReadString("status");
        _mNames = reader.ReadStringArray("names");
        _mNewVal = reader.ReadByteArray("newVal");
        _mCreationDate = reader.ReadDate("creationDate");
        _mArrayNull = reader.ReadByteArray("arrayNull");
        _mArrayZeroSize = reader.ReadByteArray("arrayZeroSize");
      }

      public void ToData(IPdxWriter writer)
      {
        writer
          .WriteInt("id", _mId)
          //identity field
          .MarkIdentityField("id")
          .WriteString("pkid", _mPkid)
          .WriteString("type", _mType)
          .WriteString("status", _mStatus)
          .WriteStringArray("names", _mNames)
          .WriteByteArray("newVal", _mNewVal)
          .WriteDate("creationDate", _mCreationDate)
          .WriteByteArray("arrayNull", _mArrayNull)
          .WriteByteArray("arrayZeroSize", _mArrayZeroSize);
      }

      public static IPdxSerializable CreateDeserializable()
      {
        return new Pdx1(777, 100, new[] {"LEAF", "Volt", "Bolt"});
      }
    }

    private class Pdx2 : IPdxSerializable
    {
      // object fields
      private int _mId;
      private string _mPkid;
      private string _mPkid2;
      private string _mType;
      private string _mStatus;
      private string _mStatus2;
      private string[] _mNames;
      private string[] _mAddresses;
      private byte[] _mNewVal;
      private DateTime _mCreationDate;
      private byte[] _mArrayZeroSize;
      private byte[] _mArrayNull;

      public Pdx2()
      {
      }

      public Pdx2(int id, int size, string[] names, string[] addresses)
      {
        _mNames = names;
        _mAddresses = addresses;
        _mId = id;
        _mPkid = id.ToString();
        _mPkid2 = id + "two";
        _mStatus = (id % 2 == 0) ? "active" : "inactive";
        _mStatus2 = (id % 3 == 0) ? "red" : "green";
        _mType = "type" + (id % 3);

        if (size > 0)
        {
          _mNewVal = new byte[size];
          for (var index = 0; index < size; index++)
          {
            _mNewVal[index] = (byte) 'B';
          }
        }

        _mCreationDate = DateTime.Now;
        _mArrayNull = null;
        _mArrayZeroSize = new byte[0];
      }

      public void FromData(IPdxReader reader)
      {
        _mId = reader.ReadInt("id");

        var isIdentity = reader.IsIdentityField("id");

        if (isIdentity == false)
          throw new IllegalStateException("Pdx2 id is identity field");

        var isId = reader.HasField("id");

        if (isId == false)
          throw new IllegalStateException("Pdx2 id field not found");

        var isNotId = reader.HasField("ID");

        if (isNotId)
          throw new IllegalStateException("Pdx2 isNotId field found");

        _mPkid = reader.ReadString("pkid");
        _mPkid2 = reader.ReadString("pkid2");
        _mType = reader.ReadString("type");
        _mStatus = reader.ReadString("status");
        _mStatus2 = reader.ReadString("status2");
        _mNames = reader.ReadStringArray("names");
        _mAddresses = reader.ReadStringArray("addresses");
        _mNewVal = reader.ReadByteArray("newVal");
        _mCreationDate = reader.ReadDate("creationDate");
        _mArrayNull = reader.ReadByteArray("arrayNull");
        _mArrayZeroSize = reader.ReadByteArray("arrayZeroSize");
      }

      public void ToData(IPdxWriter writer)
      {
        writer
          .WriteInt("id", _mId)
          //identity field
          .MarkIdentityField("id")
          .WriteString("pkid", _mPkid)
          .WriteString("pkid2", _mPkid2)
          .WriteString("type", _mType)
          .WriteString("status", _mStatus)
          .WriteString("status2", _mStatus2)
          .WriteStringArray("names", _mNames)
          .WriteStringArray("addresses", _mAddresses)
          .WriteByteArray("newVal", _mNewVal)
          .WriteDate("creationDate", _mCreationDate)
          .WriteByteArray("arrayNull", _mArrayNull)
          .WriteByteArray("arrayZeroSize", _mArrayZeroSize);
      }

      public static IPdxSerializable CreateDeserializable()
      {
        return new Pdx2(777, 100, new[] {"Nissan", "Chevy", "Volvo"},
          new[] {"4451 Court St", "1171 Elgin Ave", "721 NW 173rd Pl"});
      }
    }
  }
}
