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
using System.Linq;
using Xunit;
using PdxTests;
using System.Collections;
using System.Collections.Generic;
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
	public class TestClassA : IDataSerializable
	{
		public int Id { get; set; }
		public string Name { get; set; }
		public short NumSides { get; set; }

		// A default constructor is required for deserialization
		public TestClassA() { }

		public TestClassA(int id, string name, short numSides)
		{
			Id = id;
			Name = name;
			NumSides = numSides;
		}

		public override string ToString()
		{
			return string.Format("TestClassA: [{0}, {1}, {2}]", Id, Name, NumSides);
		}

		public void ToData(DataOutput output)
		{
			output.WriteInt32(Id);
			output.WriteUTF(Name);
			output.WriteInt16(NumSides);
		}

		public void FromData(DataInput input)
		{
			Id = input.ReadInt32();
			Name = input.ReadUTF();
			NumSides = input.ReadInt16();
		}

		public ulong ObjectSize
		{
			get { return 0; }
		}

		public static ISerializable CreateDeserializable()
		{
			return new TestClassA();
		}

		public override bool Equals(object obj)
		{
            TestClassA other = (TestClassA)obj;

            if (other != null) {
			    if (Id == other.Id &&
				    Name == other.Name &&
				    NumSides == other.NumSides)
				    return true;
                else
                    return false;
            }
			else
				return false;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
	}

	public class TestClassB : IDataSerializable
	{
		public int Width { get; set; }
		public int Height { get; set; }
		public string Name { get; set; }

		// A default constructor is required for deserialization
		public TestClassB() { }

		public TestClassB(int width, int height, string name)
		{
			Width = width;
			Height = height;
			Name = name;
		}

		public override string ToString()
		{
			return string.Format("TestClassB: [{0}, {1}, {2}]", Width, Height, Name);
		}

		public void ToData(DataOutput output)
		{
			output.WriteInt32(Width);
			output.WriteInt32(Height);
			output.WriteUTF(Name);
		}

		public void FromData(DataInput input)
		{
			Width = input.ReadInt32();
			Height = input.ReadInt32();
			Name = input.ReadUTF();
		}

		public ulong ObjectSize
		{
			get { return 0; }
		}

		public static ISerializable CreateDeserializable()
		{
			return new TestClassB();
		}

		public override bool Equals(object obj)
		{
			var other = (TestClassB)obj;

            if (other != null) {
			    if (Width == other.Width &&
				    Name == other.Name &&
				    Height == other.Height)
				    return true;
                else
                    return false;
            }
			else
				return false;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
	}

	public class TestClassC : IDataSerializable
	{
		public string Color { get; set; }
		public string Make { get; set; }
		public int Year { get; set; }

		// A default constructor is required for deserialization
		public TestClassC() { }

		public TestClassC(string color, string make, int year)
		{
			Color = color;
			Make = make;
			Year = year;
		}

		public override string ToString()
		{
			return string.Format("TestClassC: [{0}, {1}, {2}]", Color, Make, Year);
		}

		public void ToData(DataOutput output)
		{
			output.WriteUTF(Color);
			output.WriteUTF(Make);
			output.WriteInt32(Year);
		}

		public void FromData(DataInput input)
		{
			Color = input.ReadUTF();
			Make = input.ReadUTF();
			Year = input.ReadInt32();
		}

		public ulong ObjectSize
		{
			get { return 0; }
		}

		public static ISerializable CreateDeserializable()
		{
			return new TestClassC();
		}

		public override bool Equals(object obj)
		{
			var other = (TestClassC)obj;

			if (Color == other.Color &&
				Make == other.Make &&
				Year == other.Year)
				return true;
			else
				return false;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
	}

	public class CompositeClass : IDataSerializable
    {
        private TestClassA testclassA;
		private List<TestClassB> testclassBs;
		private List<TestClassC> testclassCs;

		public CompositeClass()
		{
			testclassA = new TestClassA();
			testclassBs = new List<TestClassB>();
			testclassCs = new List<TestClassC>();
		}

		public TestClassA A
		{
			get
			{
				return testclassA;
			}
			set
			{
				testclassA = value;
			}
		}

		public List<TestClassB> Bs
		{
			get
			{
				return testclassBs;
			}
			set
			{
				testclassBs = value;
			}
		}

		public List<TestClassC> Cs
		{
			get
			{
				return testclassCs;
			}
			set
			{
				testclassCs = value;
			}
		}

		public void ToData(DataOutput output)
		{
			testclassA.ToData(output);
			output.WriteObject(testclassBs);
			output.WriteObject(testclassCs);
		}

		public void FromData(DataInput input)
		{
			testclassA.FromData(input);

			List<object> bs = (List<object>)input.ReadObject();
			foreach (var obj in bs)
			{
				Bs.Add(obj as TestClassB);
			}
				
			List<object> cs = (List<object>)input.ReadObject();
			foreach (var obj in cs)
			{
				Cs.Add(obj as TestClassC);
			}
		}

		public ulong ObjectSize
		{
			get { return 0; }
		}

		public static ISerializable CreateDeserializable()
		{
			return new CompositeClass();
		}

		public override bool Equals(object obj)
		{
			var other = (CompositeClass)obj;

			if (testclassA.Equals(other.testclassA)
				&& testclassBs.Equals(other.testclassBs)
				&& testclassCs.Equals(other.testclassCs))
				return true;
			else
				return false;
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}
    }

	public class Order : IDataSerializable
    {
        public int OrderId { get; set; }
        public string Name { get; set; }
        public short Quantity { get; set; }

        // A default constructor is required for deserialization
        public Order() { }

        public Order(int orderId, string name, short quantity)
        {
            OrderId = orderId;
            Name = name;
            Quantity = quantity;
        }

        public override string ToString()
        {
            return string.Format("Order: [{0}, {1}, {2}]", OrderId, Name, Quantity);
        }

        public void ToData(DataOutput output)
        {
            output.WriteInt32(OrderId);
            output.WriteUTF(Name);
            output.WriteInt16(Quantity);
        }

        public void FromData(DataInput input)
        {
            OrderId = input.ReadInt32();
            Name = input.ReadUTF();
            Quantity = input.ReadInt16();
        }

        public ulong ObjectSize
        {
            get { return 0; }
        }

        public static ISerializable CreateDeserializable()
        {
            return new Order();
        }

    }

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
    public class SerializationTests : TestBase
    {
        public SerializationTests(ITestOutputHelper testOutputHelper) : base(testOutputHelper)

        {
        }

        private void putAndCheck(IRegion<object, object> region, object key, object value)
        {
            region[key] = value;
            var retrievedValue = region[key];
            Assert.Equal(value, retrievedValue);
        }


        [Fact]
        public void BuiltInSerializableTypes()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("testRegion")
                    .withType("REPLICATE")
                    .execute());

                var cache = cluster.CreateCache();

                var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("default")
                    .Create<object, object>("testRegion");
                Assert.NotNull(region);

                putAndCheck(region, "CacheableInt32", (Int32)32);
                putAndCheck(region, "CacheableString", "foo");
                putAndCheck(region, "CacheableByte", (Byte)8);
                putAndCheck(region, "CacheableInt16", (Int16)16);
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
            }
        }

        [Fact]
        public void PutGetCustomSerializableTypes()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("testRegion")
                    .withType("REPLICATE")
                    .execute());
                
                var cache = cluster.CreateCache();

                cache.TypeRegistry.RegisterType(Order.CreateDeserializable, 0x42);

                var orderRegion = cache.CreateRegionFactory(RegionShortcut.PROXY)
                  .SetPoolName("default")
                  .Create<int, Order>("testRegion");
                Assert.NotNull(orderRegion);

                const int orderKey = 65;
                var order = new Order(orderKey, "Donuts", 12);

                orderRegion.Put(orderKey, order, null);
                var orderRetrieved = orderRegion.Get(orderKey, null);
                Assert.Equal(order.ToString(), orderRetrieved.ToString());

                cache.Close();
            }
        }

        [Fact]
        public void ClassAsKey()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.Equal(cluster.Start(), true);
                Assert.Equal(0, cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute());

                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("testRegion")
                    .withType("REPLICATE")
                    .execute());

                Assert.Equal(0, cluster.Gfsh.executeFunction()
                    .withId("InstantiateDataSerializable")
                    .withMember("ClassAsKey_server_0")
                    .execute());

                var cache = cluster.CreateCache();

                cache.TypeRegistry.RegisterType(PositionKey.CreateDeserializable, 21);
                cache.TypeRegistry.RegisterType(Position.CreateDeserializable, 22);

                var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
                  .SetPoolName("default")
                  .Create<PositionKey, Position>("testRegion");
                Assert.NotNull(region);

                var key1 = new PositionKey(1000);
                var key2 = new PositionKey(1000000);
                var key3 = new PositionKey(1000000000);

                var pos1 = new Position("GOOG", 23);
                var pos2 = new Position("IBM", 37);
                var pos3 = new Position("PVTL", 101);

                region.Put(key1, pos1);
                region.Put(key2, pos2);
                region.Put(key3, pos3);

                var res1 = region.Get(key1);
                var res2 = region.Get(key2);
                var res3 = region.Get(key3);

                Assert.True(
                    res1.SecId == pos1.SecId &&
                    res1.SharesOutstanding == pos1.SharesOutstanding);
                Assert.True(
                    res2.SecId == pos2.SecId &&
                    res2.SharesOutstanding == pos2.SharesOutstanding);
                Assert.True(
                    res3.SecId == pos3.SecId &&
                    res3.SharesOutstanding == pos3.SharesOutstanding);

                cache.Close();
            }
        }

        [Fact]
		public void CompositeClassWithClassAsKey()
		{
			using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
			{
				Assert.Equal(cluster.Start(), true);
				Assert.Equal(0, cluster.Gfsh.deploy()
					.withJar(Config.JavaobjectJarPath)
					.execute());

				Assert.Equal(0, cluster.Gfsh.create()
					.region()
					.withName("testRegion")
					.withType("REPLICATE")
					.execute());

				Assert.Equal(0, cluster.Gfsh.executeFunction()
					.withId("InstantiateDataSerializable")
					.withMember("CompositeClassWithClassAsKey_server_0")
					.execute());

				var cache = cluster.CreateCache();

				cache.TypeRegistry.RegisterType(PositionKey.CreateDeserializable, 21);
				cache.TypeRegistry.RegisterType(TestClassA.CreateDeserializable, 100);
				cache.TypeRegistry.RegisterType(TestClassB.CreateDeserializable, 101);
				cache.TypeRegistry.RegisterType(TestClassC.CreateDeserializable, 102);
				cache.TypeRegistry.RegisterType(CompositeClass.CreateDeserializable, 125);

				var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
				  .SetPoolName("default")
				  .Create<PositionKey, CompositeClass>("testRegion");
				Assert.NotNull(region);

				var key1 = new PositionKey(23);
				var key2 = new PositionKey(37);
				var key3 = new PositionKey(38);

                ICollection<PositionKey> keys = new List<PositionKey>();
				keys.Add(key1);
				keys.Add(key2);
				keys.Add(key3);

                // First CompositeClass

                var cc1 = new CompositeClass();

				cc1.A = new TestClassA(1, "Square", 4);

				cc1.Bs = new List<TestClassB>() {
					new TestClassB(10, 20, "Brick"),
					new TestClassB(11, 21, "Block"),
					new TestClassB(100, 200, "Stone")};

				cc1.Cs = new List<TestClassC>() {
					new TestClassC("Red", "Volvo", 2010),
					new TestClassC("White", "Toyota", 2011),
					new TestClassC("Blue", "Nissan", 2012)};

                // Second CompositeClass

                var cc2 = new CompositeClass();

				cc2.A = new TestClassA(1, "Triangle", 3);

				cc2.Bs = new List<TestClassB>() {
					new TestClassB(100, 200, "BiggerBrick"),
					new TestClassB(110, 210, "BiggerBlock"),
					new TestClassB(1000, 2000, "BiggerStone")};

				cc2.Cs = new List<TestClassC>() {
				new TestClassC("Gray", "Volvo", 2010),
				new TestClassC("Silver", "Toyota", 2011),
				new TestClassC("Black", "Nissan", 2012)};

                // Third CompositeClass

                var cc3 = new CompositeClass();

                cc3.A = new TestClassA(1, "Hexagon", 6);

                cc3.Bs = new List<TestClassB>() {
                new TestClassB(1000, 2000, "BiggestBrick"),
                new TestClassB(1100, 2100, "BiggestBlock"),
                new TestClassB(10000, 20000, "BiggestStone")};

                cc3.Cs = new List<TestClassC>() {
                new TestClassC("Orange", "Volvo", 2010),
                new TestClassC("Yellow", "Toyota", 2011),
                new TestClassC("Purple", "Nissan", 2012)};

                region.Put(key1, cc1);
				region.Put(key2, cc2);
				region.Put(key3, cc3);

                Dictionary<PositionKey, CompositeClass> ccs = new Dictionary<PositionKey, CompositeClass>();
				Dictionary<PositionKey, Exception> exceptions = new Dictionary<PositionKey, Exception>();

				region.GetAll(keys, ccs, exceptions, false);

				Assert.True(cc1.A.Equals(ccs[key1].A) &&
							Enumerable.SequenceEqual(cc1.Bs, ccs[key1].Bs) &&
							Enumerable.SequenceEqual(cc1.Cs, ccs[key1].Cs));
				Assert.True(cc2.A.Equals(ccs[key2].A) &&
							Enumerable.SequenceEqual(cc2.Bs, ccs[key2].Bs) &&
							Enumerable.SequenceEqual(cc2.Cs, ccs[key2].Cs));
                Assert.True(cc3.A.Equals(ccs[key3].A) &&
                            Enumerable.SequenceEqual(cc3.Bs, ccs[key3].Bs) &&
                            Enumerable.SequenceEqual(cc3.Cs, ccs[key3].Cs));

                cache.Close();
			}
		}
	}
}