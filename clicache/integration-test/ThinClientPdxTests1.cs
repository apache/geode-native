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
  class ThinClientPdxTests1 : ThinClientRegionSteps
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
      return new ClientBase[] { m_client1, m_client2, m_client3, m_client4 };
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
      try {
        m_client1.Call(DestroyRegions);
        m_client2.Call(DestroyRegions);
        CacheHelper.ClearEndpoints();
        CacheHelper.ClearLocators();
      }
      finally {
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

    void PutAndVerifyPdxInGet()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);

      Region region0 = CacheHelper.GetVerifyRegion<object,object>(m_regionNames[0]);

      region0[1] = new PdxType();

      PdxType pRet = (PdxType)region0[1];
      checkPdxInstanceToStringAtServer(region0);

      Assert.AreEqual(CacheHelper.DCache.GetPdxReadSerialized(), false, "Pdx read serialized property should be false.");

    }

     void VerifyGetOnly()
     {
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxType.CreateDeserializable);

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       
       PdxType pRet = (PdxType)region0[1];
       checkPdxInstanceToStringAtServer(region0);


     }

    void PutAndVerifyVariousPdxTypes()
    {
      var typeRegistry = CacheHelper.DCache.TypeRegistry;
      typeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTypes9.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTests.PortfolioPdx.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTests.PositionPdx.CreateDeserializable);
      typeRegistry.RegisterPdxType(PdxTests.AllPdxTypes.Create);


      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      {
        PdxTypes1 p1 = new PdxTypes1();
        region0[11] = p1;
        PdxTypes1 pRet = (PdxTypes1)region0[11];
        Assert.AreEqual(p1, pRet);
        checkPdxInstanceToStringAtServer(region0);

      }

      {
        PdxTypes2 p2 = new PdxTypes2();
        region0[12] = p2;
        PdxTypes2 pRet2 = (PdxTypes2)region0[12];
        Assert.AreEqual(p2, pRet2);
        checkPdxInstanceToStringAtServer(region0);

      }

      {
        PdxTypes3 p3 = new PdxTypes3();
        region0[13] = p3;
        PdxTypes3 pRet3 = (PdxTypes3)region0[13];
        Assert.AreEqual(p3, pRet3);
        checkPdxInstanceToStringAtServer(region0);

      }

      {
        PdxTypes4 p4 = new PdxTypes4();
        region0[14] = p4;
        PdxTypes4 pRet4 = (PdxTypes4)region0[14];
        Assert.AreEqual(p4, pRet4);
        checkPdxInstanceToStringAtServer(region0);

      }

      {
        PdxTypes5 p5 = new PdxTypes5();
        region0[15] = p5;
        PdxTypes5 pRet5 = (PdxTypes5)region0[15];
        Assert.AreEqual(p5, pRet5);
        checkPdxInstanceToStringAtServer(region0);
      }

      {
        PdxTypes6 p6 = new PdxTypes6();
        region0[16] = p6;
        PdxTypes6 pRet6 = (PdxTypes6)region0[16];
        Assert.AreEqual(p6, pRet6);
        checkPdxInstanceToStringAtServer(region0);
      }

      {
        PdxTypes7 p7 = new PdxTypes7();
        region0[17] = p7;
        PdxTypes7 pRet7 = (PdxTypes7)region0[17];
        Assert.AreEqual(p7, pRet7);
        checkPdxInstanceToStringAtServer(region0);
      }

      {
        PdxTypes8 p8 = new PdxTypes8();
        region0[18] = p8;
        PdxTypes8 pRet8 = (PdxTypes8)region0[18];
        Assert.AreEqual(p8, pRet8);
        checkPdxInstanceToStringAtServer(region0);
      }
      {
        PdxTypes9 p9 = new PdxTypes9();
        region0[19] = p9;
        PdxTypes9 pRet9 = (PdxTypes9)region0[19];
        Assert.AreEqual(p9, pRet9);
        checkPdxInstanceToStringAtServer(region0);
      }

      {
        PortfolioPdx pf = new PortfolioPdx(1001, 10);
        region0[20] = pf;
        PortfolioPdx retpf = (PortfolioPdx)region0[20];
        checkPdxInstanceToStringAtServer(region0);
        //Assert.AreEqual(p9, pRet9);
      }

      {
        PortfolioPdx pf = new PortfolioPdx(1001, 10, new string[] { "one", "two", "three" });
        region0[21] = pf;
        PortfolioPdx retpf = (PortfolioPdx)region0[21];
        checkPdxInstanceToStringAtServer(region0);
        //Assert.AreEqual(p9, pRet9);
      }
      {
        PdxTypes10 p10 = new PdxTypes10();
        region0[22] = p10;
        PdxTypes10 pRet10 = (PdxTypes10)region0[22];
        Assert.AreEqual(p10, pRet10);
        checkPdxInstanceToStringAtServer(region0);
      }
      {
        AllPdxTypes apt = new AllPdxTypes(true);
        region0[23] = apt;
        AllPdxTypes aptRet = (AllPdxTypes)region0[23];
        Assert.AreEqual(apt, aptRet);
        checkPdxInstanceToStringAtServer(region0);
      }
    }

     void VerifyVariousPdxGets()
     {
       var typeRegistry = CacheHelper.DCache.TypeRegistry;
       typeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTypes9.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTests.PortfolioPdx.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTests.PositionPdx.CreateDeserializable);
       typeRegistry.RegisterPdxType(PdxTests.AllPdxTypes.Create);

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       {
         PdxTypes1 p1 = new PdxTypes1();
         PdxTypes1 pRet = (PdxTypes1)region0[11];
         Assert.AreEqual(p1, pRet);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes2 p2 = new PdxTypes2();
         PdxTypes2 pRet2 = (PdxTypes2)region0[12];
         Assert.AreEqual(p2, pRet2);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes3 p3 = new PdxTypes3();
         PdxTypes3 pRet3 = (PdxTypes3)region0[13];
         Assert.AreEqual(p3, pRet3);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes4 p4 = new PdxTypes4();
         PdxTypes4 pRet4 = (PdxTypes4)region0[14];
         Assert.AreEqual(p4, pRet4);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes5 p5 = new PdxTypes5();
         PdxTypes5 pRet5 = (PdxTypes5)region0[15];
         Assert.AreEqual(p5, pRet5);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes6 p6 = new PdxTypes6();
         PdxTypes6 pRet6 = (PdxTypes6)region0[16];
         Assert.AreEqual(p6, pRet6);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes7 p7 = new PdxTypes7();
         PdxTypes7 pRet7 = (PdxTypes7)region0[17];
         Assert.AreEqual(p7, pRet7);
         checkPdxInstanceToStringAtServer(region0);
       }

       {
         PdxTypes8 p8 = new PdxTypes8();
         PdxTypes8 pRet8 = (PdxTypes8)region0[18];
         Assert.AreEqual(p8, pRet8);
         checkPdxInstanceToStringAtServer(region0);
       }
       {
         PdxTypes9 p9 = new PdxTypes9();
         PdxTypes9 pRet9 = (PdxTypes9)region0[19];
         Assert.AreEqual(p9, pRet9);
         checkPdxInstanceToStringAtServer(region0);
       }       
       {
         PortfolioPdx retpf = (PortfolioPdx)region0[20];
         checkPdxInstanceToStringAtServer(region0);
       }
       {
         PortfolioPdx retpf = (PortfolioPdx)region0[21];
         checkPdxInstanceToStringAtServer(region0);
       }
       {
         PdxTypes10 p10 = new PdxTypes10();
         PdxTypes10 pRet10 = (PdxTypes10)region0[22];
         Assert.AreEqual(p10, pRet10);
         checkPdxInstanceToStringAtServer(region0);
       }
       {
         AllPdxTypes apt = new AllPdxTypes(true);
         AllPdxTypes aptRet = (AllPdxTypes)region0[23];
         Assert.AreEqual(apt, aptRet);
         checkPdxInstanceToStringAtServer(region0);
       }
     }

     void checkPdxInstanceToStringAtServer(Region region)
     {
       bool retVal = (bool)region["success"];
       Assert.IsTrue(retVal);
     }

    void runPdxDistOps()
    {

      CacheHelper.SetupJavaServers(true, "cacheserverPdx.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(PutAndVerifyPdxInGet);
      Util.Log("StepThree complete.");

      m_client2.Call(VerifyGetOnly);
      Util.Log("StepFour complete.");

      m_client1.Call(PutAndVerifyVariousPdxTypes);
      Util.Log("StepFive complete.");

      m_client2.Call(VerifyVariousPdxGets);
      Util.Log("StepSeven complete.");
      
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

     void VerifyDataOutputAdvance()
     {
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(MyClass.Create);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(MyClasses.Create);

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       MyClasses mcs = new MyClasses("1", 1000);

       region0[1] = mcs;

       object ret = region0[1];

       Assert.AreEqual(mcs, ret);
     }

     void runPdxDistOps2()
     {


        CacheHelper.SetupJavaServers(true, "cacheserverPdxSerializer.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

        m_client1.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepOne (pool locators) complete.");

        m_client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepTwo (pool locators) complete.");


       m_client1.Call(VerifyDataOutputAdvance);
       Util.Log("StepThree complete.");

    
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

    void PutAndVerifyNestedPdxInGet()
    {
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);

      Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      NestedPdx np = new NestedPdx();
      region0[1] = np;

      NestedPdx pRet = (NestedPdx)region0[1];

      Assert.AreEqual(np, pRet);
    }

     void VerifyNestedGetOnly()
     {
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);


       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       NestedPdx orig = new NestedPdx();
       NestedPdx pRet = (NestedPdx)region0[1];

       Assert.AreEqual(orig, pRet);
     }

     void runNestedPdxOps()
     {


        CacheHelper.SetupJavaServers(true, "cacheserver.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

        m_client1.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepOne (pool locators) complete.");

        m_client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepTwo (pool locators) complete.");


       m_client1.Call(PutAndVerifyNestedPdxInGet);
       Util.Log("StepThree complete.");

       m_client2.Call(VerifyNestedGetOnly);
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

     void PutAndVerifyPdxInIGFSInGet()
     {
       try
       {
         CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(PdxInsideIGeodeSerializable.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
       }
       catch (Exception )
       { 
       }

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       PdxInsideIGeodeSerializable np = new PdxInsideIGeodeSerializable();
       region0[1] = np;

       PdxInsideIGeodeSerializable pRet = (PdxInsideIGeodeSerializable)region0[1];

       Assert.AreEqual(np, pRet);
     }

     void VerifyPdxInIGFSGetOnly()
     {
       try
       {
         CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(PdxInsideIGeodeSerializable.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
       }
       catch (Exception )
       { }


       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       PdxInsideIGeodeSerializable orig = new PdxInsideIGeodeSerializable();
       PdxInsideIGeodeSerializable pRet = (PdxInsideIGeodeSerializable)region0[1];

       Assert.AreEqual(orig, pRet);
     }

     void runPdxInIGFSOps()
     {


        CacheHelper.SetupJavaServers(true, "cacheserver.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

        m_client1.Call(CreateTCRegions_Pool, RegionNames,
            CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepOne (pool locators) complete.");

        m_client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepTwo (pool locators) complete.");

       m_client1.Call(PutAndVerifyPdxInIGFSInGet);
       Util.Log("StepThree complete.");

       m_client2.Call(VerifyPdxInIGFSGetOnly);
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

     void JavaPutGet_LinedListType()
     {
       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       
       //Do some put to invike attached listener,
       region0[1] = 123;

       //Get
       int value = (int)region0[1];
       //Util.Log("JavaPutGet_LinedListType value received = " + value);
       
       //verify that listener methods have been called.
       Assert.IsTrue((bool)region0["success"]);

       //LinkedList validation
       LinkedList<Object> myList1 = new LinkedList<Object>();
       myList1.AddFirst("Manan");
       myList1.AddLast("Nishka");

       //get the JSON document (as PdxInstance) that has been put from java in attached cacheListener code.
       IPdxInstance ret = (IPdxInstance)region0["jsondoc1"];
       LinkedList<Object> linkedList = (LinkedList<Object>)ret.GetField("kids");
       
       //verify sizes
       Assert.AreEqual((linkedList.Count == myList1.Count), true, " LinkedList size should be equal.");

       LinkedList<Object>.Enumerator e1 = linkedList.GetEnumerator();
       LinkedList<Object>.Enumerator e2 = myList1.GetEnumerator();
             
       //verify content of LinkedList
       while (e1.MoveNext() && e2.MoveNext())
       {
         //Util.Log("JavaPutGet_LinedListType Kids = " + e1.Current);
         PdxType.GenericValCompare(e1.Current, e2.Current);
       }
        
        Util.Log("successfully completed JavaPutGet_LinedListType");
     }
      
     void JavaPutGet()
     {
       try
       {
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTests.PdxType.CreateDeserializable);         
       }
       catch (Exception )
       {
       }

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       PdxType np = new PdxType();
       region0[1] = np;

       PdxType pRet = (PdxType)region0[1];

       //Assert.AreEqual(np, pRet);

       Assert.IsTrue((bool)region0["success"]);
     }

     void JavaGet()
     {
       try
       {
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTests.PdxType.CreateDeserializable);
       }
       catch (Exception )
       {
       }

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       
       PdxType np = new PdxType();
       
       PdxType pRet = (PdxType)region0[1];

       PdxType putFromjava = (PdxType)region0["putFromjava"];
     }

     void runJavaInterOpsWithLinkedListType()
     {
 

        CacheHelper.SetupJavaServers(true, "cacheserverForPdx.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
        Util.Log("Cacheserver 1 started.");

        m_client1.Call(CreateTCRegions_Pool_PDXWithLL, RegionNames,
              CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("Clinet-1 CreateTCRegions_Pool_PDXWithLL (pool with locator) completed.");

        m_client1.Call(JavaPutGet_LinedListType);
        Util.Log("JavaPutGet_LinedListType complete.");

       
        m_client1.Call(Close);
        Util.Log("Client 1 closed");

        CacheHelper.StopJavaServer(1);
        Util.Log("Cacheserver 1 stopped.");

        CacheHelper.StopJavaLocator(1);
        Util.Log("Locator 1 stopped.");

        CacheHelper.ClearEndpoints();
        CacheHelper.ClearLocators();
     }

     void runJavaInteroperableOps()
     {

        CacheHelper.SetupJavaServers(true, "cacheserverForPdx.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

        m_client1.Call(CreateTCRegions_Pool, RegionNames,
            CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepOne (pool locators) complete.");

        m_client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
        Util.Log("StepTwo (pool locators) complete.");


       m_client1.Call(JavaPutGet);
       Util.Log("StepThree complete.");

       m_client2.Call(JavaGet);
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

     void putallAndGetallPdx()
     {
       try
       {
         CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(PdxInsideIGeodeSerializable.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes7.CreateDeserializable);
         CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes8.CreateDeserializable);
       }
       catch (Exception )
       { }

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
       IDictionary<object, object> all = new Dictionary<object, object>();

       PdxTypes1 p1 = new PdxTypes1();
       PdxTypes2 p2 = new PdxTypes2();
       PdxTypes3 p3 = new PdxTypes3();
       PdxTypes4 p4 = new PdxTypes4();
       PdxTypes5 p5 = new PdxTypes5();
       PdxTypes6 p6 = new PdxTypes6();
       PdxTypes7 p7 = new PdxTypes7();
       PdxTypes8 p8 = new PdxTypes8();

       all.Add(21, p1);
       all.Add(22, p2);
       all.Add(23, p3);
       all.Add(24, p4);
       all.Add(25, p5);
       all.Add(26, p6);
       all.Add(27, p7);
       all.Add(28, p8);
       region0.PutAll(all);
       
       
       ICollection<object> keys = new List<object>();
       IDictionary<object, object> getall = new Dictionary<object, object>();

       keys.Add(21);
       keys.Add(22);
       keys.Add(23);
       keys.Add(24);
       keys.Add(25);
       keys.Add(26);
       keys.Add(27);
       keys.Add(28);
       //keys.Add(p1);
       //keys.Add(p2);
       region0.GetAll(keys, getall, null);
       foreach (KeyValuePair<object, object> kv in all)
       {
         object key = kv.Key;
         Util.Log("putall keys "+ key.GetType() + " : " + key);
       }
       //IEnumerator<KeyValuePair<object, object>> ie = getall.GetEnumerator();
       foreach (KeyValuePair<object, object> kv in getall)
       {
         object key = kv.Key;
         if (key != null)
           Util.Log("got key " + key.GetType() + " : " + key);
         else
           Util.Log("got NULL key ");
         object origVal = all[key];
         Assert.AreEqual(kv.Value, origVal);
       }
     }

     
     void runPutAllGetAllOps()
     {


        CacheHelper.SetupJavaServers(true, "cacheserver.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

         m_client1.Call(CreateTCRegions_Pool, RegionNames,
             CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
         Util.Log("StepOne (pool locators) complete.");

         m_client2.Call(CreateTCRegions_Pool, RegionNames,
           CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
         Util.Log("StepTwo (pool locators) complete.");
      
       m_client1.Call(putallAndGetallPdx);
       Util.Log("StepThree complete.");

       m_client2.Call(putallAndGetallPdx);
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
     void LocalOpsStep()
     {
         try
         {
            CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(PdxTests.PdxInsideIGeodeSerializable.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
            CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);

         }
         catch (Exception)
         { }
         Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
         IRegion<object, object> localregion = region0.GetLocalView();
          

         PdxTypes1 p1 = new PdxTypes1();
         string x = "";
         localregion.Add(p1, x);
         object val = localregion[p1];
         //object val = region0[p1];
         val = localregion[p1];
         val = localregion[p1];
         Assert.IsTrue(val.Equals(x), "value should be equal");
         Assert.IsTrue(localregion.Remove(new KeyValuePair<Object, Object>(p1, x)), "Result of remove should be true, as this value null exists locally.");
         Assert.IsFalse(localregion.ContainsKey(p1), "containsKey should be false");
         try
         {
             localregion[p1] = null;
             Assert.Fail("Expected IllegalArgumentException here for put");
         }
         catch (IllegalArgumentException)
         {
             Util.Log("Got Expected IllegalArgumentException");
         }

         localregion[p1] = 1;
         localregion.Invalidate(p1);
         try
         {
             object retVal = localregion[p1];
         }
         catch (Apache.Geode.Client.KeyNotFoundException)
         {
             Util.Log("Got expected KeyNotFoundException exception");
         }
         Assert.IsFalse(localregion.Remove(new KeyValuePair<Object, Object>(p1, 1)), "Result of remove should be false, as this value does not exists locally.");
         Assert.IsTrue(localregion.ContainsKey(p1), "containsKey should be true");
         localregion[p1] = 1;
         Assert.IsTrue(localregion.Remove(p1), "Result of remove should be true, as this value exists locally.");
         Assert.IsFalse(localregion.ContainsKey(p1), "containsKey should be false");

         PdxTypes2 p2 = new PdxTypes2();
         localregion.Add(p2, 1);
         object intVal1 = localregion[p2]; // local get work for pdx object as key but it wont work with caching enable. Throws KeyNotFoundException.
         Assert.IsTrue(intVal1.Equals(1), "intVal should be 1.");
         
         PdxTypes3 p3 = new PdxTypes3();
         localregion.Add(p3, "testString");
         if (localregion.ContainsKey(p3))
         {
             object strVal1 = localregion[p3];
             Assert.IsTrue(strVal1.Equals("testString"), "strVal should be testString.");
         }

         try
         {
             if (localregion.ContainsKey(p3))
             {
                 localregion.Add(p3, 11);
                 Assert.Fail("Expected EntryExistException here");
             }
         }
         catch (EntryExistsException)
         {
             Util.Log(" Expected EntryExistsException exception thrown by localCreate");
         }

         PdxTypes4 p4 = new PdxTypes4();
         localregion.Add(p4, p1);
         object objVal1 = localregion[p4];
         Assert.IsTrue(objVal1.Equals(p1), "valObject and objVal should match.");
         Assert.IsTrue(localregion.Remove(new KeyValuePair<Object, Object>(p4, p1)), "Result of remove should be true, as this value exists locally.");
         Assert.IsFalse(localregion.ContainsKey(p4), "containsKey should be false");
         localregion[p4] = p1;
         Assert.IsTrue(localregion.Remove(p4), "Result of remove should be true, as this value exists locally.");
         Assert.IsFalse(localregion.ContainsKey(p4), "containsKey should be false");

         PdxTypes5 p5 = new PdxTypes5();

         //object cval = region0[p1]; //this will only work when caching is enable else throws KeyNotFoundException
         
         localregion.Clear();

     }
     void runLocalOps()
     {

       CacheHelper.SetupJavaServers(true, "cacheserver.xml");
       CacheHelper.StartJavaLocator(1, "GFELOC");
       Util.Log("Locator 1 started.");
       CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

       m_client1.Call(CreateTCRegions_Pool, RegionNames,
           CacheHelper.Locators, "__TESTPOOL1_", false, false, true/*local caching false*/);
       Util.Log("StepOne (pool locators) complete.");


       m_client1.Call(LocalOpsStep);
       Util.Log("localOps complete.");

       CacheHelper.StopJavaServer(1);
       Util.Log("Cacheserver 1 stopped.");

        CacheHelper.StopJavaLocator(1);
        Util.Log("Locator 1 stopped.");

       CacheHelper.ClearEndpoints();
       CacheHelper.ClearLocators();
     }
     Assembly m_pdxVesionOneAsm;
     Assembly m_pdxVesionTwoAsm;

     #region "Version Fisrt will be here PdxType1"
     void initializePdxAssemblyOne(bool useWeakHashmap)
     {
       m_pdxVesionOneAsm = Assembly.LoadFrom("PdxVersion1Lib.dll");
       
       CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeOne);

       Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");

       object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
       m_useWeakHashMap = useWeakHashmap;
     }

     IPdxSerializable registerPdxTypeOne()
     {
       Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");

       object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, null);

       return (IPdxSerializable)ob;
     }

     void initializePdxAssemblyTwo(bool useWeakHashmap)
     {
       m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

       CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwo);
       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");

       object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
       m_useWeakHashMap = useWeakHashmap;
     }
     IPdxSerializable registerPdxTypeTwo()
     {
       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");

       object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, null);

       return (IPdxSerializable)ob;
     }
    
     void putAtVersionOne11(bool useWeakHashmap)
     {
       initializePdxAssemblyOne(useWeakHashmap);

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypes1");
       object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance , null, null, null);
       region0[1] = np;

       object pRet = region0[1];

       Console.WriteLine( np.ToString());
       Console.WriteLine( pRet.ToString());

       bool isEqual = np.Equals(pRet);
       Assert.IsTrue(isEqual);

     }

     void getPutAtVersionTwo12(bool useWeakHashmap)
     {
       initializePdxAssemblyTwo(useWeakHashmap);

       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypes1");
       object np = pt.InvokeMember("PdxTypes1", BindingFlags.CreateInstance , null, null, null);

       object pRet = (object)region0[1];

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

         object pRet = (object)region0[1];

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

       object pRet = (object)region0[1];

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
     void runBasicMergeOps()
     {

        CacheHelper.SetupJavaServers(true, "cacheserver.xml");
        CacheHelper.StartJavaLocator(1, "GFELOC");
        Util.Log("Locator 1 started.");
        CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
       Util.Log("Cacheserver 1 started.");

         m_client1.Call(CreateTCRegions_Pool, RegionNames,
             CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
         Util.Log("StepOne (pool locators) complete.");

         m_client2.Call(CreateTCRegions_Pool, RegionNames,
           CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
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

       CacheHelper.DCache.TypeRegistry.PdxSerializer = (IPdxSerializer)np;

       //created new object
       np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] { true });

       Type keytype = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestKey");
       object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-1" });

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
       catch (Apache.Geode.Client.KeyNotFoundException )
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

       CacheHelper.DCache.TypeRegistry.PdxSerializer = (IPdxSerializer)np;

       np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] { true });

       Type keytype = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestKey");
       object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-1" });

       region0[key] = np;

       object pRet = region0[key];

       Console.WriteLine(np.ToString());
       Console.WriteLine(pRet.ToString());

       bool isEqual = np.Equals(pRet);
       Assert.IsTrue(isEqual);

       object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-2" });
       region0[key2] = np;
     }


     void getputFromVersion1_PS()
     {
       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestDiffTypePdxS");
       object np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] { true });


       Type keytype = m_pdxVesionOneAsm.GetType("PdxVersionTests.TestKey");
       object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-1" });


       object pRet = region0[key];

       Assert.IsTrue(np.Equals(pRet));

       //get then put.. this should merge data back
       region0[key] = pRet;

       object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-2" });

       pRet = region0[key2];

       Assert.IsTrue(np.Equals(pRet));

       //get then put.. this should Not merge data back
       region0[key2] = np;

     }

     void getAtVersion2_PS()
     {
       Region region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestDiffTypePdxS");
       object np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] { true });


       Type keytype = m_pdxVesionTwoAsm.GetType("PdxVersionTests.TestKey");
       object key = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-1" });

       bool gotexcep = false;
       try
       {
         object r = region0.GetLocalView()[key];
       }
       catch (Exception )
       {
         gotexcep = true;
       }
       Assert.IsTrue(gotexcep);

       object pRet = region0[key];

       Console.WriteLine(np.ToString());
       Console.WriteLine(pRet.ToString());

       bool isEqual = np.Equals(pRet);
       Assert.IsTrue(isEqual);

       object key2 = keytype.InvokeMember("TestKey", BindingFlags.CreateInstance, null, null, new object[] { "key-2" });

       np = pt.InvokeMember("TestDiffTypePdxS", BindingFlags.CreateInstance, null, null, new object[] { true });

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
            CacheHelper.Locators, "__TESTPOOL1_", false, false, true/*local caching true*/);
        Util.Log("StepOne (pool locators) complete.");

        m_client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
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

       object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });

     }

     IPdxSerializable registerPdxTypeOneR1()
     {
       Type pt = m_pdxVesionOneAsm.GetType("PdxVersionTests.PdxTypesR1");

       object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, null);

       return (IPdxSerializable)ob;
     }

     void initializePdxAssemblyTwoR1(bool useWeakHashmap)
     {
       m_pdxVesionTwoAsm = Assembly.LoadFrom("PdxVersion2Lib.dll");

       CacheHelper.DCache.TypeRegistry.RegisterPdxType(registerPdxTypeTwoR1);

       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");

       object ob = pt.InvokeMember("Reset", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, new object[] { useWeakHashmap });
     }

     IPdxSerializable registerPdxTypeTwoR1()
     {
       Type pt = m_pdxVesionTwoAsm.GetType("PdxVersionTests.PdxTypesR1");

       object ob = pt.InvokeMember("CreateDeserializable", BindingFlags.Default | BindingFlags.InvokeMethod, null, null, null);

       return (IPdxSerializable)ob;
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
         
         object pRet = region0[1];//get

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
             CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
         Util.Log("StepOne (pool locators) complete.");

         m_client2.Call(CreateTCRegions_Pool, RegionNames,
           CacheHelper.Locators, "__TESTPOOL1_", false, false, false/*local caching false*/);
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

     int nPdxPuts = 100000;
     int pdxobjsize = 5000;

     int nBAPuts = 25;
     int baSize = 16240000;

     string testSysPropFileName = "testLR.properties";

     #region Tests
     
     [Test]
     public void DistOps()
     {
       runPdxDistOps();
     }

     [Test]
     public void DistOps2()
     {
       runPdxDistOps2();
     }

     [Test]
     public void NestedPdxOps()
     {
       runNestedPdxOps();
     }

     [Test]
     public void PdxInIGFSOps()
     {
       runPdxInIGFSOps();
     }
     
     [Test]
     public void JavaInteroperableOps()
     {
       runJavaInteroperableOps();
     }
     
     [Test]
     public void JavaInterOpsWithLinkedListType()
     {
       runJavaInterOpsWithLinkedListType();
     }

     [Test]
     public void PutAllGetAllOps()
     {
       runPutAllGetAllOps();
     }

    #endregion
   }

   #region IpDxSerializer stuff
  public class SerializePdx : IPdxSerializable
  {
    [PdxIdentityField]
    public int i1;
    public int i2;
    public string s1;
    public string s2;

    /*public static SerializePdx1 CreateDeserializable()
    {
      return new SerializePdx1(false);
    }*/

    //public SerializePdx()
    //{
    //}

    public static SerializePdx Create()
    {
      return new SerializePdx(false);
    }
    public SerializePdx(bool init)
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

      SerializePdx other = obj as SerializePdx;

      if (obj == null)
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
    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      i1 = reader.ReadInt("i1");
      i2 = reader.ReadInt("i2");
      s1 = reader.ReadString("s1");
      s2 = reader.ReadString("s2");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("i1", i1);
      writer.MarkIdentityField("i1");
      writer.WriteInt("i2", i2);
      writer.WriteString("s1", s1);
      writer.MarkIdentityField("s1");
      writer.WriteString("s2", s2);
    }

    #endregion
  }
   public class SerializePdx1
   {
     [PdxIdentityField]
     public int i1;
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

       SerializePdx1 other = obj as SerializePdx1;

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
     [PdxIdentityField]
     public int i1;
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

       SerializePdx2 other = obj as SerializePdx2;

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
       BaseClass bc = obj as BaseClass;
       if (bc == null)
         return false;

       if (bc == this)
         return true;

       if (bc._baseclassmember == this._baseclassmember)
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

   public class Address
   {
     private static Guid oddGuid = new Guid("924243B5-9C2A-41d7-86B1-E0B905C7EED3");
     private static Guid evenGuid = new Guid("47AA8F17-FF6B-4a7d-B398-D83790977574");
     private string _street;
     private string _aptName;
     private int _flatNumber;
     private Guid _guid;
     public Address()
     { }
     public Address(int id)
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
       Address other = obj as Address;

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
     public void ToData(IPdxWriter w)
     {
       w.WriteString("_street", _street);
       w.WriteString("_aptName", _aptName);
       w.WriteInt("_flatNumber", _flatNumber);
       w.WriteString("_guid", _guid.ToString());
     }

     public void FromData(IPdxReader r)
     {
       _street = r.ReadString("_street");
       _aptName = r.ReadString("_aptName");
       _flatNumber = r.ReadInt("_flatNumber");
       string s = r.ReadString("_guid");
       _guid = new Guid(s);
     }
   }

   public class SerializePdx3 : BaseClass
   {
     private string s0;
     [PdxIdentityField]
     private int i1;
     public int i2;
     public string s1;
     public string s2;
     private SerializePdx2 nestedObject;
     private ArrayList _addressList;
     private Address _address;
     private Hashtable _hashTable;
     //private int arrayCountS3= 10;
     private List<object> _addressListObj;
     //private Address[] _arrayOfAddress;

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
         i1 = 1;
         i2 = 2;
         s1 = "s1";
         s2 = "s2";
         nestedObject = new SerializePdx2(true);

         _addressList = new ArrayList();
         _hashTable = new Hashtable();
         _addressListObj = new List<object>();

         for (int i = 0; i < 10; i++)
         {
           _addressList.Add(new Address(i));
           _hashTable.Add(i, new SerializePdx2(true));
           _addressListObj.Add(new Address(i));
         }

         _address = new Address(nAddress);

         //_arrayOfAddress = new Address[3];

         //for (int i = 0; i < 3; i++)
         //{
         //  _arrayOfAddress[i] = new Address(i);
         //}
       }
     }

     public override bool Equals(object obj)
     {
       if (obj == null)
         return false;
       if (obj == this)
         return true;

       SerializePdx3 other = obj as SerializePdx3;

       if (other == null)
         return false;

       if (s0 == other.s0
          && i1 == other.i1
          && i2 == other.i2
           && s1 == other.s1
            && s2 == other.s2)
       {
         bool ret = nestedObject.Equals(other.nestedObject);
         if (ret)
         {
           if (_addressList.Count == 10 &&
             _addressList.Count == other._addressList.Count//&&
             //_arrayOfAddress.Length == other._arrayOfAddress.Length &&
             //_arrayOfAddress[0].Equals(other._arrayOfAddress[0])
             )
           {
             for (int i = 0; i < _addressList.Count; i++)
             {
               ret = _addressList[i].Equals(other._addressList[i]);
               if (!ret)
                 return false;
             }

             if (_hashTable.Count != other._hashTable.Count)
               return false;
             foreach (DictionaryEntry de in _hashTable)
             {
               object otherHe = other._hashTable[de.Key];
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
       w.WriteInt("i1", i1);
       w.WriteInt("i2", i2);
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
       i1 = r.ReadInt("i1");
       i2 = r.ReadInt("i2");
       s1 = r.ReadString("s1");
       s2 = r.ReadString("s2");
       nestedObject = (SerializePdx2)r.ReadObject("nestedObject");
       _addressList = (ArrayList)r.ReadObject("_addressList");
       _address = (Address)r.ReadObject("_address");
       _hashTable = (Hashtable)r.ReadObject("_hashTable");
     }
   }

   public class SerializePdx4 : BaseClass
   {
     private string s0;
     [PdxIdentityField]
     private int i1;
     public int i2;
     public string s1;
     public string s2;
     private SerializePdx2 nestedObject;
     private ArrayList _addressList;
     private Address[] _addressArray;
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
         _addressArray = new Address[10];

         for (int i = 0; i < 10; i++)
         {
           _addressList.Add(new Address(i));
           _addressArray[i] = new Address(i);
         }
       }
     }

     public override bool Equals(object obj)
     {
       if (obj == null)
         return false;
       if (obj == this)
         return true;

       SerializePdx4 other = obj as SerializePdx4;

       if (other == null)
         return false;

       if (s0 == other.s0
          && i1 == other.i1
          && i2 == other.i2
           && s1 == other.s1
            && s2 == other.s2)
       {
         bool ret = nestedObject.Equals(other.nestedObject);
         if (ret)
         {
           if (_addressList.Count == other._addressList.Count &&
             _addressList[0].Equals(other._addressList[0]))
           {
             for (int i = 0; i < _addressList.Count; i++)
             {
               ret = _addressList[i].Equals(other._addressList[i]);
               if (!ret)
                 return false;
             }
             for (int i = 0; i < _addressArray.Length; i++)
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
     string _notInclude = "default_value";
     int _nameChange;
     int _identityField;

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

       PdxFieldTest other = obj as PdxFieldTest;

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

   public class PdxSerializer : IPdxSerializer
   {

     #region IPdxSerializer Members

     public object FromData(String className, IPdxReader reader)
     {
       object o = Activator.CreateInstance(Type.GetType(className));
       SerializePdx1 obj = o as SerializePdx1;

       if (obj != null)
       {
         obj.i1 = reader.ReadInt("i1");
         obj.i2 = reader.ReadInt("i2");
         obj.s1 = reader.ReadString("s1");
         obj.s2 = reader.ReadString("s2");
         return o;
       }
       else
       {
         SerializePdx2 obj2 = o as SerializePdx2;
         if (obj2 != null)
         {
           obj2.s0 = reader.ReadString("s0");
           obj2.i1 = reader.ReadInt("i1");
           obj2.i2 = reader.ReadInt("i2");
           obj2.s1 = reader.ReadString("s1");
           obj2.s2 = reader.ReadString("s2");
           return o;
         }
         else
         {
           SerializePdx3 sp3 = o as SerializePdx3;

           if (sp3 != null)
           {
             sp3.FromData(reader);
             return o;
           }
           else
           {
             Address ad = o as Address;
             if (ad != null)
             {
               ad.FromData(reader);
               return o;
             }
           }
           return null;
         }
       }
     }

     public bool ToData(object o, IPdxWriter writer)
     {
       SerializePdx1 obj = o as SerializePdx1;

       if (obj != null)
       {
         writer.WriteInt("i1", obj.i1);
         writer.WriteInt("i2", obj.i2);
         writer.WriteString("s1", obj.s1);
         writer.WriteString("s2", obj.s2);
         return true;
       }
       else
       {
         SerializePdx2 obj2 = o as SerializePdx2;
         if (obj2 != null)
         {
           writer.WriteString("s0", obj2.s0);
           writer.WriteInt("i1", obj2.i1);
           writer.WriteInt("i2", obj2.i2);
           writer.WriteString("s1", obj2.s1);
           writer.WriteString("s2", obj2.s2);
           return true;
         }
         else
         {
           SerializePdx3 sp3 = o as SerializePdx3;

           if (sp3 != null)
           {
             sp3.ToData(writer);
             return true;
           }
           else
           {
             Address ad = o as Address;
             if (ad != null)
             {
               ad.ToData(writer);
               return true;
             }
           }
         }
         return false;
       }
     }

     #endregion
   }

  public class SerializePdxNoRegister :IPdxSerializable
  {
    public int i1;
    public int i2;
    public string s1;
    public string s2;

    /*public static SerializePdx1 CreateDeserializable()
    {
      return new SerializePdx1(false);
    }*/

    public SerializePdxNoRegister()
    {
    }
    public SerializePdxNoRegister(bool init)
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

      SerializePdxNoRegister other = obj as SerializePdxNoRegister;

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

    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      i1 = reader.ReadInt("i1");
      i2 = reader.ReadInt("i2");
      s1 = reader.ReadString("s1");
      s2 = reader.ReadString("s2");
    }

    public void ToData(IPdxWriter writer)
    {
       writer.WriteInt("i1" , i1);
       writer.WriteInt("i2", i2);
       writer.WriteString("s1" ,s1);
       writer.WriteString("s2", s2);
    }

    #endregion


  }
   #endregion

  #region Extension of ReflectionBasedAutoSerializer

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

      return fi.Name ;
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
        Guid g = new Guid((string)serializeValue);

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

  #endregion

  #region Classes for per test and Dataoutput.Advance method
  class MyClasses : IPdxSerializable
  {
    [PdxIdentityField]
    public string Key;
    public List<Object> Children;

    public MyClasses()
    {
    }

    public MyClasses(string key, int nClasses)
    {
      Key = key;
      Children = new List<object>(nClasses);
      for (int i = 0; i < nClasses; i++)
      {
        MyClass my = new MyClass(i);
        Children.Add(my);
      }
    }

    public static IPdxSerializable Create()
    {
      return new MyClasses();
    }

    public override bool Equals(object obj)
    {
      if (obj == null)
        return false;

      MyClasses other = obj as MyClasses;
      if (other == null)
        return false;

      if (Children.Count == other.Children.Count)
        return true;
      return false;
    }
    public override int GetHashCode()
    {
        return base.GetHashCode();
    }
    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      Key = reader.ReadString("Key");
      Children = (List<Object>)(reader.ReadObject("Children"));
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteString("Key", Key);
      writer.WriteObject("Children", Children);
    }

    #endregion
  }

  class MyClass : IPdxSerializable
  {
    [PdxIdentityField]
    public string Key;
    public int SecKey;
    public double ShareQuantity;
    public double Cost;
    public double Price;
    public int SettleSecKey;
    public double SettleFxRate;
    public double ValueBasis;
    public double OpenDate;
    public double Strategy;

    public MyClass() { }

    public MyClass(int key)
    {
      Key = key.ToString();
      SecKey = key;
      ShareQuantity = key * 9278;
      Cost = ShareQuantity * 100;
      Price = Cost * 10;
      SettleSecKey = SecKey + 100000;
      SettleFxRate = Price * 1.5;
      ValueBasis = 1.5;
      OpenDate = 100000;
      Strategy = 3.6;
    }

    public static IPdxSerializable Create()
    {
      return new MyClass();
    }
    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      Key = reader.ReadString("Key");
      SecKey = reader.ReadInt("SecKey");
      ShareQuantity = reader.ReadDouble("ShareQuantity");
      Cost = reader.ReadDouble("Cost");
      Price = reader.ReadDouble("Price");
      SettleSecKey = reader.ReadInt("SettleSecKey");
      SettleFxRate = reader.ReadDouble("SettleFxRate");
      ValueBasis = reader.ReadDouble("ValueBasis");
      OpenDate = reader.ReadDouble("OpenDate");
      Strategy = reader.ReadDouble("Strategy");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteString("Key", Key);
      writer.WriteInt("SecKey", SecKey);
      writer.WriteDouble("ShareQuantity", ShareQuantity);
      writer.WriteDouble("Cost", Cost);
      writer.WriteDouble("Price", Price);
      writer.WriteInt("SettleSecKey", SettleSecKey);
      writer.WriteDouble("SettleFxRate", SettleFxRate);
      writer.WriteDouble("ValueBasis", ValueBasis);
      writer.WriteDouble("OpenDate", OpenDate);
      writer.WriteDouble("Strategy", Strategy);
    }

    #endregion
  }
  #endregion

  #region PdxTypeMapper

  public class PdxTypeMapper : IPdxTypeMapper
  { 
    public string ToPdxTypeName(string localTypeName)
    {
      return "my" + localTypeName;
    }

    public string FromPdxTypeName(string pdxTypeName)
    {
      return pdxTypeName.Substring(2);//need to extract "my"
    }
  }
  #endregion

  #region Pdx nested class
  public enum Gender { male, female, other};
  public class ChildPdx : IPdxSerializable
  {
    public int _childId;
    public Gender _gender;
    public string _childName;

    public ChildPdx() { }
    public ChildPdx(int id)
    {
      _childId = id;
      _childName = "name" + id.ToString();
      if (id % 2 == 0)
        _gender = Gender.female;
      else
        _gender = Gender.male;
    }
    public override string ToString()
    {
      return _childId + ":" + _childName + ":" + _gender;
    }
    public override bool Equals(object obj)
    {
      if (obj == null)
        return false;
      ChildPdx other = obj as ChildPdx;
      if (other == null)
        return false;
      if (_childName == other._childName
          && _gender == other._gender
            && _childId == other._childId)
        return true;
      return false;
    }
    public override int GetHashCode()
    {
        return base.GetHashCode();
    }
    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      _childId = reader.ReadInt("_childId");      
      _gender = (Gender)reader.ReadObject("_gender");
      _childName = reader.ReadString("_childName");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("_childId", _childId);
      writer.MarkIdentityField("_childId");
      writer.WriteObject("_gender", _gender);
      writer.WriteString("_childName", _childName);
    }

    #endregion
  }
  
  public class ParentPdx : IPdxSerializable
  {
    public int _parentId;
    public Gender _gender;
    public string _parentName;
    public ChildPdx _childPdx;

    public ParentPdx() { }
    public ParentPdx(int id)
    {
      _parentId = id;
      _parentName = "name" + id.ToString();
      if (id % 2 == 0)
        _gender = Gender.female;
      else
        _gender = Gender.male;
      _childPdx = new ChildPdx(id * 1393);
    }
    public override string ToString()
    {
      return _parentId + ":" + _gender + ":" + _parentName+ ":" + _childPdx;
    }
    public override bool Equals(object obj)
    {
      if (obj == null)
        return false;
      ParentPdx other = obj as ParentPdx;
      if (other == null)
        return false;
      if (_parentId == other._parentId
          && _gender == other._gender
            && _parentName == other._parentName
              && _childPdx.Equals(other._childPdx))
        return true;
      return  false;
    }
    public override int GetHashCode()
    {
        return base.GetHashCode();
    }
    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      _parentId = reader.ReadInt("_parentId");
      _gender = (Gender)reader.ReadObject("_gender");
      _parentName = reader.ReadString("_parentName");
      _childPdx = (ChildPdx)reader.ReadObject("_childPdx");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("_parentId", _parentId);
      writer.MarkIdentityField("_parentId");
      writer.WriteObject("_gender", _gender);
      writer.WriteString("_parentName", _parentName);
      writer.WriteObject("_childPdx", _childPdx);
      writer.MarkIdentityField("_childPdx");
    }

    #endregion
  }

  public class ChildPdxAS 
  {
    [PdxIdentityField]
    private int _childId;
    private Gender _gender;
    private string _childName;
     public ChildPdxAS() { }
    public ChildPdxAS(int id)
    {
      _childId = id;
      _childName = "name" + id.ToString();
      if (id % 2 == 0)
        _gender = Gender.female;
      else
        _gender = Gender.male;
    }

    public override string ToString()
    {
      return _childId + ":" + _childName + ":" + _gender;
    }
    
    public override bool Equals(object obj)
    {
      if (obj == null)
        return false;
      ChildPdxAS other = obj as ChildPdxAS;
      if (other == null)
        return false;
      if (_childName == other._childName
          && _gender == other._gender
            && _childId == other._childId)
        return true;
      return false;
    }
    public override int GetHashCode()
    {
        return base.GetHashCode();
    }
  }

  public class ParentPdxAS 
  {
    [PdxIdentityField]
    private int _parentId;
    private string _parentName;
    private Gender _gender;
    [PdxIdentityField]
    private ChildPdxAS _childPdx;
     public ParentPdxAS() { }
    public ParentPdxAS(int id)
    {
      _parentId = id;
      _parentName = "name" + id.ToString();
      if (id % 2 == 0)
        _gender = Gender.female;
      else
        _gender = Gender.male;
      _childPdx = new ChildPdxAS(id * 1393);
    }

    public override string ToString()
    {
      return _parentId + ":" + _gender + ":" + _parentName + ":" + _childPdx;
    }

    public override bool Equals(object obj)
    {
      if (obj == null)
        return false;
      ParentPdxAS other = obj as ParentPdxAS;
      if (other == null)
        return false;
      if (_parentId == other._parentId
          && _gender == other._gender
            && _parentName == other._parentName
              && _childPdx.Equals(other._childPdx))
        return true;
      return false;
    }
    public override int GetHashCode()
    {
        return base.GetHashCode();
    }
  }

  public class Heaptest : IPdxSerializable
  {
    int _id;
    byte[] _data;

    public Heaptest() { }

    public Heaptest(int id)
    {
      _id = id;
      _data = new byte[id];
    }


    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      _id = reader.ReadInt("_id");
      _data = reader.ReadByteArray("_data");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("_id", _id);
      writer.WriteByteArray("_data", _data);
    }

    #endregion
  }

  public enum enumQuerytest { id1, id2, id3};

  public class PdxEnumTestClass :IPdxSerializable
  {
    int _id;
    enumQuerytest _enumid;
    public int ID
    {
      get { return _id; }
    }
    public PdxEnumTestClass(int id)
    {
      _id = id;
      switch (id)
      { 
        case 0:
          _enumid = enumQuerytest.id1;
          break;
        case 1:
          _enumid = enumQuerytest.id2;
          break;
        case 2:
          _enumid = enumQuerytest.id3;
          break;
        default:
           _enumid = enumQuerytest.id1;
           break;
      }
    }

    public PdxEnumTestClass() { }

    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      _id = reader.ReadInt("_id");
      _enumid = (enumQuerytest)reader.ReadObject("_enumid");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("_id", _id);
      writer.WriteObject("_enumid", _enumid);
    }

    #endregion
  }

  #endregion
 
}

