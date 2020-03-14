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
using Xunit.Abstractions;
using System.Reflection;

namespace Apache.Geode.Client.IntegrationTests
{
    public class AddressR
    {
        int _aptNumber;
        string _street;
        string _city;

        public AddressR()
        { }
        public override string ToString()
        {
            return _aptNumber + " :" + _street + " : " + _city;
        }
        public AddressR(int aptN, string street, string city)
        {
            _aptNumber = aptN;
            _street = street;
            _city = city;
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            AddressR other = obj as AddressR;
            if (other == null)
                return false;
            if (_aptNumber == other._aptNumber
                && _street == other._street
                  && _city == other._city)
                return true;
            return false;
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }

    public class PdxTypesReflectionTest
    {
        char m_char;
        bool m_bool;
        sbyte m_byte;
        sbyte m_sbyte;
        short m_int16;
        short m_uint16;
        Int32 m_int32;
        Int32 m_uint32;
        long m_long;
        Int64 m_ulong;
        float m_float;
        double m_double;

        string m_string;

        bool[] m_boolArray;
        byte[] m_byteArray;
        byte[] m_sbyteArray;

        char[] m_charArray;

        DateTime m_dateTime;

        Int16[] m_int16Array;
        Int16[] m_uint16Array;

        Int32[] m_int32Array;
        Int32[] m_uint32Array;

        long[] m_longArray;
        Int64[] m_ulongArray;

        float[] m_floatArray;
        double[] m_doubleArray;

        byte[][] m_byteByteArray;

        string[] m_stringArray;

        List<object> m_arraylist = new List<object>();
        IDictionary<object, object> m_map = new Dictionary<object, object>();
        Hashtable m_hashtable = new Hashtable();
        ArrayList m_vector = new ArrayList();

        CacheableHashSet m_chs = CacheableHashSet.Create();
        CacheableLinkedHashSet m_clhs = CacheableLinkedHashSet.Create();

        byte[] m_byte252 = new byte[252];
        byte[] m_byte253 = new byte[253];
        byte[] m_byte65535 = new byte[65535];
        byte[] m_byte65536 = new byte[65536];

        pdxEnumTest m_pdxEnum = pdxEnumTest.pdx3;
        AddressR[] m_address = new AddressR[10];

        LinkedList<Object> m_LinkedList = new LinkedList<Object>();

        public void Init()
        {
            m_char = 'C';
            m_bool = true;
            m_byte = 0x74;
            m_sbyte = 0x67;
            m_int16 = 0xab;
            m_uint16 = 0x2dd5;
            m_int32 = 0x2345abdc;
            m_uint32 = 0x2a65c434;
            m_long = 324897980;
            m_ulong = 238749898;
            m_float = 23324.324f;
            m_double = 3243298498d;

            m_string = "gfestring";

            m_boolArray = new bool[] { true, false, true };
            m_byteArray = new byte[] { 0x34, 0x64 };
            m_sbyteArray = new byte[] { 0x34, 0x64 };

            m_charArray = new char[] { 'c', 'v' };

            long ticks = 634460644691540000L;
            m_dateTime = new DateTime(ticks);

            m_int16Array = new short[] { 0x2332, 0x4545 };
            m_uint16Array = new short[] { 0x3243, 0x3232 };

            m_int32Array = new int[] { 23, 676868, 34343, 2323 };
            m_uint32Array = new int[] { 435, 234324, 324324, 23432432 };

            m_longArray = new long[] { 324324L, 23434545L };
            m_ulongArray = new Int64[] { 3245435, 3425435 };

            m_floatArray = new float[] { 232.565f, 2343254.67f };
            m_doubleArray = new double[] { 23423432d, 4324235435d };

            m_byteByteArray = new byte[][]{new byte[] {0x23},
                                             new byte[]{0x34, 0x55}
                                              };

            m_stringArray = new string[] { "one", "two" };

            m_arraylist = new List<object>();
            m_arraylist.Add(1);
            m_arraylist.Add(2);

            m_LinkedList = new LinkedList<Object>();
            m_LinkedList.AddFirst("Item1");
            m_LinkedList.AddLast("Item2");
            m_LinkedList.AddLast("Item3");


            m_map = new Dictionary<object, object>();
            m_map.Add(1, 1);
            m_map.Add(2, 2);

            m_hashtable = new Hashtable();
            m_hashtable.Add(1, "1111111111111111");
            m_hashtable.Add(2, "2222222222221111111111111111");

            m_vector = new ArrayList();
            m_vector.Add(1);
            m_vector.Add(2);
            m_vector.Add(3);

            m_chs.Add(1);
            m_clhs.Add(1);
            m_clhs.Add(2);
            m_pdxEnum = pdxEnumTest.pdx3;

            m_address = new AddressR[10];

            for (int i = 0; i < m_address.Length; i++)
            {
                m_address[i] = new AddressR(i, "street" + i, "city" + i);
            }
        }

