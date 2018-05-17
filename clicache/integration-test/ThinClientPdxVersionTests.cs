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

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  using Apache.Geode.Client;
  using Region = Apache.Geode.Client.IRegion<Object, Object>;


  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  class ThinClientPdxVersionTests : ThinClientRegionSteps
  {
    static bool m_useWeakHashMap = false;

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
      //return new ClientBase[] { m_client1, m_client2 };
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

    void cleanup()
    {
      {
        CacheHelper.SetExtraPropertiesFile(null);
        if (m_clients != null)
        {
          foreach (ClientBase client in m_clients)
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

    Assembly m_pdxVesionOneAsm;
    Assembly m_pdxVesionTwoAsm;

    #region "Version Fisrt will be here PdxType1"

    void initializePdxAssemblyOne(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOne);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
      m_useWeakHashMap = useWeakHashmap;
    }

    IPdxSerializable registerPdxTypeOne()
    {
      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    void initializePdxAssemblyTwo(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwo);
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
      m_useWeakHashMap = useWeakHashmap;
    }

    IPdxSerializable registerPdxTypeTwo()
    {
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    void putAtVersionOne11(bool useWeakHashmap)
    {
      initializePdxAssemblyOne(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      object pRet = region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);
    }

    void getPutAtVersionTwo12(bool useWeakHashmap)
    {
      initializePdxAssemblyTwo(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    public void getPutAtVersionOne13()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());
      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    public void getPutAtVersionTwo14()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());
      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    public void getPutAtVersionOne15()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];
      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());
      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      region0[1] = pRet;
      if (m_useWeakHashMap == false)
      {
        Assert.AreEqual(CacheHelper.DCache.GetPdxTypeRegistry().testNumberOfPreservedData(), 0);
      }
      else
      {
        Assert.IsTrue(CacheHelper.DCache.GetPdxTypeRegistry().testNumberOfPreservedData() > 0);
      }
    }

    public void getPutAtVersionTwo16()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");
      object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());
      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;

      if (m_useWeakHashMap == false)
      {
        Assert.AreEqual(CacheHelper.DCache.GetPdxTypeRegistry().testNumberOfPreservedData(), 0);
      }
      else
      {
        //it has extra fields, so no need to preserve data
        Assert.IsTrue(CacheHelper.DCache.GetPdxTypeRegistry().testNumberOfPreservedData() == 0);
      }
    }

    #endregion

    #region Basic merge three PDxType2

    private void initializePdxAssemblyOne2(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOne2);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes2");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeOne2()
    {
      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes2");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxAssemblyTwo2(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwo2);
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes2");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeTwo2()
    {
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes2");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    #endregion

    #region Basic merge three PDxType3

    private void initializePdxAssemblyOne3(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOne3);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeOne3()
    {
      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxAssemblyTwo3(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwo3);
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeTwo3()
    {
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    #endregion
    
    #region PdxType2 Version two first

    private void initializePdxAssemblyOneR2(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOneR2);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR2");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeOneR2()
    {
      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR2");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    private void initializePdxAssemblyTwoR2(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwoR2);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR2");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    private IPdxSerializable registerPdxTypeTwoR2()
    {
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR2");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    #endregion

    void runBasicMergeOps()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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

      m_client1.Call(putAtVersionOne11, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client2.Call(getPutAtVersionTwo12, m_useWeakHashMap);
      Util.Log("StepFour complete.");

      m_client1.Call(getPutAtVersionOne13);
      Util.Log("StepFive complete.");

      m_client2.Call(getPutAtVersionTwo14);
      Util.Log("StepSix complete.");

      for (int i = 0; i < 10; i++)
      {
        m_client1.Call(getPutAtVersionOne15);
        Util.Log("StepSeven complete." + i);

        m_client2.Call(getPutAtVersionTwo16);
        Util.Log("StepEight complete." + i);
      }

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


    void initializePdxAssemblyOnePS(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");


      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestDiffTypePdxS");

      //object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
      m_useWeakHashMap = useWeakHashmap;
    }

    void putFromVersion1_PS(bool useWeakHashmap)
    {
      //local cache is on
      initializePdxAssemblyOnePS(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestDiffTypePdxS");
      object np = pt.InvokeMember("Create", BindingFlags.InvokeMethod, null, null, null);

      CacheHelper.DCache.TypeRegistry.PdxSerializer = (IPdxSerializer) np;

      //created new object
      np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] {true});

      Type keytype = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestKey");
      object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-1"});

      region0[key] = np;

      object pRet = region0[key];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      //this should come from local caching
      pRet = region0.GetLocalView()[key];

      Assert.IsNotNull(pRet);

      region0.GetLocalView().Invalidate(key);
      bool isKNFE = false;
      try
      {
        pRet = region0.GetLocalView()[key];
      }
      catch (Apache.Geode.Client.KeyNotFoundException)
      {
        isKNFE = true;
      }

      Assert.IsTrue(isKNFE);

      pRet = region0[key];

      isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      region0.GetLocalView().Remove(key);
    }

    void initializePdxAssemblyTwoPS(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");


      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestDiffTypePdxS");

      //object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
      m_useWeakHashMap = useWeakHashmap;
    }

    void putFromVersion2_PS(bool useWeakHashmap)
    {
      initializePdxAssemblyTwoPS(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestDiffTypePdxS");

      object np = pt.InvokeMember("Create", BindingFlags.InvokeMethod, null, null, null);

      CacheHelper.DCache.TypeRegistry.PdxSerializer = (IPdxSerializer) np;

      np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] {true});

      Type keytype = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestKey");
      object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-1"});

      region0[key] = np;

      object pRet = region0[key];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-2"});
      region0[key2] = np;
    }

    void getputFromVersion1_PS()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestDiffTypePdxS");
      object np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] {true});


      Type keytype = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestKey");
      object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-1"});


      object pRet = region0[key];

      Assert.IsTrue(np.Equals(pRet));

      //get then put.. this should merge data back
      region0[key] = pRet;

      object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-2"});

      pRet = region0[key2];

      Assert.IsTrue(np.Equals(pRet));

      //get then put.. this should Not merge data back
      region0[key2] = np;
    }

    void getAtVersion2_PS()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestDiffTypePdxS");
      object np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] {true});


      Type keytype = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestKey");
      object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-1"});

      bool gotexcep = false;
      try
      {
        object r = region0.GetLocalView()[key];
      }
      catch (Exception)
      {
        gotexcep = true;
      }

      Assert.IsTrue(gotexcep);

      object pRet = region0[key];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] {"key-2"});

      np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] {true});

      pRet = region0[key2];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      Assert.IsTrue(!np.Equals(pRet));
    }

    void runBasicMergeOpsWithPdxSerializer()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching true*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false /*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");


      m_client1.Call(putFromVersion1_PS, m_useWeakHashMap);
      Util.Log("StepOne complete.");

      m_client2.Call(putFromVersion2_PS, m_useWeakHashMap);
      Util.Log("StepTwo complete.");

      m_client1.Call(getputFromVersion1_PS);
      Util.Log("Stepthree complete.");

      m_client2.Call(getAtVersion2_PS);
      Util.Log("StepFour complete.");

      m_client1.Call(dinitPdxSerializer);
      m_client2.Call(dinitPdxSerializer);

      //m_client1.Call(getPutAtVersionOne13);
      //Util.Log("StepFive complete.");

      //m_client2.Call(getPutAtVersionTwo14);
      //Util.Log("StepSix complete.");

      //for (int i = 0; i < 10; i++)
      //{
      //  m_client1.Call(getPutAtVersionOne15);
      //  Util.Log("StepSeven complete." + i);

      //  m_client2.Call(getPutAtVersionTwo16);
      //  Util.Log("StepEight complete." + i);
      //}

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

    #region "Version two will first here"

    void initializePdxAssemblyOneR1(bool useWeakHashmap)
    {
      m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOneR1);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    IPdxSerializable registerPdxTypeOneR1()
    {
      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    void initializePdxAssemblyTwoR1(bool useWeakHashmap)
    {
      m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

      CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwoR1);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");

      object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        new object[] {useWeakHashmap});
    }

    IPdxSerializable registerPdxTypeTwoR1()
    {
      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");

      object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null,
        null);

      return (IPdxSerializable) ob;
    }

    public void putAtVersionTwo1(bool useWeakHashmap)
    {
      initializePdxAssemblyTwoR1(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      Assert.AreEqual(np, pRet);
    }

    public void getPutAtVersionOne2(bool useWeakHashmap)
    {
      initializePdxAssemblyOneR1(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    public void getPutAtVersionTwo3()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1]; //get

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    public void getPutAtVersionOne4()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    public void getPutAtVersionTwo5()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    public void getPutAtVersionOne6()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");
      object np = pt.InvokeMember("PdxTypesR1", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    #endregion

    void runBasicMergeOpsR1()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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


      m_client2.Call(putAtVersionTwo1, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client1.Call(getPutAtVersionOne2, m_useWeakHashMap);
      Util.Log("StepFour complete.");

      m_client2.Call(getPutAtVersionTwo3);
      Util.Log("StepFive complete.");

      m_client1.Call(getPutAtVersionOne4);
      Util.Log("StepSix complete.");

      for (int i = 0; i < 10; i++)
      {
        m_client2.Call(getPutAtVersionTwo5);
        Util.Log("StepSeven complete." + i);

        m_client1.Call(getPutAtVersionOne6);
        Util.Log("StepEight complete." + i);
      }

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

    void dinitPdxSerializer()
    {
      CacheHelper.DCache.TypeRegistry.PdxSerializer = null;
    }

    private void runBasicMergeOps2()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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


      m_client1.Call(putAtVersionOne21, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client2.Call(getPutAtVersionTwo22, m_useWeakHashMap);

      for (int i = 0; i < 10; i++)
      {
        m_client1.Call(getPutAtVersionOne23);
        m_client2.Call(getPutAtVersionTwo24);

        Util.Log("step complete " + i);
      }


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

    private void runBasicMergeOps3()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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


      m_client1.Call(putAtVersionOne31, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client2.Call(getPutAtVersionTwo32, m_useWeakHashMap);

      for (int i = 0; i < 10; i++)
      {
        m_client1.Call(getPutAtVersionOne33);
        m_client2.Call(getPutAtVersionTwo34);

        Util.Log("step complete " + i);
      }


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

    private void putAtVersionOne21(bool useWeakHashmap)
    {
      initializePdxAssemblyOne2(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes2");
      object np = pt.InvokeMember("PdxTypes2", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      object pRet = region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);

      pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.Pdx1");
      object pdx1 = pt.InvokeMember("Pdx1", BindingFlags.CreateInstance, null, null, null);

      for (int i = 1000; i < 1010; i++)
      {
        region0[i] = pdx1;
      }
    }

    private void getPutAtVersionTwo22(bool useWeakHashmap)
    {
      initializePdxAssemblyTwo2(useWeakHashmap);
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes2");
      object np = pt.InvokeMember("PdxTypes2", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;

      pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.Pdx1");
      object pdx1 = pt.InvokeMember("Pdx1", BindingFlags.CreateInstance, null, null, null);

      for (int i = 1000; i < 1010; i++)
      {
        object ret = region0[i];
      }
    }

    private void getPutAtVersionOne23()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes2");
      object np = pt.InvokeMember("PdxTypes2", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());
      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    private void getPutAtVersionTwo24()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes2");
      object np = pt.InvokeMember("PdxTypes2", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    private void putAtVersionOne31(bool useWeakHashmap)
    {
      initializePdxAssemblyOne3(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");
      object np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      object pRet = region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);
      Assert.IsTrue(isEqual);
    }

    private void getPutAtVersionTwo32(bool useWeakHashmap)
    {
      initializePdxAssemblyTwo3(useWeakHashmap);
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");
      object np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    private void getPutAtVersionOne33()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes3");
      object np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    private void getPutAtVersionTwo34()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes3");
      object np = pt.InvokeMember("PdxTypes3", BindingFlags.CreateInstance, null, null, null);

      object pRet = (object) region0[1];

      Console.WriteLine(np.ToString());
      Console.WriteLine(pRet.ToString());

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);

      region0[1] = pRet;
    }

    private void runBasicMergeOpsR2()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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


      m_client2.Call(putAtVersionTwoR21, m_useWeakHashMap);
      Util.Log("StepThree complete.");

      m_client1.Call(getPutAtVersionOneR22, m_useWeakHashMap);
      Util.Log("StepFour complete.");

      for (int i = 0; i < 10; i++)
      {
        m_client2.Call(getPutAtVersionTwoR23);
        Util.Log("StepFive complete.");

        m_client1.Call(getPutAtVersionOneR24);
        Util.Log("StepSix complete.");
      }

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

    private void putAtVersionTwoR21(bool useWeakHashmap)
    {
      initializePdxAssemblyTwoR2(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR2");
      object np = pt.InvokeMember("PdxTypesR2", BindingFlags.CreateInstance, null, null, null);
      region0[1] = np;

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool isEqual = np.Equals(pRet);

      Assert.IsTrue(isEqual);
    }

    private void getPutAtVersionOneR22(bool useWeakHashmap)
    {
      initializePdxAssemblyOneR2(useWeakHashmap);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR2");
      object np = pt.InvokeMember("PdxTypesR2", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    private void getPutAtVersionTwoR23()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR2");
      object np = pt.InvokeMember("PdxTypesR2", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1]; //get

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    private void getPutAtVersionOneR24()
    {
      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR2");
      object np = pt.InvokeMember("PdxTypesR2", BindingFlags.CreateInstance, null, null, null);

      object pRet = region0[1];

      Console.WriteLine(np);
      Console.WriteLine(pRet);

      bool retVal = np.Equals(pRet);
      Assert.IsTrue(retVal);

      region0[1] = pRet;
    }

    int nPdxPuts = 100000;
    int pdxobjsize = 5000;

    int nBAPuts = 25;
    int baSize = 16240000;

    string testSysPropFileName = "testLR.properties";

    #region Tests

    [Test]
    public void BasicMergeOps()
    {
      m_useWeakHashMap = true;
      runBasicMergeOps();
      m_useWeakHashMap = false;
      runBasicMergeOps();
    }

    [Test]
    public void BasicMergeOpsWithPdxSerializer()
    {
      m_useWeakHashMap = true;
      runBasicMergeOpsWithPdxSerializer();
      m_useWeakHashMap = false;
      runBasicMergeOpsWithPdxSerializer();
    }

    [Test]
    public void BasicMergeOpsR1() //first register with higher version
    {
      m_useWeakHashMap = true;
      runBasicMergeOpsR1();
      m_useWeakHashMap = false;
      runBasicMergeOpsR1();
    }

    [Test]
    public void BasicMergeOpsR2() //first register with higher version
    {
      m_useWeakHashMap = true;
      runBasicMergeOpsR2();
      m_useWeakHashMap = false;
      runBasicMergeOpsR2();
    }

    [Test]
    public void BasicMergeOps2() //first register with higher version
    {
      m_useWeakHashMap = true;
      runBasicMergeOps2();
      m_useWeakHashMap = false;
      runBasicMergeOps2();
    }

    [Test]
    public void BasicMergeOps3() //first register with higher version
    {
      m_useWeakHashMap = true;
      runBasicMergeOps3();
      m_useWeakHashMap = false;
      runBasicMergeOps3();
    }

    #endregion
  }
}