namespace javaobject
{
  using Apache.Geode.Client;
  #region Pdx Delta class
  public class PdxDelta : IPdxSerializable, IGeodeDelta, ICloneable
  {
    public static int GotDelta = 0;
    int _delta = 0;
    int _id;

    public PdxDelta() { }
    public PdxDelta(int id)
    {
      _id = id;
    }

    #region IPdxSerializable Members

    public void FromData(IPdxReader reader)
    {
      _id = reader.ReadInt("_id");
      _delta = reader.ReadInt("_delta");
    }

    public void ToData(IPdxWriter writer)
    {
      writer.WriteInt("_id", _id);
      writer.WriteInt("_delta", _delta);
    }

    #endregion
    public int Delta
    {
      get { return _delta; }
    }
    #region IGeodeDelta Members

    public void FromDelta(DataInput input)
    {
      Console.WriteLine(" in fromdelta " + GotDelta);
      _delta = input.ReadInt32();
      GotDelta++;
    }

    public bool HasDelta()
    {
      Console.WriteLine(" in hasdelta " + _delta);
      if (_delta > 0)
      {
        _delta++;
        return true;
      }
      else
      {
        _delta++;
        return false;
      }
    }

    public void ToDelta(DataOutput output)
    {
      Console.WriteLine(" in todelta " + GotDelta);
      output.WriteInt32(_delta);
      GotDelta++;
    }

    #endregion

    #region ICloneable Members

    public object Clone()
    {
      PdxDelta pd  = new PdxDelta();
      pd._id = _id;
      pd._delta = _delta;
      return pd;
    }

    #endregion
  }
  #endregion
}