        public PdxTypesReflectionTest()
        {


        }

        public PdxTypesReflectionTest(bool initialize)
        {
            if (initialize)
                Init();

        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;

            PdxTypesReflectionTest other = obj as PdxTypesReflectionTest;

            if (other == null)
                return false;

            this.checkEquality(other);
            return true;
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
        #region IPdxSerializable Members
        byte[][] compareByteByteArray(byte[][] baa, byte[][] baa2)
        {
            if (baa.Length == baa2.Length)
            {
                int i = 0;
                while (i < baa.Length)
                {
                    compareByteArray(baa[i], baa2[i]);
                    i++;
                }
                if (i == baa2.Length)
                    return baa2;
            }

            throw new IllegalStateException("Not got expected value for type: " + baa2.GetType().ToString());
        }
        bool compareBool(bool b, bool b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        bool[] compareBoolArray(bool[] a, bool[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        byte compareByte(byte b, byte b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        byte[] compareByteArray(byte[] a, byte[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        char[] compareCharArray(char[] a, char[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        List<object> compareCompareCollection(List<object> a, List<object> a2)
        {
            if (a.Count == a2.Count)
            {
                int i = 0;
                while (i < a.Count)
                {
                    if (!a[i].Equals(a2[i]))
                        break;
                    else
                        i++;
                }
                if (i == a2.Count)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }

        LinkedList<object> compareCompareCollection(LinkedList<object> a, LinkedList<object> a2)
        {
            if (a.Count == a2.Count)
            {
                LinkedList<Object>.Enumerator e1 = a.GetEnumerator();
                LinkedList<Object>.Enumerator e2 = a2.GetEnumerator();
                while (e1.MoveNext() && e2.MoveNext())
                {
                    if (!e1.Current.Equals(e2.Current))
                        break;
                }
                return a2;
            }
            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }

        DateTime compareData(DateTime b, DateTime b2)
        {
            //TODO:
            // return b;
            if ((b.Ticks / 10000L) == (b2.Ticks / 10000L))
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        Double compareDouble(Double b, Double b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        double[] compareDoubleArray(double[] a, double[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        float compareFloat(float b, float b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        float[] compareFloatArray(float[] a, float[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        Int16 compareInt16(Int16 b, Int16 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        Int32 compareInt32(Int32 b, Int32 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        Int64 compareInt64(Int64 b, Int64 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        Int32[] compareIntArray(Int32[] a, Int32[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        long[] compareLongArray(long[] a, long[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        Int16[] compareSHortArray(Int16[] a, Int16[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        sbyte compareSByte(sbyte b, sbyte b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        sbyte[] compareSByteArray(sbyte[] a, sbyte[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        string[] compareStringArray(string[] a, string[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        UInt16 compareUInt16(UInt16 b, UInt16 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        UInt32 compareUInt32(UInt32 b, UInt32 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        UInt64 compareUint64(UInt64 b, UInt64 b2)
        {
            if (b == b2)
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        UInt32[] compareUnsignedIntArray(UInt32[] a, UInt32[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        UInt64[] compareUnsignedLongArray(UInt64[] a, UInt64[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }
        UInt16[] compareUnsignedShortArray(UInt16[] a, UInt16[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (a[i] != a2[i])
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }

        T[] GenericCompare<T>(T[] a, T[] a2)
        {
            if (a.Length == a2.Length)
            {
                int i = 0;
                while (i < a.Length)
                {
                    if (!a[i].Equals(a2[i]))
                        break;
                    else
                        i++;
                }
                if (i == a2.Length)
                    return a2;
            }

            throw new IllegalStateException("Not got expected value for type: " + a2.GetType().ToString());
        }


        T GenericValCompare<T>(T b, T b2)
        {
            if (b.Equals(b2))
                return b;
            throw new IllegalStateException("Not got expected value for type: " + b2.GetType().ToString());
        }
        public void checkEquality(PdxTypesReflectionTest other)
        {
            byte[][] baa = other.m_byteByteArray;
            m_byteByteArray = compareByteByteArray(baa, m_byteByteArray);
            m_char = GenericValCompare(other.m_char, m_char);

            m_bool = GenericValCompare(other.m_bool, m_bool);
            m_boolArray = GenericCompare(other.m_boolArray, m_boolArray);

            m_byte = GenericValCompare(other.m_byte, m_byte);
            m_byteArray = GenericCompare(other.m_byteArray, m_byteArray);
            m_charArray = GenericCompare(other.m_charArray, m_charArray);

            List<object> tmpl = new List<object>();
            m_arraylist = compareCompareCollection(other.m_arraylist, m_arraylist);

            m_LinkedList = compareCompareCollection(other.m_LinkedList, m_LinkedList);

            IDictionary<object, object> tmpM = other.m_map;
            if (tmpM.Count != m_map.Count)
                throw new IllegalStateException("Not got expected value for type: " + m_map.GetType().ToString());

            Hashtable tmpH = other.m_hashtable;

            if (tmpH.Count != m_hashtable.Count)
                throw new IllegalStateException("Not got expected value for type: " + m_hashtable.GetType().ToString());

            ArrayList arrAl = other.m_vector;

            if (arrAl.Count != m_vector.Count)
                throw new IllegalStateException("Not got expected value for type: " + m_vector.GetType().ToString());

            CacheableHashSet rmpChs = other.m_chs;

            if (rmpChs.Count != m_chs.Count)
                throw new IllegalStateException("Not got expected value for type: " + m_chs.GetType().ToString());

            CacheableLinkedHashSet rmpClhs = other.m_clhs;

            if (rmpClhs.Count != m_clhs.Count)
                throw new IllegalStateException("Not got expected value for type: " + m_clhs.GetType().ToString());


            m_string = GenericValCompare(other.m_string, m_string);

            m_dateTime = compareData(other.m_dateTime, m_dateTime);

            m_double = GenericValCompare(other.m_double, m_double);

            m_doubleArray = GenericCompare(other.m_doubleArray, m_doubleArray);
            m_float = GenericValCompare(other.m_float, m_float);
            m_floatArray = GenericCompare(other.m_floatArray, m_floatArray);
            m_int16 = GenericValCompare(other.m_int16, m_int16);
            m_int32 = GenericValCompare(other.m_int32, m_int32);
            m_long = GenericValCompare(other.m_long, m_long);
            m_int32Array = GenericCompare(other.m_int32Array, m_int32Array);
            m_longArray = GenericCompare(other.m_longArray, m_longArray);
            m_int16Array = GenericCompare(other.m_int16Array, m_int16Array);
            m_sbyte = GenericValCompare(other.m_sbyte, m_sbyte);
            m_sbyteArray = GenericCompare(other.m_sbyteArray, m_sbyteArray);
            m_stringArray = GenericCompare(other.m_stringArray, m_stringArray);
            m_uint16 = GenericValCompare(other.m_uint16, m_uint16);
            m_uint32 = GenericValCompare(other.m_uint32, m_uint32);
            m_ulong = GenericValCompare(other.m_ulong, m_ulong);
            m_uint32Array = GenericCompare(other.m_uint32Array, m_uint32Array);
            m_ulongArray = GenericCompare(other.m_ulongArray, m_ulongArray);
            m_uint16Array = GenericCompare(other.m_uint16Array, m_uint16Array);

            byte[] ret = other.m_byte252;
            if (ret.Length != 252)
                throw new Exception("Array len 252 not found");

            ret = other.m_byte253;
            if (ret.Length != 253)
                throw new Exception("Array len 253 not found");

            ret = other.m_byte65535;
            if (ret.Length != 65535)
                throw new Exception("Array len 65535 not found");

            ret = other.m_byte65536;
            if (ret.Length != 65536)
                throw new Exception("Array len 65536 not found");
            if (other.m_pdxEnum != m_pdxEnum)
                throw new Exception("Pdx enum is not equal");
            //byte[] m_byte252 = new byte[252];
            //byte[] m_byte253 = new byte[253];
            //byte[] m_byte65535 = new byte[65535];
            //byte[] m_byte65536 = new byte[65536];
            AddressR[] otherA = other.m_address;
            for (int i = 0; i < m_address.Length; i++)
            {
                if (!m_address[i].Equals(otherA[i]))
                    throw new Exception("AddressR array is not equal " + i);
            }
        }



        #endregion
    }

    public class SerializePdx1
    {
        [PdxIdentityField] public int i1;
        public int i2;
        public string s1;
        public string s2;

        /*public static SerializePdx1 CreateDeserializable()
        {
          return new SerializePdx1(false);
        }*/

        public SerializePdx1()
        {
        }

        public SerializePdx1(bool init)
        {
            if (init)
            {
                i1 = 1;
                i2 = 2;
                s1 = "s1";
                s2 = "s2";
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            if (obj == this)
                return true;

            var other = obj as SerializePdx1;

            if (other == null)
                return false;

            if (i1 == other.i1
                && i2 == other.i2
                && s1 == other.s1
                && s2 == other.s2)
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }

    public class SerializePdx2
    {
        public string s0;
        [PdxIdentityField] public int i1;
        public int i2;
        public string s1;
        public string s2;

        public SerializePdx2()
        {
        }

        public override string ToString()
        {
            return i1 + i2 + s1 + s2;
        }

        public SerializePdx2(bool init)
        {
            if (init)
            {
                s0 = "s9999999999999999999999999999999999";
                i1 = 1;
                i2 = 2;
                s1 = "s1";
                s2 = "s2";
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            if (obj == this)
                return true;

            var other = obj as SerializePdx2;

            if (other == null)
                return false;

            if (s0 == other.s0
                && i1 == other.i1
                && i2 == other.i2
                && s1 == other.s1
                && s2 == other.s2)
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }

    public class BaseClass
    {
        //private readonly int _b1 = 1000;
        [NonSerialized]
        //private int _nonserialized = 1001;
        //private static int _static = 1002;
        private const int _const = 1003;

        private int _baseclassmember;

        public BaseClass()
        {
        }

        public BaseClass(bool init)
        {
            if (init)
            {
                _baseclassmember = 101;
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            var bc = obj as BaseClass;
            if (bc == null)
                return false;

            if (bc == this)
                return true;

            if (bc._baseclassmember == _baseclassmember)
            {
                return true;
            }

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public void ToData(IPdxWriter w)
        {
            w.WriteInt("_baseclassmember", _baseclassmember);
        }

        public void FromData(IPdxReader r)
        {
            _baseclassmember = r.ReadInt("_baseclassmember");
        }
    }

    public class AddressWithGuid
    {
        private static Guid oddGuid = new Guid("924243B5-9C2A-41d7-86B1-E0B905C7EED3");
        private static Guid evenGuid = new Guid("47AA8F17-FF6B-4a7d-B398-D83790977574");
        private string _street;
        private string _aptName;
        private int _flatNumber;
        private Guid _guid;

        public AddressWithGuid()
        {
        }

        public AddressWithGuid(int id)
        {
            _flatNumber = id;
            _aptName = id.ToString();
            _street = id.ToString() + "_street";
            if (id % 2 == 0)
                _guid = evenGuid;
            else
                _guid = oddGuid;
        }

        public override string ToString()
        {
            return _flatNumber + " " + _aptName + " " + _street + "  " + _guid.ToString();
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            var other = obj as AddressWithGuid;

            if (other == null)
                return false;

            if (_street == other._street &&
                _aptName == other._aptName &&
                _flatNumber == other._flatNumber &&
                _guid.Equals(other._guid))
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        //public void ToData(IPdxWriter w)
        //{
        //    w.WriteString("_street", _street);
        //    w.WriteString("_aptName", _aptName);
        //    w.WriteInt("_flatNumber", _flatNumber);
        //    w.WriteString("_guid", _guid.ToString());
        //}

        //public void FromData(IPdxReader r)
        //{
        //    _street = r.ReadString("_street");
        //    _aptName = r.ReadString("_aptName");
        //    _flatNumber = r.ReadInt("_flatNumber");
        //    var s = r.ReadString("_guid");
        //    _guid = new Guid(s);
        //}
    }

    public class SerializePdx3 : BaseClass
    {
        private string s0;
        [PdxIdentityField] private int identity;

        public short shortVal;
        public ushort ushortVal;
        public short[] shortArray;
        public ushort[] ushortArray;

        public int intVal;
        public uint uintVal;
        public int[] intArray;
        public uint[] uintArray;

        public long longVal;
        public ulong ulongVal;
        public long[] longArray;
        public ulong[] ulongArray;

        public string s1;
        public string s2;
        private SerializePdx2 nestedObject;
        private ArrayList _addressList;
        private AddressWithGuid _address;

        private Hashtable _hashTable;

        public SerializePdx3()
          : base()
        {
        }

        public SerializePdx3(bool init, int nAddress)
          : base(init)
        {
            if (init)
            {
                s0 = "s9999999999999999999999999999999999";
                identity = 1;

                shortVal = -2;
                ushortVal = 2;
                shortArray = new short[] { 1, 11, 111 };
                ushortArray = new ushort[] {(ushort)(Math.Pow(2,16)-1), (ushort)(Math.Pow(2,16)-11), (ushort)(Math.Pow(2,16)-111) };

                intVal = -22;
                uintVal = 22;
                intArray = new int[] {10, 110, 1110 };
                uintArray = new uint[] {(uint)(Math.Pow(2,32)-10), (uint)(Math.Pow(2,32)-110), (uint)(Math.Pow(2,32)-1110)};

                longVal = -222;
                ulongVal = 222;
                longArray = new long[] { 100, 1100, 11100 };
                ulongArray = new ulong[] { (ulong)(Math.Pow(2,64)-100), (ulong)(Math.Pow(2,64)-1100), (ulong)(Math.Pow(2, 64)-11100) };

                s1 = "s1";
                s2 = "s2";
                nestedObject = new SerializePdx2(true);

                _addressList = new ArrayList();
                _hashTable = new Hashtable();

                for (var i = 0; i < 10; i++)
                {
                    _addressList.Add(new AddressWithGuid(i));
                    _hashTable.Add(i, new SerializePdx2(true));
                }

                _address = new AddressWithGuid(nAddress);
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            if (obj == this)
                return true;

            var other = obj as SerializePdx3;

            if (other == null)
                return false;

            if (s0 == other.s0
                && identity == other.identity

                && shortVal == other.shortVal
                && ushortVal == other.ushortVal
                && shortArray == other.shortArray
                && ushortArray == other.ushortArray

                && intVal == other.intVal
                && uintVal == other.uintVal
                && intArray == other.intArray
                && uintArray == other.uintArray

                && longVal == other.longVal
                && ulongVal == other.ulongVal
                && longArray == other.longArray
                && ulongArray == other.ulongArray

                && s1 == other.s1
                && s2 == other.s2)
            {
                var ret = nestedObject.Equals(other.nestedObject);
                if (ret)
                {
                    if (_addressList.Count == 10 &&
                        _addressList.Count == other._addressList.Count //&&
                                                                       //_arrayOfAddress.Length == other._arrayOfAddress.Length &&
                                                                       //_arrayOfAddress[0].Equals(other._arrayOfAddress[0])
                    )
                    {
                        for (var i = 0; i < _addressList.Count; i++)
                        {
                            ret = _addressList[i].Equals(other._addressList[i]);
                            if (!ret)
                                return false;
                        }

                        if (_hashTable.Count != other._hashTable.Count)
                            return false;
                        foreach (DictionaryEntry de in _hashTable)
                        {
                            var otherHe = other._hashTable[de.Key];
                            ret = de.Value.Equals(otherHe);
                            if (!ret)
                                return false;
                        }

                        if (!_address.Equals(other._address))
                            return false;
                        return base.Equals(other);
                    }
                }
            }

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public new void ToData(IPdxWriter w)
        {
            base.ToData(w);
            w.WriteString("s0", s0);
            w.WriteInt("identity", identity);

            w.WriteShort("shortVal", shortVal);
            w.WriteUShort("ushortVal", ushortVal);
            w.WriteShortArray("shortArray", shortArray);
            w.WriteUShortArray("ushortArray", ushortArray);

            w.WriteInt("intVal", intVal);
            w.WriteUInt("uintVal", uintVal);
            w.WriteIntArray("intArray", intArray);
            w.WriteUIntArray("uintArray", uintArray);

            w.WriteLong("longVal", longVal);
            w.WriteULong("ulongVal", ulongVal);
            w.WriteLongArray("longArray", longArray);
            w.WriteULongArray("ulongArray", ulongArray);

            w.WriteString("s1", s1);
            w.WriteString("s2", s2);

            w.WriteObject("nestedObject", nestedObject);
            w.WriteObject("_addressList", _addressList);
            w.WriteObject("_address", _address);
            w.WriteObject("_hashTable", _hashTable);
        }

        public new void FromData(IPdxReader r)
        {
            base.FromData(r);
            s0 = r.ReadString("s0");
            identity = r.ReadInt("identity");

            shortVal = r.ReadShort("shortVal");
            ushortVal = r.ReadUShort("ushortVal");
            shortArray = r.ReadShortArray("shortArray");
            ushortArray = r.ReadUShortArray("ushortArray");

            intVal = r.ReadInt("intVal");
            uintVal = r.ReadUInt("uintVal");
            intArray = r.ReadIntArray("intArray");
            uintArray = r.ReadUIntArray("uintArray");

            longVal = r.ReadLong("longVal");
            ulongVal = r.ReadULong("ulongVal");
            longArray = r.ReadLongArray("longArray");
            ulongArray = r.ReadULongArray("ulongArray");

            s1 = r.ReadString("s1");
            s2 = r.ReadString("s2");
            nestedObject = (SerializePdx2)r.ReadObject("nestedObject");
            _addressList = (ArrayList)r.ReadObject("_addressList");
            _address = (AddressWithGuid)r.ReadObject("_address");
            _hashTable = (Hashtable)r.ReadObject("_hashTable");
        }
    }

    public class SerializePdx4 : BaseClass
    {
        private string s0;
        [PdxIdentityField] private int i1;
        public int i2;
        public string s1;
        public string s2;
        private SerializePdx2 nestedObject;
        private ArrayList _addressList;

        private AddressWithGuid[] _addressArray;

        //private int arrayCount = 10;
        public SerializePdx4()
          : base()
        {
        }

        public override string ToString()
        {
            return i1 + ":" + i2 + ":" + s1 + ":" + s2 + nestedObject.ToString() + " add: " + _addressList[0].ToString();
        }

        public SerializePdx4(bool init)
          : base(init)
        {
            if (init)
            {
                s0 = "s9999999999999999999999999999999999";
                i1 = 1;
                i2 = 2;
                s1 = "s1";
                s2 = "s2";
                nestedObject = new SerializePdx2(true);

                _addressList = new ArrayList();
                _addressArray = new AddressWithGuid[10];

                for (var i = 0; i < 10; i++)
                {
                    _addressList.Add(new AddressWithGuid(i));
                    _addressArray[i] = new AddressWithGuid(i);
                }
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;
            if (obj == this)
                return true;

            var other = obj as SerializePdx4;

            if (other == null)
                return false;

            if (s0 == other.s0
                && i1 == other.i1
                && i2 == other.i2
                && s1 == other.s1
                && s2 == other.s2)
            {
                var ret = nestedObject.Equals(other.nestedObject);
                if (ret)
                {
                    if (_addressList.Count == other._addressList.Count &&
                        _addressList[0].Equals(other._addressList[0]))
                    {
                        for (var i = 0; i < _addressList.Count; i++)
                        {
                            ret = _addressList[i].Equals(other._addressList[i]);
                            if (!ret)
                                return false;
                        }

                        for (var i = 0; i < _addressArray.Length; i++)
                        {
                            ret = _addressArray[i].Equals(other._addressArray[i]);
                            if (!ret)
                                return false;
                        }

                        return base.Equals(other);
                    }
                }
            }

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }

    public class PdxFieldTest
    {
        private string _notInclude = "default_value";
        private int _nameChange;
        private int _identityField;

        public PdxFieldTest()
        {
        }

        public string NotInclude
        {
            set { _notInclude = "default_value"; }
        }

        public PdxFieldTest(bool init)
        {
            if (init)
            {
                _notInclude = "valuechange";
                _nameChange = 11213;
                _identityField = 1038193;
            }
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;

            var other = obj as PdxFieldTest;

            if (other == null)
                return false;

            if (_notInclude == other._notInclude
                && _nameChange == other._nameChange
                && _identityField == other._identityField)
                return true;


            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }
    }

    public class AutoSerializerEx : ReflectionBasedAutoSerializer
    {
        public override bool IsIdentityField(FieldInfo fi, Type type)
        {
            if (fi.Name == "_identityField")
                return true;
            return base.IsIdentityField(fi, type);
        }

        public override string GetFieldName(FieldInfo fi, Type type)
        {
            if (fi.Name == "_nameChange")
                return fi.Name + "NewName";

            return fi.Name;
        }

        public override bool IsFieldIncluded(FieldInfo fi, Type type)
        {
            if (fi.Name == "_notInclude")
                return false;
            return base.IsFieldIncluded(fi, type);
        }

        public override FieldType GetFieldType(FieldInfo fi, Type type)
        {
            if (fi.FieldType.Equals(Type.GetType("System.Guid")))
                return FieldType.STRING;
            return base.GetFieldType(fi, type);
        }

        public override object WriteTransform(FieldInfo fi, Type type, object originalValue)
        {
            if (fi.FieldType.Equals(Type.GetType("System.Guid")))
            {
                //writer.WriteField(fi.Name, fi.GetValue(o).ToString(), Type.GetType("System.String"));
                return originalValue.ToString();
            }
            else
                return base.WriteTransform(fi, type, originalValue);
        }

        public override object ReadTransform(FieldInfo fi, Type type, object serializeValue)
        {
            if (fi.FieldType.Equals(Type.GetType("System.Guid")))
            {
                var g = new Guid((string)serializeValue);

                //fi.SetValue(o, g);
                return g;
            }
            else
                return base.ReadTransform(fi, type, serializeValue);
        }

        /*public override void SerializeField(object o, FieldInfo fi, IPdxWriter writer)
        {
          if (fi.FieldType.Equals(Type.GetType("System.Guid")))
          {
            writer.WriteField(fi.Name, fi.GetValue(o).ToString(), Type.GetType("System.String"));
          }
          else
          base.SerializeField(o, fi, writer);
        }*/

        /* public override object DeserializeField(object o, FieldInfo fi, IPdxReader reader)
         {
           if (fi.FieldType.Equals(Type.GetType("System.Guid")))
           {
             string gStr = (string)reader.ReadField(fi.Name, Type.GetType("System.String"));
             Guid g = new Guid(gStr);

             //fi.SetValue(o, g);
             return g;
           }
           else
             return base.DeserializeField(o, fi, reader);
         }*/
    }

    [Trait("Category", "Integration")]
    public class AutoSerializationTests : TestBase
    {
        public AutoSerializationTests(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void AutoPutGet()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("testRegion")
                    .withType("REPLICATE")
                    .execute());

                var properties = new Dictionary<string, string>()
                    {
                        { "log-level", "debug" },
                        { "log-file", "c:/temp/autoserializertest.log" }
                    };

                var cache = cluster.CreateCache(properties);

                var region = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("default")
                    .Create<object, object>("testRegion");
                Assert.NotNull(region);

                // Register the reflectionbased serializer
                cache.TypeRegistry.PdxSerializer = new AutoSerializerEx();

                for (var i = 0; i < 1; i++)
                {
                    object put = new SerializePdx1(true);

                    region[i] = put;

                    var ret = region[i];

                    Assert.Equal(put, ret);

                    put = new SerializePdx2(true);
                    region[i + 10] = put;


                    ret = region[i + 10];

                    Assert.Equal(put, ret);

                    put = new PdxTypesReflectionTest(true);
                    region[i + 20] = put;


                    ret = region[i + 20];

                    Assert.Equal(put, ret);

                    put = new SerializePdx3(true, i % 2);
                    region[i + 30] = put;


                    ret = region[i + 30];

                    Assert.Equal(put, ret);

                    put = new SerializePdx4(true);
                    region[i + 40] = put;


                    ret = region[i + 40];

                    Assert.Equal(put, ret);

                    var p1 = region[i + 30];
                    var p2 = region[i + 40];

                    Assert.True(p1 != p2, "This should NOt be equals");

                    var pft = new PdxFieldTest(true);
                    region[i + 50] = pft;
                    ret = region[i + 50];

                    Assert.NotEqual(pft, ret);

                    pft.NotInclude = "default_value";
                    Assert.Equal(pft, ret);
                }
            }
        }

        [Fact]
        public void AutoPutAll()
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

                // Register the reflectionbased serializer
                cache.TypeRegistry.PdxSerializer = new AutoSerializerEx();

                IDictionary<object, object> putall = new Dictionary<object, object>();
                putall.Add(100, new SerializePdx3(true, 0));
                putall.Add(200, new SerializePdx3(true, 1));
                putall.Add(300, new SerializePdx4(true));
                region.PutAll(putall);

                //doGetWithPdxSerializerC2R();
                //doQueryTest();
            }
        }

        [Fact]
        public void DataType()
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

                // Register the reflectionbased serializer
                //cache.TypeRegistry.PdxSerializer = new AutoSerializerEx();
                cache.TypeRegistry.PdxSerializer = new ReflectionBasedAutoSerializer();

                int[] intArray = new int[] {1,11,111};
                uint[] uintArray = new uint[] { 7,77,777 };

                //region.Put("intArray", intArray);
                //var resultIntArray = region.Get("intArray");
                //Assert.Equal(intArray, resultIntArray);

                region.Put("uintArray", uintArray);
                var resultUIntArray = region.Get("uintArray");
                Assert.Equal(uintArray, resultUIntArray);

                //doGetWithPdxSerializerC2R();
                //doQueryTest();
            }
        }
    }
}
