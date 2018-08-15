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
using PdxTests;
using System.Collections;
using System.Collections.Generic;

namespace Apache.Geode.Client.IntegrationTests
{
  public struct CData
  {
    #region Private members

    private Int32 m_first;
    private Int64 m_second;

    #endregion

    #region Public accessors

    public Int32 First
    {
      get
      {
        return m_first;
      }
      set
      {
        m_first = value;
      }
    }

    public Int64 Second
    {
      get
      {
        return m_second;
      }
      set
      {
        m_second = value;
      }
    }

    #endregion

    public CData(Int32 first, Int64 second)
    {
      m_first = first;
      m_second = second;
    }

    public static bool operator ==(CData obj1, CData obj2)
    {
      return ((obj1.m_first == obj2.m_first) && (obj1.m_second == obj2.m_second));
    }

    public static bool operator !=(CData obj1, CData obj2)
    {
      return ((obj1.m_first != obj2.m_first) || (obj1.m_second != obj2.m_second));
    }

    public override bool Equals(object obj)
    {
      if (obj is CData)
      {
        CData otherObj = (CData)obj;
        return ((m_first == otherObj.m_first) && (m_second == otherObj.m_second));
      }
      return false;
    }

    public override int GetHashCode()
    {
      return m_first.GetHashCode() ^ m_second.GetHashCode();
    }
  };

  public class OtherType : IDataSerializable
  {
    private CData m_struct;
    private ExceptionType m_exType;

    public enum ExceptionType
    {
      None,
      Geode,
      System,
      // below are with inner exceptions
      GeodeGeode,
      GeodeSystem,
      SystemGeode,
      SystemSystem
    }

    public OtherType()
    {
      m_exType = ExceptionType.None;
    }

    public OtherType(Int32 first, Int64 second)
      : this(first, second, ExceptionType.None)
    {
    }

    public OtherType(Int32 first, Int64 second, ExceptionType exType)
    {
      m_struct.First = first;
      m_struct.Second = second;
      m_exType = exType;
    }

    public CData Data
    {
      get
      {
        return m_struct;
      }
    }

    #region IDataSerializable Members

    public void FromData(DataInput input)
    {
      m_struct.First = input.ReadInt32();
      m_struct.Second = input.ReadInt64();
      switch (m_exType)
      {
        case ExceptionType.None:
          break;
        case ExceptionType.Geode:
          throw new GeodeIOException("Throwing an exception");
        case ExceptionType.System:
          throw new IOException("Throwing an exception");
        case ExceptionType.GeodeGeode:
          throw new GeodeIOException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.GeodeSystem:
          throw new CacheServerException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
        case ExceptionType.SystemGeode:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.SystemSystem:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
      }
    }

    public void ToData(DataOutput output)
    {
      output.WriteInt32(m_struct.First);
      output.WriteInt64(m_struct.Second);
      switch (m_exType)
      {
        case ExceptionType.None:
          break;
        case ExceptionType.Geode:
          throw new GeodeIOException("Throwing an exception");
        case ExceptionType.System:
          throw new IOException("Throwing an exception");
        case ExceptionType.GeodeGeode:
          throw new GeodeIOException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.GeodeSystem:
          throw new CacheServerException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
        case ExceptionType.SystemGeode:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.SystemSystem:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
      }
    }

    public UInt64 ObjectSize
    {
      get
      {
        return (UInt32)(sizeof(Int32) + sizeof(Int64));
      }
    }

    public String Type
    {
      get { return this.GetType().ToString(); }
    }

    #endregion

    public static ISerializable CreateDeserializable()
    {
      return new OtherType();
    }

    public override int GetHashCode()
    {
      return m_struct.First.GetHashCode() ^ m_struct.Second.GetHashCode();
    }

    public override bool Equals(object obj)
    {
      OtherType ot = obj as OtherType;
      if (ot != null)
      {
        return (m_struct.Equals(ot.m_struct));
      }
      return false;
    }
  }

