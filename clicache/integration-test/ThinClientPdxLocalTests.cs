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
  class ThinClientPdxLocalTests : ThinClientRegionSteps
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

     int nPdxPuts = 100000;
     int pdxobjsize = 5000;

     int nBAPuts = 25;
     int baSize = 16240000;

     string testSysPropFileName = "testLR.properties";

     #region Tests
     
     [Test]
     public void LocalOps()
     {
       runLocalOps();
     }

    #endregion
   }
}