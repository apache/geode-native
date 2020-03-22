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
using PdxTests;
using System.Reflection;
using System.Linq;

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
  internal class ThinClientPdxTests2 : ThinClientRegionSteps
  {
    private static bool m_useWeakHashMap = false;

    #region Private members

    private UnitProcess m_client1, m_client2;

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      return new ClientBase[] {m_client1, m_client2};
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

    private IPdxSerializable registerPdxUIV1()
    {
      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxUIAssemblyOne(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxUIV1);

      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");

      var ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private void putV1PdxUI(bool useWeakHashmap)
    {
      initializePdxUIAssemblyOne(useWeakHashmap);

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");
      var np = pt.InvokeMember("PdxTypesIgnoreUnreadFields", BindingFlags.CreateInstance, null, null, null);
      var pRet = region0[1];
      region0[1] = pRet;


      Console.WriteLine(np);
      Console.WriteLine(pRet);

      //Assert.AreEqual(np, pRet);
    }

    private IPdxSerializable registerPdxUIV2()
    {
      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");

      var ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxUIAssemblyTwo(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxUIV2);

      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");

      var ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private void putV2PdxUI(bool useWeakHashmap)
    {
      initializePdxUIAssemblyTwo(useWeakHashmap);

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");
      var np = pt.InvokeMember("PdxTypesIgnoreUnreadFields", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      var pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      Assert.AreEqual(np, pRet);
      region0[1] = pRet;
      Console.WriteLine(" " + pRet);
    }

    private void getV2PdxUI()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesIgnoreUnreadFields");
      var np = pt.InvokeMember("PdxTypesIgnoreUnreadFields", BindingFlags.CreateInstance, null, null, null);

      var pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      Assert.AreEqual(np, pRet);
    }

    private void runPdxIgnoreUnreadFieldTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client2.Call(putV2PdxUI, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client1.Call(putV1PdxUI, m_useWeakHashMap);
      Util.Log("StepFour complete.");

      m_client2.Call(getV2PdxUI);
      Util.Log("StepFive complete.");

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

    private void putFromPool1()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      Util.Log("Put from pool-1 started");
      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[0]);

      region0[1] = new PdxTypes1();

      region0[2] = new PdxType();
      Util.Log("Put from pool-1 Completed");
    }

    private void putFromPool2()
    {
      Util.Log("Put from pool-21 started");
      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[1]);

      region0[1] = new PdxTypes1();
      region0[2] = new PdxType();
      var ret = region0[1];
      ret = region0[2];
      Util.Log("Put from pool-2 completed");

      var pdxIds = CacheHelper.DCache.GetPdxTypeRegistry().testGetNumberOfPdxIds();

      Assert.AreEqual(3, pdxIds);
    }

    private void runMultipleDSTest()
    {
      Util.Log("Starting runMultipleDSTest. ");

      CacheHelper.SetupJavaServers(true, "cacheserverMDS1.xml", "cacheserverMDS2.xml");
      CacheHelper.StartJavaLocator_MDS(1, "GFELOC", null, 1 /*ds id is one*/);
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaLocator_MDS(2, "GFELOC2", null, 2 /*ds id is one*/);
      Util.Log("Locator 2 started.");

      CacheHelper.StartJavaServerWithLocator_MDS(1, "GFECS1", 1);
      Util.Log("Server 1 started with locator 1.");

      CacheHelper.StartJavaServerWithLocator_MDS(2, "GFECS2", 2);
      Util.Log("Server 2 started with locator 2.");

      //client intialization 
      /*
       *  CreateTCRegion_Pool(string name, bool ack, bool caching,
     ICacheListener listener, string endpoints, string locators, string poolName, bool clientNotification, bool ssl,
     bool cloningEnabled)
       * 
       */

      m_client1.Call(CacheHelper.CreateTCRegion_Pool_MDS,
        RegionNames[0], true, false,
        CacheHelper.LocatorFirst, "__TESTPOOL1_",
        false, false, false);

      Util.Log("StepOne (pool-1 locators) complete. " + CacheHelper.LocatorFirst);

      m_client1.Call(CacheHelper.CreateTCRegion_Pool_MDS,
        RegionNames[1], false, false,
        CacheHelper.LocatorSecond, "__TESTPOOL2_",
        false, false, false);

      Util.Log("StepTwo (pool-2 locators) complete. " + CacheHelper.LocatorSecond);


      m_client1.Call(putFromPool1);

      m_client1.Call(putFromPool2);

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      //m_client2.Call(Close);
      //Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");
      CacheHelper.StopJavaLocator(2);
      Util.Log("Locator 2 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void initializePdxSerializer()
    {
      CacheHelper.DCache.TypeRegistry.PdxSerializer = new PdxSerializer();

      //Serializable.RegisterTypeForPdxSerializer(SerializePdx1.CreateDeserializable);
    }

    private void doPutGetWithPdxSerializer()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        object put = new SerializePdx1(true);
        ;

        region0[i] = put;

        var ret = region0[i];

        Assert.AreEqual(put, ret);

        put = new SerializePdx2(true);
        region0[i + 10] = put;


        ret = region0[i + 10];

        Assert.AreEqual(put, ret);


        put = new SerializePdx3(true, i % 2);
        region0[i + 20] = put;

        ret = region0[i + 20];

        Assert.AreEqual(put, ret);
      }
    }

    private void doGetWithPdxSerializerC2()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var sp3Even = new SerializePdx3(true, 0);
      var sp3Odd = new SerializePdx3(true, 1);

      for (var i = 0; i < 10; i++)
      {
        object local = new SerializePdx1(true);
        ;

        var ret = region0[i];

        Assert.AreEqual(local, ret);

        ret = region0[i + 10];
        Assert.AreEqual(new SerializePdx2(true), ret);

        ret = region0[i + 20];
        if (i % 2 == 0)
        {
          Assert.AreEqual(ret, sp3Even);
          Assert.AreNotEqual(ret, sp3Odd);
        }
        else
        {
          Assert.AreEqual(ret, sp3Odd);
          Assert.AreNotEqual(ret, sp3Even);
        }
      }
    }

    private void doQueryTest()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        var result = region0.Query<object>("i1 = " + i);
        Util.Log(" query result size " + result.Size);
      }

      var result2 = region0.Query<object>("1 = 1");
      Util.Log(" query result size " + result2.Size);

      //private Address[] _addressArray;
      //private int arrayCount = 10;
      //private ArrayList _addressList;
      //private Address _address;
      //private Hashtable _hashTable;

      //var qs = PoolManager/*<object, object>*/.Find("__TESTPOOL1_").GetQueryService();

      //Query<object> qry = qs.NewQuery<object>("select _addressArray from /" + m_regionNames[0] + " where arrayCount = 10");
      //ISelectResults<object> results = qry.Execute();
      //Assert.Greater(results.Size, 5, "query should have result");
      //IEnumerator<object> ie = results.GetEnumerator();
      //Address[] ad;
      //while (ie.MoveNext())
      //{
      //  Address[] ar = (Address[])ie.Current;
      //  Assert.AreEqual(ar.Length, 10, "Array size should be 10");
      //}
    }

    private void runPdxSerializerTest()
    {
      Util.Log("Starting iteration for pool locator runPdxSerializerTest");

      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(initializePdxSerializer);
      m_client2.Call(initializePdxSerializer);
      Util.Log("StepThree complete.");

      m_client1.Call(doPutGetWithPdxSerializer);
      Util.Log("StepFour complete.");

      m_client2.Call(doGetWithPdxSerializerC2);
      Util.Log("StepFive complete.");

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

    private void initializeReflectionPdxSerializer()
    {
      CacheHelper.DCache.TypeRegistry.PdxSerializer = new AutoSerializerEx();

      //Serializable.RegisterTypeForPdxSerializer(SerializePdx1.CreateDeserializable);
    }

    private void doPutGetWithPdxSerializerR()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        object put = new SerializePdx1(true);
        ;

        region0[i] = put;

        var ret = region0[i];

        Assert.AreEqual(put, ret);

        put = new SerializePdx2(true);
        region0[i + 10] = put;


        ret = region0[i + 10];

        Assert.AreEqual(put, ret);

        put = new PdxTypesReflectionTest(true);
        region0[i + 20] = put;


        ret = region0[i + 20];

        Assert.AreEqual(put, ret);

        put = new SerializePdx3(true, i % 2);
        region0[i + 30] = put;


        ret = region0[i + 30];

        Assert.AreEqual(put, ret);

        put = new SerializePdx4(true);
        region0[i + 40] = put;


        ret = region0[i + 40];

        Assert.AreEqual(put, ret);

        var p1 = region0[i + 30];
        var p2 = region0[i + 40];

        Assert.AreNotEqual(p1, p2, "This should NOt be equals");

        var pft = new PdxFieldTest(true);
        region0[i + 50] = pft;
        ret = region0[i + 50];

        Assert.AreNotEqual(pft, ret);

        pft.NotInclude = "default_value";
        Assert.AreEqual(pft, ret);
      }

      IDictionary<object, object> putall = new Dictionary<object, object>();
      putall.Add(100, new SerializePdx3(true, 0));
      putall.Add(200, new SerializePdx3(true, 1));
      putall.Add(300, new SerializePdx4(true));
      region0.PutAll(putall);
    }

    private void doGetWithPdxSerializerC2R()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        object local = new SerializePdx1(true);
        ;

        var ret = region0[i];

        Assert.AreEqual(local, ret);

        ret = region0[i + 10];
        Assert.AreEqual(new SerializePdx2(true), ret);

        ret = region0[i + 20];
        Assert.AreEqual(new PdxTypesReflectionTest(true), ret);

        var sp3Odd = new SerializePdx3(true, 1);
        var sp3Even = new SerializePdx3(true, 0);

        ret = region0[i + 30];

        if (i % 2 == 0)
        {
          Assert.AreEqual(sp3Even, ret);
          Assert.AreNotEqual(sp3Odd, ret);
        }
        else
        {
          Assert.AreEqual(sp3Odd, ret);
          Assert.AreNotEqual(sp3Even, ret);
        }

        ret = region0[i + 40];
        var sp4 = new SerializePdx4(true);
        Assert.AreEqual(sp4, ret);
        Console.WriteLine(sp4 + "===" + ret);

        var p1 = region0[i + 30];
        var p2 = region0[i + 40];

        Assert.AreNotEqual(p1, p2, "This should NOt be equal");
      }

      IDictionary<object, object> getall = new Dictionary<object, object>();
      ICollection<object> keys = new List<object>();
      keys.Add(100);
      keys.Add(200);
      keys.Add(300);
      //putall.Add(100, new SerializePdx3(true, 0));
      //putall.Add(200, new SerializePdx3(true, 1));
      //putall.Add(300, new SerializePdx4(true));
      region0.GetAll(keys, getall, null);

      Assert.AreEqual(getall[100], new SerializePdx3(true, 0));
      Assert.AreEqual(getall[200], new SerializePdx3(true, 1));
      Assert.AreEqual(getall[300], new SerializePdx4(true));
    }

    private void runReflectionPdxSerializerTest()
    {
      Util.Log("Starting iteration for pool locator runPdxSerializerTest");

      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(initializeReflectionPdxSerializer);
      m_client2.Call(initializeReflectionPdxSerializer);
      Util.Log("StepThree complete.");

      m_client1.Call(doPutGetWithPdxSerializerR);
      Util.Log("StepFour complete.");

      m_client2.Call(doGetWithPdxSerializerC2R);
      Util.Log("StepFive complete.");

      m_client2.Call(doQueryTest);
      Util.Log("StepSix complete.");

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

    private void dinitPdxSerializer()
    {
      CacheHelper.DCache.TypeRegistry.PdxSerializer = null;
    }

    private void doPutGetWithPdxSerializerNoReg()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        object put = new SerializePdxNoRegister(true);
        ;

        region0[i] = put;

        var ret = region0[i];

        Assert.AreEqual(put, ret);
      }
    }

    private void doGetWithPdxSerializerC2NoReg()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      for (var i = 0; i < 10; i++)
      {
        object local = new SerializePdxNoRegister(true);
        ;

        var ret = region0[i];

        Assert.AreEqual(local, ret);
      }
    }

    private void runPdxTestWithNoTypeRegister()
    {
      Util.Log("Starting iteration for pool locator runPdxTestWithNoTypeRegister");

      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool_PDX, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      Util.Log("StepThree complete.");

      m_client1.Call(dinitPdxSerializer);
      m_client2.Call(dinitPdxSerializer);
      m_client1.Call(doPutGetWithPdxSerializerNoReg);
      Util.Log("StepFour complete.");

      m_client2.Call(doGetWithPdxSerializerC2NoReg);
      Util.Log("StepFive complete.");

      m_client2.Call(doQueryTest);
      Util.Log("StepSix complete.");

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

    private void pdxPut()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      region0["pdxput"] = new PdxType();
      region0["pdxput2"] = new ParentPdx(1);
    }

    private void getObject()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var ret = (IPdxInstance) region0["pdxput"];

      Assert.AreEqual(ret.GetClassName(), "PdxTests.PdxType",
        "PdxInstance.GetClassName should return PdxTests.PdxType");

      var pt = (PdxType) ret.GetObject();

      var ptorig = new PdxType();


      Assert.AreEqual(pt, ptorig, "PdxInstance.getObject not equals original object.");

      ret = (IPdxInstance) region0["pdxput2"];
      var pp = (ParentPdx) ret.GetObject();

      var ppOrig = new ParentPdx(1);

      Assert.AreEqual(pp, ppOrig, "Parent pdx should be equal ");
    }

    private void verifyPdxInstanceEquals()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var ret = (IPdxInstance) region0["pdxput"];
      var ret2 = (IPdxInstance) region0["pdxput"];


      Assert.AreEqual(ret, ret2, "PdxInstance equals are not matched.");

      Util.Log(ret.ToString());
      Util.Log(ret2.ToString());

      ret = (IPdxInstance) region0["pdxput2"];
      ret2 = (IPdxInstance) region0["pdxput2"];

      Assert.AreEqual(ret, ret2, "parent pdx equals are not matched.");
    }

    private void verifyPdxInstanceHashcode()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var ret = (IPdxInstance) region0["pdxput"];
      var dPdxType = new PdxType();

      var pdxInstHashcode = ret.GetHashCode();
      Util.Log("pdxinstance hash code " + pdxInstHashcode);

      var javaPdxHC = (int) region0["javaPdxHC"];

      //TODO: need to fix this is beacause Enum hashcode is different in java and .net
      //Assert.AreEqual(javaPdxHC, pdxInstHashcode, "Pdxhashcode hashcode not matched with java padx hash code.");

      //for parent pdx
      ret = (IPdxInstance) region0["pdxput2"];
      pdxInstHashcode = ret.GetHashCode();
      Assert.AreEqual(javaPdxHC, pdxInstHashcode,
        "Pdxhashcode hashcode not matched with java padx hash code for Parentpdx class.");
    }

    private void accessPdxInstance()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var ret = (IPdxInstance) region0["pdxput"];

      var dPdxType = new PdxType();

      var retStr = (string) ret.GetField("m_string");

      Assert.AreEqual(dPdxType.PString, retStr);

      PdxType.GenericValCompare((char) ret.GetField("m_char"), dPdxType.Char);

      var baa = (byte[][])ret.GetField("m_byteByteArray");
      PdxType.compareByteByteArray(baa, dPdxType.ByteByteArray);

      var sbaa = (sbyte[][])ret.GetField("m_sbyteByteArray");
      PdxType.compareSByteSByteArray(sbaa, dPdxType.SByteByteArray);

      PdxType.GenericCompare((char[]) ret.GetField("m_charArray"), dPdxType.CharArray);

      var bl = (bool) ret.GetField("m_bool");
      PdxType.GenericValCompare(bl, dPdxType.Bool);
      PdxType.GenericCompare((bool[]) ret.GetField("m_boolArray"), dPdxType.BoolArray);

      PdxType.GenericValCompare((byte)(sbyte)(ret.GetField("m_byte")), dPdxType.Byte);
      PdxType.GenericValCompare(ret.GetField("m_byteArray").ToString(), dPdxType.ByteArray.ToString());

      var tmpl = (List<object>) ret.GetField("m_arraylist");

      PdxType.compareCompareCollection(tmpl, dPdxType.Arraylist);

      var tmpM = (IDictionary<object, object>) ret.GetField("m_map");
      if (tmpM.Count != dPdxType.Map.Count)
        throw new IllegalStateException("Not got expected value for type: " + dPdxType.Map.GetType().ToString());

      var tmpH = (Hashtable) ret.GetField("m_hashtable");

      if (tmpH.Count != dPdxType.Hashtable.Count)
        throw new IllegalStateException("Not got expected value for type: " + dPdxType.Hashtable.GetType().ToString());

      var arrAl = (ArrayList) ret.GetField("m_vector");

      if (arrAl.Count != dPdxType.Vector.Count)
        throw new IllegalStateException("Not got expected value for type: " + dPdxType.Vector.GetType().ToString());

      var rmpChs = (CacheableHashSet) ret.GetField("m_chs");

      if (rmpChs.Count != dPdxType.Chs.Count)
        throw new IllegalStateException("Not got expected value for type: " + dPdxType.Chs.GetType().ToString());

      var rmpClhs = (CacheableLinkedHashSet) ret.GetField("m_clhs");

      if (rmpClhs.Count != dPdxType.Clhs.Count)
        throw new IllegalStateException("Not got expected value for type: " + dPdxType.Clhs.GetType().ToString());


      PdxType.GenericValCompare((string) ret.GetField("m_string"), dPdxType.String);

      PdxType.compareData((DateTime) ret.GetField("m_dateTime"), dPdxType.DateTime);

      PdxType.GenericValCompare((double) ret.GetField("m_double"), dPdxType.Double);

      PdxType.GenericCompare((long[]) ret.GetField("m_longArray"), dPdxType.LongArray);
      PdxType.GenericCompare((short[]) ret.GetField("m_int16Array"), dPdxType.Int16Array);
      PdxType.GenericValCompare((sbyte) ret.GetField("m_sbyte"), dPdxType.Sbyte);
      PdxType.GenericCompare((sbyte[]) ret.GetField("m_sbyteArray"), dPdxType.SbyteArray);
      PdxType.GenericCompare((string[]) ret.GetField("m_stringArray"), dPdxType.StringArray);
      PdxType.GenericValCompare((UInt16)(Int16)(ret.GetField("m_uint16")), dPdxType.Uint16);
      PdxType.GenericValCompare((UInt32)(Int32)ret.GetField("m_uint32"), dPdxType.Uint32);
      PdxType.GenericValCompare((UInt64)(Int64)ret.GetField("m_ulong"), dPdxType.Ulong);

      PdxType.GenericCompare((double[]) ret.GetField("m_doubleArray"), dPdxType.DoubleArray);
      PdxType.GenericValCompare((float) ret.GetField("m_float"), dPdxType.Float);
      PdxType.GenericCompare((float[]) ret.GetField("m_floatArray"), dPdxType.FloatArray);
      PdxType.GenericValCompare((short) ret.GetField("m_int16"), dPdxType.Int16);
      PdxType.GenericValCompare((int) ret.GetField("m_int32"), dPdxType.Int32);
      PdxType.GenericValCompare((long) ret.GetField("m_long"), dPdxType.Long);
      PdxType.GenericCompare((int[]) ret.GetField("m_int32Array"), dPdxType.Int32Array);

      string retUInt16ArrayStr = String.Join(",", ((short[])ret.GetField("m_uint16Array")).Select(p => ((ushort)(short)p).ToString()).ToArray());
      string dPdxTypeUShortArrayStr = String.Join(",", dPdxType.Uint16Array.Select(p => p.ToString()).ToArray());
      PdxType.GenericValCompare(retUInt16ArrayStr, dPdxTypeUShortArrayStr);

      string retUintArrayStr = String.Join(",", ((int[])ret.GetField("m_uint32Array")).Select(p => ((uint)(int)p).ToString()).ToArray());
      string dPdxTypeUintArrayStr = String.Join(",", dPdxType.Uint32Array.Select(p => p.ToString()).ToArray());
      PdxType.GenericValCompare(retUintArrayStr, dPdxTypeUintArrayStr);

      string retUlongArrayStr = String.Join(",", ((long[])ret.GetField("m_ulongArray")).Select(p => ((ulong)(long)p).ToString()).ToArray());
      string dPdxTypeUlongArrayStr = String.Join(",", dPdxType.UlongArray.Select(p => p.ToString()).ToArray());
      PdxType.GenericValCompare(retUlongArrayStr, dPdxTypeUlongArrayStr);


      var retbA = (byte[]) ret.GetField("m_byte252");
      if (retbA.Length != 252)
        throw new Exception("Array len 252 not found");

      retbA = (byte[]) ret.GetField("m_byte253");
      if (retbA.Length != 253)
        throw new Exception("Array len 253 not found");

      retbA = (byte[]) ret.GetField("m_byte65535");
      if (retbA.Length != 65535)
        throw new Exception("Array len 65535 not found");

      retbA = (byte[]) ret.GetField("m_byte65536");
      if (retbA.Length != 65536)
        throw new Exception("Array len 65536 not found");

      var ev = (pdxEnumTest) ret.GetField("m_pdxEnum");
      if (ev != dPdxType.PdxEnum)
        throw new Exception("Pdx enum is not equal");

      var addreaaPdxI = (IPdxInstance[]) ret.GetField("m_address");
      Assert.AreEqual(addreaaPdxI.Length, dPdxType.AddressArray.Length);

      Assert.AreEqual(addreaaPdxI[0].GetObject(), dPdxType.AddressArray[0]);


      var objArr = (List<object>) ret.GetField("m_objectArray");
      Assert.AreEqual(objArr.Count, dPdxType.ObjectArray.Count);

      Assert.AreEqual(((IPdxInstance) objArr[0]).GetObject(), dPdxType.ObjectArray[0]);


      ret = (IPdxInstance) region0["pdxput2"];

      var cpi = (IPdxInstance) ret.GetField("_childPdx");

      var cpo = (ChildPdx) cpi.GetObject();

      Assert.AreEqual(cpo, new ChildPdx(1393), "child pdx should be equal");
    }

    private void modifyPdxInstance()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      IPdxInstance newpdxins;
      var pdxins = (IPdxInstance) region0["pdxput"];

      var oldVal = (int) pdxins.GetField("m_int32");

      var iwpi = pdxins.CreateWriter();

      iwpi.SetField("m_int32", oldVal + 1);
      iwpi.SetField("m_string", "change the string");
      region0["pdxput"] = iwpi;

      newpdxins = (IPdxInstance) region0["pdxput"];

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
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((char) newpdxins.GetField("m_char"), 'D', "Char is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");


      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_bool", false);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((bool) newpdxins.GetField("m_bool"), false, "bool is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_byte", (sbyte) 0x75);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((sbyte) newpdxins.GetField("m_byte"), (sbyte) 0x75, "sbyte is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_sbyte", (sbyte) 0x57);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((sbyte) newpdxins.GetField("m_sbyte"), (sbyte) 0x57, "sbyte is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int16", (short) 0x5678);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((short) newpdxins.GetField("m_int16"), (short) 0x5678, "int16 is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_long", (long) 0x56787878);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((long) newpdxins.GetField("m_long"), (long) 0x56787878, "long is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_float", 18389.34f);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((float) newpdxins.GetField("m_float"), 18389.34f, "float is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_float", 18389.34f);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((float) newpdxins.GetField("m_float"), 18389.34f, "float is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_double", 18389.34d);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((double) newpdxins.GetField("m_double"), 18389.34d, "double is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_boolArray", new bool[] {true, false, true, false, true, true, false, true});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((bool[]) newpdxins.GetField("m_boolArray"),
        new bool[] {true, false, true, false, true, true, false, true}, "bool array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_byteArray", new byte[] {0x34, 0x64, 0x34, 0x64});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((byte[]) newpdxins.GetField("m_byteArray"), new byte[] {0x34, 0x64, 0x34, 0x64},
        "byte array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_charArray", new char[] {'c', 'v', 'c', 'v'});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((char[]) newpdxins.GetField("m_charArray"), new char[] {'c', 'v', 'c', 'v'},
        "char array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      var ticks = 634460644691580000L;
      var tdt = new DateTime(ticks);
      iwpi.SetField("m_dateTime", tdt);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((DateTime) newpdxins.GetField("m_dateTime"), tdt, "datetime is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int16Array", new short[] {0x2332, 0x4545, 0x88, 0x898});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((short[]) newpdxins.GetField("m_int16Array"), new short[] {0x2332, 0x4545, 0x88, 0x898},
        "short array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_int32Array", new int[] {23, 676868, 34343});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((int[]) newpdxins.GetField("m_int32Array"), new int[] {23, 676868, 34343},
        "int32 array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_longArray", new long[] {3245435, 3425435});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((long[]) newpdxins.GetField("m_longArray"), new long[] {3245435, 3425435},
        "long array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_floatArray", new float[] {232.565f, 234323354.67f});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((float[]) newpdxins.GetField("m_floatArray"), new float[] {232.565f, 234323354.67f},
        "float array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_doubleArray", new double[] {23423432d, 43242354315d});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
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
      newpdxins = (IPdxInstance) region0["pdxput"];
      var retbb = (byte[][]) newpdxins.GetField("m_byteByteArray");

      PdxType.compareByteByteArray(tmpbb, retbb);

      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_stringArray", new string[] {"one", "two", "eeeee"});
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual((string[]) newpdxins.GetField("m_stringArray"), new string[] {"one", "two", "eeeee"},
        "string array is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var tl = new List<object>();
      tl.Add(new PdxType());
      tl.Add(new byte[] {0x34, 0x55});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_arraylist", tl);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((List<object>) newpdxins.GetField("m_arraylist")).Count, tl.Count, "list<object> is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var map = new Dictionary<object, object>();
      map.Add(1, new bool[] {true, false, true, false, true, true, false, true});
      map.Add(2, new string[] {"one", "two", "eeeee"});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_map", map);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((Dictionary<object, object>) newpdxins.GetField("m_map")).Count, map.Count, "map is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var hashtable = new Hashtable();
      hashtable.Add(1, new string[] {"one", "two", "eeeee"});
      hashtable.Add(2, new int[] {23, 676868, 34343});

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_hashtable", hashtable);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((Hashtable) newpdxins.GetField("m_hashtable")).Count, hashtable.Count, "hashtable is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_pdxEnum", pdxEnumTest.pdx1);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((pdxEnumTest) newpdxins.GetField("m_pdxEnum")), pdxEnumTest.pdx1, "pdx enum is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var vector = new ArrayList();
      vector.Add(1);
      vector.Add(2);

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_vector", vector);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((ArrayList) newpdxins.GetField("m_vector")).Count, vector.Count, "vector is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var chm = CacheableHashSet.Create();
      chm.Add(1);
      chm.Add("jkfdkjdsfl");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_chs", chm);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.True(chm.Equals(newpdxins.GetField("m_chs")), "CacheableHashSet is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var clhs = CacheableLinkedHashSet.Create();
      clhs.Add(111);
      clhs.Add(111343);

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_clhs", clhs);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.True(clhs.Equals(newpdxins.GetField("m_clhs")), "CacheableLinkedHashSet is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");

      var aa = new PdxTests.Address[2];
      for (var i = 0; i < aa.Length; i++)
      {
        aa[i] = new PdxTests.Address(i + 1, "street" + i.ToString(), "city" + i.ToString());
      }

      iwpi = pdxins.CreateWriter();

      iwpi.SetField("m_address", aa);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      var iaa = (IPdxInstance[]) newpdxins.GetField("m_address");
      Assert.AreEqual(iaa.Length, aa.Length, "address array length should equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal for address array");

      var oa = new List<object>();
      oa.Add(new PdxTests.Address(1, "1", "12"));
      oa.Add(new PdxTests.Address(1, "1", "12"));

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("m_objectArray", oa);
      region0["pdxput"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput"];
      Assert.AreEqual(((List<object>) newpdxins.GetField("m_objectArray")).Count, oa.Count,
        "Object arary is not equal");
      Assert.AreNotEqual(pdxins, newpdxins, "PdxInstance should not be equal");


      pdxins = (IPdxInstance) region0["pdxput2"];
      var cpi = (IPdxInstance) pdxins.GetField("_childPdx");

      iwpi = pdxins.CreateWriter();
      iwpi.SetField("_childPdx", new ChildPdx(2));
      region0["pdxput2"] = iwpi;
      newpdxins = (IPdxInstance) region0["pdxput2"];
      Console.WriteLine(pdxins);
      Console.WriteLine(newpdxins);
      Assert.AreNotEqual(pdxins, newpdxins, "parent pdx should be not equal");
      Assert.AreNotEqual(cpi, newpdxins.GetField("_childPdx"), "child pdx instance should be equal");
      Assert.AreEqual(new ChildPdx(2), ((IPdxInstance) (newpdxins.GetField("_childPdx"))).GetObject(),
        "child pdx instance should be equal");
    }

    private void runPdxInstanceTest()
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


      m_client1.Call(pdxPut);
      m_client2.Call(getObject);
      m_client2.Call(verifyPdxInstanceEquals);
      m_client2.Call(verifyPdxInstanceHashcode);
      m_client2.Call(accessPdxInstance);

      m_client2.Call(modifyPdxInstance);

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

    private void InitClientXml(string cacheXml, int serverport1, int serverport2)
    {
      CacheHelper.HOST_PORT_1 = serverport1;
      CacheHelper.HOST_PORT_2 = serverport2;
      CacheHelper.InitConfig(cacheXml);
    }

    private void testReadSerializedXMLProperty()
    {
      Assert.AreEqual(CacheHelper.DCache.GetPdxReadSerialized(), true);
    }

    private void putPdxWithIdentityField()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(SerializePdx.Create);
      var sp = new SerializePdx(true);

      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[0]);

      region0[1] = sp;
    }

    private void verifyPdxIdentityField()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[0]);

      var pi = (IPdxInstance) region0[1];

      Assert.AreEqual(pi.GetFieldNames().Count, 4, "number of fields should be four in SerializePdx");

      Assert.AreEqual(pi.IsIdentityField("i1"), true, "SerializePdx1.i1 should be identity field");

      Assert.AreEqual(pi.IsIdentityField("i2"), false, "SerializePdx1.i2 should NOT be identity field");

      Assert.AreEqual(pi.HasField("i1"), true, "SerializePdx1.i1 should be in PdxInstance stream");

      Assert.AreEqual(pi.HasField("i3"), false, "There is no field i3 in SerializePdx1's PdxInstance stream");

      var javaPdxHC = (int) region0["javaPdxHC"];

      Assert.AreEqual(javaPdxHC, pi.GetHashCode(),
        "Pdxhashcode for identity field object SerializePdx1 not matched with java pdx hash code.");

      var pi2 = (IPdxInstance) region0[1];

      Assert.AreEqual(pi, pi2, "Both pdx instance should equal.");
    }

    private void putPdxWithNullIdentityFields()
    {
      var sp = new SerializePdx(false); //not initialized

      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[0]);

      region0[2] = sp;
    }

    private void verifyPdxNullIdentityFieldHC()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(RegionNames[0]);

      var pi = (IPdxInstance) region0[2];

      var javaPdxHC = (int) region0["javaPdxHC"];

      Assert.AreEqual(javaPdxHC, pi.GetHashCode(),
        "Pdxhashcode for identity field object SerializePdx1 not matched with java pdx hash code.");

      var pi2 = (IPdxInstance) region0[2];

      Assert.AreEqual(pi, pi2, "Both pdx instance should equal.");

      var values = new Dictionary<object, object>();
      var keys = new List<object>();
      keys.Add(1);
      keys.Add(2);
      region0.GetAll(keys, values, null);

      Assert.AreEqual(values.Count, 2, "Getall count should be two");
    }

    private void runPdxReadSerializedTest()
    {
      Util.Log("runPdxReadSerializedTest");

      CacheHelper.SetupJavaServers(false, "cacheserver_pdxinstance_hashcode.xml");
      CacheHelper.StartJavaServer(1, "GFECS1");

      Util.Log("Cacheserver 1 started.");

      m_client1.Call(InitClientXml, "client_pdx.xml", CacheHelper.HOST_PORT_1, CacheHelper.HOST_PORT_2);
      m_client2.Call(InitClientXml, "client_pdx.xml", CacheHelper.HOST_PORT_1, CacheHelper.HOST_PORT_2);

      m_client1.Call(testReadSerializedXMLProperty);
      m_client2.Call(testReadSerializedXMLProperty);

      m_client1.Call(putPdxWithIdentityField);
      m_client2.Call(verifyPdxIdentityField);

      m_client1.Call(putPdxWithNullIdentityFields);
      m_client2.Call(verifyPdxNullIdentityFieldHC);

      m_client1.Call(Close);
      m_client2.Call(Close);

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void PdxIgnoreUnreadFieldTest()
    {
      m_useWeakHashMap = true;
      runPdxIgnoreUnreadFieldTest();
      m_useWeakHashMap = false;
      runPdxIgnoreUnreadFieldTest();
    }

    [Test]
    public void MultipleDSTest()
    {
      runMultipleDSTest();
    }

    [Test]
    public void PdxSerializerTest()
    {
      runPdxSerializerTest();
    }

    [Test]
    public void ReflectionPdxSerializerTest()
    {
      runReflectionPdxSerializerTest();
    }

    [Test]
    public void PdxTestWithNoTypeRegister()
    {
      runPdxTestWithNoTypeRegister();
    }

    [Test]
    public void PdxInstanceTest()
    {
      runPdxInstanceTest();
    }

    [Test]
    public void PdxReadSerializedTest()
    {
      runPdxReadSerializedTest();
    }

    #endregion
  }
}