  public class OtherType2 : IDataSerializable
  {
    private CData m_struct;
    private ExceptionType m_exType;

    public enum ExceptionType
    {
      None,
      Geode,
      System,
      // below are with inner exceptions
      GeodeGeode,
      GeodeSystem,
      SystemGeode,
      SystemSystem
    }

    public OtherType2()
    {
      m_exType = ExceptionType.None;
    }

    public OtherType2(Int32 first, Int64 second)
      : this(first, second, ExceptionType.None)
    {
    }

    public OtherType2(Int32 first, Int64 second, ExceptionType exType)
    {
      m_struct.First = first;
      m_struct.Second = second;
      m_exType = exType;
    }

    public CData Data
    {
      get
      {
        return m_struct;
      }
    }
    #region IDataSerializable Members

    public void FromData(DataInput input)
    {
      m_struct.First = input.ReadInt32();
      m_struct.Second = input.ReadInt64();
      switch (m_exType)
      {
        case ExceptionType.None:
          break;
        case ExceptionType.Geode:
          throw new GeodeIOException("Throwing an exception");
        case ExceptionType.System:
          throw new IOException("Throwing an exception");
        case ExceptionType.GeodeGeode:
          throw new GeodeIOException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.GeodeSystem:
          throw new CacheServerException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
        case ExceptionType.SystemGeode:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.SystemSystem:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
      }
    }

    public void ToData(DataOutput output)
    {
      output.WriteInt32(m_struct.First);
      output.WriteInt64(m_struct.Second);
      switch (m_exType)
      {
        case ExceptionType.None:
          break;
        case ExceptionType.Geode:
          throw new GeodeIOException("Throwing an exception");
        case ExceptionType.System:
          throw new IOException("Throwing an exception");
        case ExceptionType.GeodeGeode:
          throw new GeodeIOException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.GeodeSystem:
          throw new CacheServerException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
        case ExceptionType.SystemGeode:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new CacheServerException("This is an inner exception"));
        case ExceptionType.SystemSystem:
          throw new ApplicationException("Throwing an exception with inner " +
            "exception", new IOException("This is an inner exception"));
      }
    }

    public UInt64 ObjectSize
    {
      get
      {
        return (UInt32)(sizeof(Int32) + sizeof(Int64));
      }
    }

    public String Type
    {
      get { return this.GetType().ToString(); }
    }
    #endregion

    public static ISerializable CreateDeserializable()
    {
      return new OtherType2();
    }

    public override int GetHashCode()
    {
      return m_struct.First.GetHashCode() ^ m_struct.Second.GetHashCode();
    }

