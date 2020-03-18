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
using System.Collections;
using System.IO;
using PdxTests;
using System.Reflection;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using DUnitFramework;
  using Client;
  using Region = IRegion<object, object>;

  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  internal class ThinClientPdxTests3 : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1, m_client2, m_client3, m_client4;

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      m_client3 = new UnitProcess();
      m_client4 = new UnitProcess();
      return new ClientBase[] {m_client1, m_client2, m_client3, m_client4};
    }

    [TestFixtureTearDown]
    public override void EndTests()
    {
      CacheHelper.StopJavaServers();
      base.EndTests();
    }

    [TearDown]
    public override void EndTest()
    {
      try
      {
        m_client1.Call(DestroyRegions);
        m_client2.Call(DestroyRegions);
        CacheHelper.ClearEndpoints();
        CacheHelper.ClearLocators();
      }
      finally
      {
        CacheHelper.StopJavaServers();
        CacheHelper.StopJavaLocators();
      }

      base.EndTest();
    }

    private void cleanup()
    {
      {
        CacheHelper.SetExtraPropertiesFile(null);
        if (m_clients != null)
        {
          foreach (var client in m_clients)
          {
            try
            {
              client.Call(CacheHelper.Close);
            }
            catch (System.Runtime.Remoting.RemotingException)
            {
            }
            catch (System.Net.Sockets.SocketException)
            {
            }
          }
        }

        CacheHelper.Close();
      }
    }

    private Assembly m_pdxVesionOneAsm;
    private Assembly m_pdxVesionTwoAsm;

    #region Basic merge three PDxType3

    private void initializePdxAssemblyOne3(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOne3);

      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");

      var ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeOne3()
    {
      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxAssemblyTwo3(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwo3);
      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");

      var ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeTwo3()
    {
      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    #endregion

    private void pdxPut()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      region0["pdxput"] = new PdxType();
      region0["pdxput2"] = new ParentPdx(1);
    }

    private void initializePdxAssemblyForEqualTestv1()
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeForEqualv1);

      // Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestEquals");

      //object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
    }

    private IPdxSerializable registerPdxTypeForEqualv1()
    {
      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestEquals");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxAssemblyForEqualTestv2()
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeForEqualv2);

      // Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestEquals");

      //object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
    }

    private IPdxSerializable registerPdxTypeForEqualv2()
    {
      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestEquals");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void pdxVersion1Put()
    {
      initializePdxAssemblyForEqualTestv1();

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestEquals");

      var np = pt.InvokeMember("TestEquals", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;
    }

    private void pdxVersion2Put()
    {
      initializePdxAssemblyForEqualTestv2();

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestEquals");
      var np = pt.InvokeMember("TestEquals", BindingFlags.CreateInstance, null, null, null);
      region0[2] = np;
    }

    private void getVersionObject()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var v1 = (IPdxInstance) region0[1];
      var v2 = (IPdxInstance) region0[2];

      Assert.AreEqual(v1, v2, "both pdxinstance should be equal");
    }

    private void runPdxVersionClassesEqualTest()
    {
      Util.Log("Starting iteration for pool locator runPdxInstanceTest");

      CacheHelper.SetupJavaServers(true, "cacheserver_pdxinstance_hashcode.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool_PDX2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(pdxVersion1Put);
      m_client2.Call(pdxVersion2Put);
      m_client2.Call(getVersionObject);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      //Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void modifyPdxInstanceAndCheckLocally()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      IPdxInstance newpdxins;
      var pdxins = (IPdxInstance) region0["pdxput"];

      var oldVal = (int) pdxins.GetField("m_int32");

      var iwpi = pdxins.CreateWriter();

      iwpi.SetField("m_int32", oldVal + 1);
      iwpi.SetField("m_string", "change the string");
      region0["pdxput"] = iwpi;

      var lRegion = region0.GetLocalView();

      newpdxins = (IPdxInstance) lRegion["pdxput"];

      var newVal = (int) newpdxins.GetField("m_int32");

      Assert.AreEqual(oldVal + 1, newVal);

      var cStr = (string) newpdxins.GetField("m_string");
      Assert.AreEqual("change the string", cStr);

      var arr = (List<object>) newpdxins.GetField("m_arraylist");

      Assert.AreEqual(arr.Count, 2);

      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_char", 'D');
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((char) newpdxins.GetField("m_char"), 'D', "Char is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");


      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_bool", false);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((bool) newpdxins.GetField("m_bool"), false, "bool is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_byte", (sbyte) 0x75);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((sbyte) newpdxins.GetField("m_byte"), (sbyte) 0x75, "sbyte is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_sbyte", (sbyte) 0x57);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((sbyte) newpdxins.GetField("m_sbyte"), (sbyte) 0x57, "sbyte is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int16", (short) 0x5678);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((short) newpdxins.GetField("m_int16"), (short) 0x5678, "int16 is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_long", (long) 0x56787878);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((long) newpdxins.GetField("m_long"), (long) 0x56787878, "long is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_float", 18389.34f);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((float) newpdxins.GetField("m_float"), 18389.34f, "float is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_float", 18389.34f);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((float) newpdxins.GetField("m_float"), 18389.34f, "float is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_double", 18389.34d);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((double) newpdxins.GetField("m_double"), 18389.34d, "double is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_boolArray", new bool[] {true, false, true, false, true, true, false, true});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((bool[]) newpdxins.GetField("m_boolArray"),
        new bool[] {true, false, true, false, true, true, false, true}, "bool array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_byteArray", new byte[] {0x34, 0x64, 0x34, 0x64});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((byte[]) newpdxins.GetField("m_byteArray"), new byte[] {0x34, 0x64, 0x34, 0x64},
        "byte array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_charArray", new char[] {'c', 'v', 'c', 'v'});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((char[]) newpdxins.GetField("m_charArray"), new char[] {'c', 'v', 'c', 'v'},
        "char array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      var ticks = 634460644691580000L;
      var tdt = new DateTime(ticks);
      iwpi.SetField("m_dateTime", tdt);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((DateTime) newpdxins.GetField("m_dateTime"), tdt, "datetime is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int16Array", new short[] {0x2332, 0x4545, 0x88, 0x898});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((short[]) newpdxins.GetField("m_int16Array"), new short[] {0x2332, 0x4545, 0x88, 0x898},
        "short array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int32Array", new int[] {23, 676868, 34343});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((int[]) newpdxins.GetField("m_int32Array"), new int[] {23, 676868, 34343},
        "int32 array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_longArray", new long[] {3245435, 3425435});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((long[]) newpdxins.GetField("m_longArray"), new long[] {3245435, 3425435},
        "long array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_floatArray", new float[] {232.565f, 234323354.67f});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((float[]) newpdxins.GetField("m_floatArray"), new float[] {232.565f, 234323354.67f},
        "float array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_doubleArray", new double[] {23423432d, 43242354315d});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((double[]) newpdxins.GetField("m_doubleArray"), new double[] {23423432d, 43242354315d},
        "double array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var tmpbb = new byte[][]
      {
        new byte[] {0x23},
        new byte[] {0x34, 0x55},
        new byte[] {0x23},
        new byte[] {0x34, 0x55}
      };
      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_byteByteArray", tmpbb);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      var retbb = (byte[][]) newpdxins.GetField("m_byteByteArray");

      PdxType.compareByteByteArray(tmpbb, retbb);

      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_stringArray", new string[] {"one", "two", "eeeee"});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual((string[]) newpdxins.GetField("m_stringArray"), new string[] {"one", "two", "eeeee"},
        "string array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var tl = new List<object>();
      tl.Add(new PdxType());
      tl.Add(new byte[] {0x34, 0x55});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_arraylist", tl);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual(((List<object>) newpdxins.GetField("m_arraylist")).Count, tl.Count, "list<object> is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var map = new Dictionary<object, object>();
      map.Add(1, new bool[] {true, false, true, false, true, true, false, true});
      map.Add(2, new string[] {"one", "two", "eeeee"});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_map", map);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual(((Dictionary<object, object>) newpdxins.GetField("m_map")).Count, map.Count, "map is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var hashtable = new Hashtable();
      hashtable.Add(1, new string[] {"one", "two", "eeeee"});
      hashtable.Add(2, new int[] {23, 676868, 34343});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_hashtable", hashtable);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual(((Hashtable) newpdxins.GetField("m_hashtable")).Count, hashtable.Count, "hashtable is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var vector = new ArrayList();
      vector.Add(1);
      vector.Add(2);

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_vector", vector);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.AreEqual(((ArrayList) newpdxins.GetField("m_vector")).Count, vector.Count, "vector is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var chm = CacheableHashSet.Create();
      chm.Add(1);
      chm.Add("jkfdkjdsfl");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_chs", chm);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.True(chm.Equals(newpdxins.GetField("m_chs")), "CacheableHashSet is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var clhs = CacheableLinkedHashSet.Create();
      clhs.Add(111);
      clhs.Add(111343);

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_clhs", clhs);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) lRegion["pdxput"];
      Assert.True(clhs.Equals(newpdxins.GetField("m_clhs")), "CacheableLinkedHashSet is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");
    }

    private void runPdxInstanceLocalTest()
    {
      Util.Log("Starting iteration for pool locator runPdxInstanceTest");

      CacheHelper.SetupJavaServers(true, "cacheserver_pdxinstance_hashcode.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);

      m_client1.Call(CreateTCRegions_Pool_PDX2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(pdxPut);
      m_client2.Call(modifyPdxInstanceAndCheckLocally);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      //Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    public void pdxIFPutGetTest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = new PdxType();

      var pif = CacheHelper.DCache.CreatePdxInstanceFactory("PdxTests.PdxType");

      pif.WriteInt("m_int32", pt.Int32);
      pif.WriteString("m_string", pt.PString);
      pif.WriteObject("m_arraylist", pt.Arraylist);
      pif.WriteChar("m_char", pt.Char);
      pif.WriteBoolean("m_bool", pt.Bool);
      pif.WriteUnsignedByte("m_byte", pt.Byte);
      pif.WriteByte("m_sbyte", pt.Sbyte);
      pif.WriteShort("m_int16", pt.Int16);
      pif.WriteByteArray("m_byteArray", pt.ByteArray);
      pif.WriteLong("m_long", pt.Long);
      pif.WriteFloat("m_float", pt.Float);
      pif.WriteDouble("m_double", pt.Double);
      pif.WriteBooleanArray("m_boolArray", pt.BoolArray);
      pif.WriteSByteArray("m_sbyteArray", pt.SbyteArray);
      pif.WriteCharArray("m_charArray", pt.CharArray);
      pif.WriteDate("m_dateTime", pt.DateTime);
      pif.WriteShortArray("m_int16Array", pt.Int16Array);
      pif.WriteIntArray("m_int32Array", pt.Int32Array);
      pif.WriteLongArray("m_longArray", pt.LongArray);
      pif.WriteFloatArray("m_floatArray", pt.FloatArray);
      pif.WriteDoubleArray("m_doubleArray", pt.DoubleArray);
      pif.WriteArrayOfByteArrays("m_byteByteArray", pt.ByteByteArray);
      pif.WriteStringArray("m_stringArray", pt.StringArray);
      pif.WriteObject("m_map", pt.Map);
      pif.WriteObject("m_hashtable", pt.Hashtable);
      pif.WriteObject("m_vector", pt.Vector);
      pif.WriteObject("m_chs", pt.Chs);
      pif.WriteObject("m_clhs", pt.Clhs);
      pif.WriteUInt("m_uint32", pt.Uint32);
      pif.WriteULong("m_ulong", pt.Ulong);
      pif.WriteUShort("m_uint16", pt.Uint16);
      pif.WriteUIntArray("m_uint32Array", pt.Uint32Array);
      pif.WriteULongArray("m_ulongArray", pt.UlongArray);
      pif.WriteUShortArray("m_uint16Array", pt.Uint16Array);
      pif.WriteByteArray("m_byte252", pt.Byte252);
      pif.WriteByteArray("m_byte253", pt.Byte253);
      pif.WriteByteArray("m_byte65535", pt.Byte65535);
      pif.WriteByteArray("m_byte65536", pt.Byte65536);
      pif.WriteObject("m_pdxEnum", pt.PdxEnum);

      pif.WriteObject("m_address", pt.AddressArray);
      pif.WriteObjectArray("m_objectArray", pt.ObjectArray);

      var pi = pif.Create();

      Assert.AreEqual(pi.GetClassName(), "PdxTests.PdxType",
        "PdxInstanceFactory created PdxInstance. PdxInstance.GetClassName should return PdxTests.PdxType");

      var piObject = pi.GetObject();

      Assert.AreEqual(piObject, pt);

      region0["pi"] = pi;

      var ret = region0["pi"];

      Assert.AreEqual(ret, pt);

      var gotexcep = false;
      try
      {
        pif.Create();
      }
      catch (IllegalStateException)
      {
        gotexcep = true;
      }

      Assert.IsTrue(gotexcep, "Pdx instance factory should have thrown IllegalStateException");

      var pp = new ParentPdx(2);
      var if2 = CacheHelper.DCache.CreatePdxInstanceFactory(pp.GetType().FullName);
      if2.WriteInt("_parentId", pp._parentId);
      if2.WriteObject("_gender", pp._gender);
      if2.WriteString("_parentName", pp._parentName);
      if2.WriteObject("_childPdx", pp._childPdx);

      var ip2 = if2.Create();
      region0["pp"] = ip2;

      ret = region0["pp"];

      Assert.AreEqual(ret, pp, "parent pdx should be same");
    }

    //this test use write field Api
    public void pdxIFPutGetTest2()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = new PdxType();

      var pif = CacheHelper.DCache.CreatePdxInstanceFactory("PdxTests.PdxType");

      pif.WriteField("m_int32", pt.Int32, pt.Int32.GetType());
      pif.WriteField("m_string", pt.PString, pt.PString.GetType());
      // pif.WriteField("m_arraylist", pt.Arraylist, pt.Arraylist.GetType());
      //we treat arraylist as ObjectArray as well, so here need to differentiate it
      pif.WriteObject("m_arraylist", pt.Arraylist);
      pif.WriteField("m_char", pt.Char, pt.Char.GetType());
      pif.WriteField("m_bool", pt.Bool, pt.Bool.GetType());
      pif.WriteField("m_sbyte", pt.Sbyte, pt.Sbyte.GetType());
      pif.WriteField("m_byte", pt.Byte, pt.Byte.GetType());
      pif.WriteField("m_int16", pt.Int16, pt.Int16.GetType());
      pif.WriteField("m_byteArray", pt.ByteArray, pt.ByteArray.GetType());
      pif.WriteField("m_long", pt.Long, pt.Long.GetType());
      pif.WriteField("m_float", pt.Float, pt.Float.GetType());
      pif.WriteField("m_double", pt.Double, pt.Double.GetType());
      pif.WriteField("m_boolArray", pt.BoolArray, pt.BoolArray.GetType());
      pif.WriteField("m_sbyteArray", pt.SbyteArray, pt.SbyteArray.GetType());
      pif.WriteField("m_charArray", pt.CharArray, pt.CharArray.GetType());
      pif.WriteField("m_dateTime", pt.DateTime, pt.DateTime.GetType());
      pif.WriteField("m_int16Array", pt.Int16Array, pt.Int16Array.GetType());
      pif.WriteField("m_int32Array", pt.Int32Array, pt.Int32Array.GetType());
      pif.WriteField("m_longArray", pt.LongArray, pt.LongArray.GetType());
      pif.WriteField("m_floatArray", pt.FloatArray, pt.FloatArray.GetType());
      pif.WriteField("m_doubleArray", pt.DoubleArray, pt.DoubleArray.GetType());
      pif.WriteField("m_byteByteArray", pt.ByteByteArray, pt.ByteByteArray.GetType());
      pif.WriteField("m_stringArray", pt.StringArray, pt.StringArray.GetType());
      pif.WriteField("m_map", pt.Map, pt.Map.GetType());
      pif.WriteField("m_hashtable", pt.Hashtable, pt.Hashtable.GetType());
      pif.WriteField("m_vector", pt.Vector, pt.Vector.GetType());
      pif.WriteField("m_chs", pt.Chs, pt.Chs.GetType());
      pif.WriteField("m_clhs", pt.Clhs, pt.Clhs.GetType());
      pif.WriteField("m_uint32", pt.Uint32, pt.Uint32.GetType());
      pif.WriteField("m_ulong", pt.Ulong, pt.Ulong.GetType());
      pif.WriteField("m_uint16", pt.Uint16, pt.Uint16.GetType());
      pif.WriteField("m_uint32Array", pt.Uint32Array, pt.Uint32Array.GetType());
      pif.WriteField("m_ulongArray", pt.UlongArray, pt.UlongArray.GetType());
      pif.WriteField("m_uint16Array", pt.Uint16Array, pt.Uint16Array.GetType());
      pif.WriteField("m_byte252", pt.Byte252, pt.Byte252.GetType());
      pif.WriteField("m_byte253", pt.Byte253, pt.Byte253.GetType());
      pif.WriteField("m_byte65535", pt.Byte65535, pt.Byte65535.GetType());
      pif.WriteField("m_byte65536", pt.Byte65536, pt.Byte65536.GetType());
      pif.WriteField("m_pdxEnum", pt.PdxEnum, pt.PdxEnum.GetType());

      var aa = new PdxTests.Address[10];

      for (var i = 0; i < 10; i++)
      {
        aa[i] = new PdxTests.Address(i + 1, "street" + i.ToString(), "city" + i.ToString());
      }

      pif.WriteField("m_address", pt.AddressArray, aa.GetType());
      pif.WriteField("m_objectArray", pt.ObjectArray, pt.ObjectArray.GetType());

      var pi = pif.Create();

      var piObject = pi.GetObject();

      Assert.AreEqual(piObject, pt);

      region0["pi2"] = pi;

      var ret = region0["pi2"];

      Assert.AreEqual(ret, pt);

      var pp = new ParentPdx(2);
      var if2 = CacheHelper.DCache.CreatePdxInstanceFactory(pp.GetType().FullName);
      if2.WriteField("_parentId", pp._parentId, pp._parentId.GetType());
      if2.WriteField("_gender", pp._gender, pp._gender.GetType());
      if2.WriteField("_parentName", pp._parentName, pp._parentName.GetType());
      if2.WriteField("_childPdx", pp._childPdx, pp._childPdx.GetType());

      var ip2 = if2.Create();
      region0["ppwf"] = ip2;

      ret = region0["ppwf"];

      Assert.AreEqual(ret, pp, "parent pdx should be same");
    }

    public void runPdxInstanceFactoryTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(pdxIFPutGetTest);
      //m_client2.Call();
      Util.Log("StepThree complete.");
      m_client1.Call(pdxIFPutGetTest2);
      Util.Log("StepFour complete.");

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      //Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    public void pdxTypeMapperTest1()
    {
      Console.WriteLine("pdxTypeMapperTest 1");
      CacheHelper.DCache.TypeRegistry.PdxTypeMapper = new PdxTypeMapper();
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      if (region0 == null)
        Console.WriteLine("pdxTypeMapperTest region is null");
      else
        Console.WriteLine("pdxTypeMapperTest region is NOT null");

      var pt = new PdxType();

      for (var i = 0; i < 10; i++)
      {
        region0[i] = pt;
      }

      for (var i = 0; i < 10; i++)
      {
        var ret = region0[i];

        Assert.AreEqual(ret, pt);
      }
    }

    public void pdxTypeMapperTest2()
    {
      CacheHelper.DCache.TypeRegistry.PdxTypeMapper = new PdxTypeMapper();
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = new PdxType();

      for (var i = 0; i < 10; i++)
      {
        var ret = region0[1];

        Assert.AreEqual(ret, pt);
      }
    }

    public void pdxITypeMapperTest()
    {
      CacheHelper.DCache.TypeRegistry.PdxTypeMapper = new PdxTypeMapper();
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = new PdxType();

      for (var i = 0; i < 10; i++)
      {
        var ret = region0[1];

        var pi = ret as IPdxInstance;
        Assert.IsNotNull(pi);

        using (pi)
        {
          Assert.AreEqual(pi.GetObject(), pt);
        }
      }
    }

    public void pdxASTypeMapperTest()
    {
      CacheHelper.DCache.TypeRegistry.PdxSerializer = new AutoSerializerEx();
      CacheHelper.DCache.TypeRegistry.PdxTypeMapper = new PdxTypeMapper();
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var sp3 = new SerializePdx3(true, 2);

      for (var i = 100; i < 110; i++)
      {
        region0[i] = sp3;
      }

      for (var i = 100; i < 110; i++)
      {
        var ret = region0[i];
        Assert.AreEqual(sp3, ret);
      }
    }

    public void runPdxTypeMapperTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client4.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);

      m_client3.Call(CreateTCRegions_Pool_PDX2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);


      Util.Log("StepTwo (pool locators) complete.");
      Console.WriteLine("client created");

      m_client1.Call(pdxTypeMapperTest1);
      Console.WriteLine("client created2");
      m_client2.Call(pdxTypeMapperTest2);
      m_client3.Call(pdxITypeMapperTest);
      m_client4.Call(pdxASTypeMapperTest);


      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      m_client3.Call(Close);
      m_client4.Call(Close);
      //Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private int nPdxPuts = 100000;
    private int pdxobjsize = 5000;

    private void checkLocalCache()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var localregion = region0.GetLocalView();

      var entryNotFound = 0;
      var entryFound = 0;
      for (var i = 0; i < nPdxPuts; i++)
      {
        try
        {
          var ret = localregion[i];
          if (ret != null)
          {
            var ht = ret as Heaptest;
            if (ht != null)
              entryFound++;
          }
        }
        catch (Client.KeyNotFoundException)
        {
          entryNotFound++;
        }
      }

      Assert.Greater(entryFound, 100, "enteries should be in local cache");
      Assert.Greater(nPdxPuts, entryFound + 50000, "pdx object should have evicted");
    }

    private void putPdxheaptest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < nPdxPuts; i++)
      {
        region0[i] = new Heaptest(pdxobjsize);
      }

      checkLocalCache();
    }

    private void getPdxHeaptest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      for (var i = 0; i < nPdxPuts; i++)
      {
        var ret = region0[i];
      }

      checkLocalCache();
    }

    private void runPdxTypeObjectSizeTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(SetHeaplimit, 100, 10);
      m_client2.Call(SetHeaplimit, 100, 10);

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(putPdxheaptest);
      m_client2.Call(getPdxHeaptest);

      m_client1.Call(UnsetHeapLimit);
      m_client2.Call(UnsetHeapLimit);
      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private readonly int nBAPuts = 25;
    private readonly int baSize = 16240000;

    private void checkLocalCacheBA(bool checkmem)
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var localregion = region0.GetLocalView();

      var entryNotFound = 0;
      var entryFound = 0;
      for (var i = 0; i < nBAPuts; i++)
      {
        try
        {
          var ret = localregion[i];
          if (ret != null)
          {
            var ht = ret as byte[];
            if (ht != null)
              entryFound++;
          }
        }
        catch (Client.KeyNotFoundException)
        {
          entryNotFound++;
        }
      }

      Assert.Greater(entryFound, 8, "enteries should be in local cache");
      Assert.Greater(nBAPuts, entryFound + 10, "pdx object should have evicted");

      var mem = (int) GC.GetTotalMemory(true);
      // if(checkmem)
      //Assert.Less(mem, 200000000, "Memory should be less then 200 mb");
    }

    private void putBAheaptest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < nBAPuts; i++)
      {
        region0[i] = new byte[baSize];
      }

      checkLocalCacheBA(false);
    }

    private void getBAHeaptest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      for (var i = 0; i < nBAPuts; i++)
      {
        var ret = region0[i];
      }

      checkLocalCacheBA(true);
    }

    private void runByteArrayObjectSizeTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(SetHeaplimit, 150, 5);
      m_client2.Call(SetHeaplimit, 150, 5);

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(putBAheaptest);
      m_client2.Call(getBAHeaptest);

      m_client1.Call(UnsetHeapLimit);
      m_client2.Call(UnsetHeapLimit);
      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void putPdxWithEnum()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0[0] = new PdxEnumTestClass(0);
      region0[1] = new PdxEnumTestClass(1);
      region0[2] = new PdxEnumTestClass(2);
    }

    private void pdxEnumQuery()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var sr = region0.Query<object>("_enumid.name = 'id2'");

      Assert.AreEqual(1, sr.Size, "query result should have one item");

      var en = sr.GetEnumerator();

      while (en.MoveNext())
      {
        var re = (PdxEnumTestClass) en.Current;
        Assert.AreEqual(1, re.ID, "query should have return id 1");
      }
    }

    private void runPdxEnumQueryTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(putPdxWithEnum);
      m_client1.Call(pdxEnumQuery);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void registerPdxDelta()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0.GetSubscriptionService().RegisterAllKeys();
    }

    private void putPdxDelta()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var pd = new javaobject.PdxDelta(1001);
      for (var i = 0; i < 10; i++)
      {
        region0["pdxdelta"] = pd;
      }

      Assert.Greater(javaobject.PdxDelta.GotDelta, 7, "this should have more todelta");
    }

    private void verifyPdxDelta()
    {
      System.Threading.Thread.Sleep(5000);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      //Assert.Greater(javaobject.PdxDelta.GotDelta, 7, "this should have recieve delta");
      var pd = (javaobject.PdxDelta) region0.GetLocalView()["pdxdelta"];
      Assert.Greater(pd.Delta, 7, "this should have recieve delta");
      Assert.Greater(javaobject.PdxDelta.GotDelta, 7, "this should have more todelta");
    }

    private void runPdxDeltaTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverForPdx.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");
      m_client2.Call(registerPdxDelta);
      m_client1.Call(putPdxDelta);
      m_client2.Call(verifyPdxDelta);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void generateJavaPdxType()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var args = "saveAllJavaPdxTypes";
      var filter = new List<object>();
      filter.Add(1);
      var execution = FunctionService<object>.OnRegion<object, object>(region0)
        .WithArgs<object>(args).WithFilter<object>(filter);


      var resultCollector = execution.Execute("ComparePdxTypes");

      var executeFunctionResult = resultCollector.GetResult();

      var gotResult = false;
      foreach (var item in executeFunctionResult)
      {
        Assert.AreEqual(item, true, "Function should return true");
        gotResult = true;
      }

      Assert.AreEqual(gotResult, true, "Function should return true");
    }

    private void putAllPdxTypes()
    {
      var r = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var p1 = new PdxTypes1();
      r[p1.GetType().FullName] = p1;

      var p2 = new PdxTypes2();
      r[p2.GetType().FullName] = p2;

      var p3 = new PdxTypes3();
      r[p3.GetType().FullName] = p3;

      var p4 = new PdxTypes4();
      r[p4.GetType().FullName] = p4;

      var p5 = new PdxTypes5();
      r[p5.GetType().FullName] = p5;

      var p6 = new PdxTypes6();
      r[p6.GetType().FullName] = p6;

      var p7 = new PdxTypes7();
      r[p7.GetType().FullName] = p7;

      var p8 = new PdxTypes8();
      r[p8.GetType().FullName] = p8;

      var p9 = new PdxTypes9();
      r[p9.GetType().FullName] = p9;

      var p10 = new PdxTypes10();
      r[p10.GetType().FullName] = p10;
    }

    private void verifyDotNetPdxTypes()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var args = "compareDotNETPdxTypes";
      var filter = new List<object>();
      filter.Add(1);
      var execution = FunctionService<object>.OnRegion<object, object>(region0)
        .WithArgs<object>(args).WithFilter<object>(filter);


      var resultCollector = execution.Execute("ComparePdxTypes");

      var executeFunctionResult = resultCollector.GetResult();

      var gotResult = false;
      foreach (var item in executeFunctionResult)
      {
        Assert.AreEqual(item, true, "Function should return true");
        gotResult = true;
      }

      Assert.AreEqual(gotResult, true, "Function should return true");
    }

    private void runPdxMetadataCheckTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdx2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(generateJavaPdxType);
      m_client1.Call(putAllPdxTypes);
      m_client1.Call(verifyDotNetPdxTypes);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void client1PutsV1Object()
    {
      initializePdxAssemblyOne3(false);


      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");
      var np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      var pRet = region0[1];
    }

    //this has v2 object
    private void client2GetsV1ObjectAndPutsV2Object()
    {
      initializePdxAssemblyTwo3(false);

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");
      var np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);

      //get v1 ojbject ..
      var pRet = (object) region0[1];

      //now put v2 object
      region0[2] = np;
    }

    //this should fails..
    private void client3GetsV2Object()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var execution = FunctionService<object>.OnRegion<object, object>(region0);

      var resultCollector = execution.Execute("IterateRegion");

      var executeFunctionResult = resultCollector.GetResult();

      var gotResult = false;
      foreach (var item in executeFunctionResult)
      {
        Assert.AreEqual(item, true, "Function should return true");
        gotResult = true;
      }

      Assert.AreEqual(gotResult, true, "Function should return true");
    }

    private void runPdxBankTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdx2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true, false, true /*local caching false*/);

      m_client3.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true, false, true /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");
      m_client1.Call(client1PutsV1Object);
      m_client2.Call(client2GetsV1ObjectAndPutsV2Object);
      m_client3.Call(client3GetsV2Object);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");
      m_client3.Call(Close);
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void putFromLongRunningClient()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      region0[1] = new PdxTypes1();
      region0[2] = new PdxTypes2();
    }

    private void put2FromLongRunningClient()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      region0[2] = new PdxTypes2();
    }

    private void VerifyEntry2FLRC()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      try
      {
        var ret = region0[2];
        Assert.Fail("Expected exception.");
      }
      catch (Exception)
      {
        // Expected
      }
    }

    private readonly string testSysPropFileName = "testLR.properties";

    private void createExtraSysPropFile(string name, string value)
    {
      // create a file for alternate properties...
      var sw = new StreamWriter(testSysPropFileName);
      sw.WriteLine(name + "=" + value);
      sw.Close();
    }

    private void runPdxLongrunningClientTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdx.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      createExtraSysPropFile("on-client-disconnect-clear-pdxType-Ids", "true");
      m_client1.Call(CacheHelper.SetExtraPropertiesFile, testSysPropFileName);
      m_client2.Call(CacheHelper.SetExtraPropertiesFile, testSysPropFileName);

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(putFromLongRunningClient);
      Util.Log("StepThree complete.");

      //m_client2.Call(VerifyEntryFLRC);
      Util.Log("StepFour complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started again.");

      //different object
      m_client1.Call(put2FromLongRunningClient);
      Util.Log("StepFive complete.");

      //this should throw exception
      m_client2.Call(VerifyEntry2FLRC);
      Util.Log("StepSeven complete.");

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();

      m_client1.Call(CacheHelper.SetExtraPropertiesFile, (string) null);
      m_client2.Call(CacheHelper.SetExtraPropertiesFile, (string) null);
    }

    #region Tests

    [Test]
    public void PdxVersionClassesEqualTest()
    {
      runPdxVersionClassesEqualTest();
    }

    [Test]
    public void PdxInstanceLocalTest()
    {
      runPdxInstanceLocalTest();
    }

    [Test]
    public void PdxInstanceFactoryTest()
    {
      runPdxInstanceFactoryTest();
    }

    [Test]
    public void PdxTypeMapperTest()
    {
      runPdxTypeMapperTest();
    }

    //[Test]
    public void PdxTypeObjectSizeTest()
    {
      runPdxTypeObjectSizeTest();
    }

    [Test]
    public void ByteArrayObjectSizeTest()
    {
      runByteArrayObjectSizeTest();
    }

    [Test]
    public void PdxEnumQueryTest()
    {
      runPdxEnumQueryTest();
    }

    [Test]
    public void PdxDeltaTest()
    {
      runPdxDeltaTest();
    }

    [Test]
    public void PdxMetadataCheckTest()
    {
      runPdxMetadataCheckTest();
    }

    [Test]
    public void PdxBankTest()
    {
      runPdxBankTest();
    }

    [Test]
    public void PdxLongrunningClientTest()
    {
      runPdxLongrunningClientTest();
    }

    #endregion
  }
}