    public override bool Equals(object obj)
    {
      OtherType2 ot = obj as OtherType2;
      if (ot != null)
      {
        return (m_struct.Equals(ot.m_struct));
      }
      return false;
    }

  }

  [Trait("Category", "Integration")]
  public class SerializationTests : IDisposable
    {
        private GeodeServer GeodeServer;
        private CacheXml CacheXml;
        private Cache Cache;

        public SerializationTests()
        {
            GeodeServer = new GeodeServer();
        }

        public void Dispose()
        {
            try
            {
                if (null != Cache)
                {
                    Cache.Close();
                }
            }
            finally
            {
                try
                {
                    if (null != CacheXml)
                    {
                        CacheXml.Dispose();
                    }
                }
                finally
                {
                    GeodeServer.Dispose();
                }
            }
        }


        private void putAndCheck(IRegion<object, object> region, object key, object value)
        {
          region[key] = value;
          var retrievedValue = region[key];
          Assert.Equal(value, retrievedValue);
        }

        private void putAndCheckCustom(IRegion<object, object> region, object key, object value)
        {
          region[key] = value;
          var retrievedValue = region[key];
          Assert.True(value.Equals(retrievedValue), "Got unexpected value");
        }
    [Fact]
      public void BuiltInSerializableTypes()
      {
          var cacheFactory = new CacheFactory()
              .Set("log-level", "none");
          var cache = cacheFactory.Create();

          var poolFactory = cache.GetPoolFactory()
              .AddLocator("localhost", GeodeServer.LocatorPort);
          poolFactory.Create("pool");

          var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
              .SetPoolName("pool");
          var region = regionFactory.Create<object, object>("testRegion");
          Assert.NotNull(region);

          putAndCheck(region, "CacheableString", "foo");
          putAndCheck(region, "CacheableByte", (Byte)8);
          putAndCheck(region, "CacheableInt16", (Int16)16);
          putAndCheck(region, "CacheableInt32", (Int32)32);
          putAndCheck(region, "CacheableInt64", (Int64)64);
          putAndCheck(region, "CacheableBoolean", (Boolean)true);
          putAndCheck(region, "CacheableCharacter", 'c');
          putAndCheck(region, "CacheableDouble", (Double)1.5);
          putAndCheck(region, "CacheableFloat", (float)2.5);

          putAndCheck(region, "CacheableStringArray", new String[] { "foo", "bar" });
          putAndCheck(region, "CacheableBytes", new Byte[] { 8, 8 });
          putAndCheck(region, "CacheableInt16Array", new Int16[] { 16, 16 });
          putAndCheck(region, "CacheableInt32Array", new Int32[] { 32, 32 });
          putAndCheck(region, "CacheableInt64Array", new Int64[] { 64, 64 });
          putAndCheck(region, "CacheableBooleanArray", new Boolean[] { true, false });
          putAndCheck(region, "CacheableCharacterArray", new Char[] { 'c', 'a' });
          putAndCheck(region, "CacheableDoubleArray", new Double[] { 1.5, 1.7 });
          putAndCheck(region, "CacheableFloatArray", new float[] { 2.5F, 2.7F });

          putAndCheck(region, "CacheableDate", new DateTime());

          putAndCheck(region, "CacheableHashMap", new Dictionary<int, string>() { { 1, "one" }, { 2, "two" } });
          putAndCheck(region, "CacheableHashTable", new Hashtable() { { 1, "one" }, { 2, "two" } });
          putAndCheck(region, "CacheableVector", new ArrayList() { "one", "two" });
          putAndCheck(region, "CacheableArrayList", new List<string>() { "one", "two" });
          putAndCheck(region, "CacheableLinkedList", new LinkedList<object>(new string[] { "one", "two" }));
          putAndCheck(region, "CacheableStack", new Stack<object>(new string[] { "one", "two" }));

          {
              var cacheableHashSet = new CacheableHashSet();
              cacheableHashSet.Add("one");
              cacheableHashSet.Add("two");
              putAndCheck(region, "CacheableHashSet", cacheableHashSet);
          }

          {
              var cacheableLinkedHashSet = new CacheableLinkedHashSet();
              cacheableLinkedHashSet.Add("one");
              cacheableLinkedHashSet.Add("two");
              putAndCheck(region, "CacheableLinkedHashSet", cacheableLinkedHashSet);
          }

          cache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
          putAndCheck(region, "PdxType", new PdxType());

          cache.Close();
        }

      [Fact]
      public void PutGetCustomSerializableTypes()
      {

        var cacheFactory = new CacheFactory()
            .Set("log-level", "none");
        var cache = cacheFactory.Create();

        var poolFactory = cache.GetPoolFactory()
            .AddLocator("localhost", GeodeServer.LocatorPort);
        poolFactory.Create("pool");

        var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
            .SetPoolName("pool");
        var region = regionFactory.Create<object, object>("testRegion");
        Assert.NotNull(region);

        cache.TypeRegistry.RegisterType(OtherType.CreateDeserializable, 0x9C);
        cache.TypeRegistry.RegisterType(OtherType2.CreateDeserializable, 0x8C);

        var otherType = new OtherType(64, 32);
        var otherType2 = new OtherType2(65, 66);

        putAndCheck(region, "OtherType", otherType);
        putAndCheck(region, "OtherType2", otherType2);
      }
  }

}
