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
using System.IO;
using System.Threading;

#pragma warning disable 618

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  using Apache.Geode.Client.Tests;
  using Apache.Geode.Client;
  using DeltaEx = Apache.Geode.Client.Tests.DeltaEx;

  public class CqDeltaListener<TKey, TResult> : ICqListener<TKey, TResult>
  {

    public CqDeltaListener()
    {
      m_deltaCount = 0;
      m_valueCount = 0;
    }

    public void OnEvent(CqEvent<TKey, TResult> aCqEvent)
    {
      byte[] deltaValue = aCqEvent.getDeltaValue();
      DeltaTestImpl newValue = new DeltaTestImpl();
      DataInput input = CacheHelper.DCache.CreateDataInput(deltaValue);
      newValue.FromDelta(input);
      if (newValue.GetIntVar() == 5)
      {
        m_deltaCount++;
      }
      DeltaTestImpl fullObject = (DeltaTestImpl)(object)aCqEvent.getNewValue();
      if (fullObject.GetIntVar() == 5)
      {
        m_valueCount++;
      }

    }

    public void OnError(CqEvent<TKey, TResult> aCqEvent)
    {
    }

    public void Close()
    {
    }

    public int GetDeltaCount()
    {
      return m_deltaCount;
    }

    public int GetValueCount()
    {
      return m_valueCount;
    }

    private int m_deltaCount;
    private int m_valueCount;
  }

  public class DeltaTestAD : IDelta, IDataSerializable
  {
    private int _deltaUpdate;
    private string _staticData;

    public static DeltaTestAD Create()
    {
      return new DeltaTestAD();
    }

    public DeltaTestAD()
    {
      _deltaUpdate = 1;
      _staticData = "Data which don't get updated";
    }


    #region IDelta Members

    public void FromDelta(DataInput input)
    {
      _deltaUpdate = input.ReadInt32();
    }

    public bool HasDelta()
    {
      _deltaUpdate++;
      bool isDelta = (_deltaUpdate % 2) == 1;
      Util.Log("In DeltaTestAD.HasDelta _deltaUpdate:" + _deltaUpdate + " : isDelta:" + isDelta);
      return isDelta;
    }

    public void ToDelta(DataOutput output)
    {
      output.WriteInt32(_deltaUpdate);
    }

    #endregion

    #region IDataSerializable Members

    public void FromData(DataInput input)
    {
      _deltaUpdate = input.ReadInt32();
      _staticData = input.ReadUTF();
    }

    public UInt64 ObjectSize
    {
      get { return (uint)(4 + _staticData.Length); }
    }

    public void ToData(DataOutput output)
    {
      output.WriteInt32(_deltaUpdate);
      output.WriteUTF(_staticData);
    }

    public int DeltaUpdate
    {
      get { return _deltaUpdate; }
      set { _deltaUpdate = value; }
    }

    #endregion
  }

  [TestFixture]
  [Category("group1")]
  [Category("unicast_only")]
  [Category("generics")]
  public class ThinClientDeltaTest : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1, m_client2;
    private CqDeltaListener<object, DeltaTestImpl> myCqListener;

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      return new ClientBase[] { m_client1, m_client2 };
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

    public void createRegionAndAttachPool(string regionName, string poolName)
    {
      createRegionAndAttachPool(regionName, poolName, false);
    }

    public void createRegionAndAttachPool(string regionName, string poolName, bool cloningEnabled)
    {
      CacheHelper.CreateTCRegion_Pool<object, object>(regionName, true, true, null, null, poolName, false,
        false, cloningEnabled);
    }

    public void createPool(string name, string locators, string serverGroup,
      int redundancy, bool subscription)
    {
      CacheHelper.CreatePool<object, object>(name, locators, serverGroup, redundancy, subscription);
    }

    public void createExpirationRegion(string name, string poolName)
    {
      IRegion<object, object> region = CacheHelper.CreateExpirationRegion<object, object>(name,
          poolName, ExpirationAction.LocalInvalidate, TimeSpan.FromSeconds(5));
    }

    void DoPutWithDelta()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaEx.create, 1);
      }
      catch (IllegalStateException)
      {
        //do nothng
      }
      string cKey = m_keys[0];
      DeltaEx val = new DeltaEx();
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");

      reg[cKey] = (object)val;
      val.SetDelta(true);
      reg[cKey] = (object)val;

      DeltaEx val1 = new DeltaEx(0); // In this case JAVA side will throw invalid DeltaException
      reg[cKey] = (object)val1;
      val1.SetDelta(true);
      reg[cKey] = (object)val1;
      if (DeltaEx.ToDeltaCount != 2)
      {
        Util.Log("DeltaEx.ToDataCount = " + DeltaEx.ToDataCount);
        Assert.Fail(" Delta count should have been 2, is " + DeltaEx.ToDeltaCount);
      }
      if (DeltaEx.ToDataCount != 3)
        Assert.Fail("Data count should have been 3, is " + DeltaEx.ToDataCount);
      DeltaEx.ToDeltaCount = 0;
      DeltaEx.ToDataCount = 0;
      DeltaEx.FromDataCount = 0;
      DeltaEx.FromDeltaCount = 0;
    }

    void Do_Put_Contains_Remove_WithDelta()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaEx.create, 1);
      }
      catch (IllegalStateException)
      {
        //do nothng
      }
      string cKey = m_keys[0];
      DeltaEx val = new DeltaEx();
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");

      reg[cKey] = (object)val;
      val.SetDelta(true);
      reg[cKey] = (object)val;

      DeltaEx val1 = new DeltaEx(0); // In this case JAVA side will throw invalid DeltaException
      reg[cKey] = (object)val1;
      val1.SetDelta(true);
      reg[cKey] = (object)val1;
      if (DeltaEx.ToDeltaCount != 2)
      {
        Util.Log("DeltaEx.ToDataCount = " + DeltaEx.ToDataCount);
        Assert.Fail(" Delta count should have been 2, is " + DeltaEx.ToDeltaCount);
      }
      if (DeltaEx.ToDataCount != 3)
        Assert.Fail("Data count should have been 3, is " + DeltaEx.ToDataCount);
      DeltaEx.ToDeltaCount = 0;
      DeltaEx.ToDataCount = 0;
      DeltaEx.FromDataCount = 0;
      DeltaEx.FromDeltaCount = 0;

      // Try Contains with key & value that are present. Result should be true.
      KeyValuePair<object, object> myentry = new KeyValuePair<object, object>(cKey, val1);
      bool containsOpflag = reg.Contains(myentry);
      Assert.IsTrue(containsOpflag, "Result should be true as key & value are present");

      // Try Remove with key & value that are present. Result should be true.
      bool removeOpflag = reg.Remove(cKey);
      Assert.IsTrue(removeOpflag, "Result should be true as key & value are present");

      //Check Contains with removed entry. Result should be false.
      bool updatedcontainsOpflag = reg.Contains(myentry);
      Assert.IsFalse(updatedcontainsOpflag, "Result should be false as key & value are removed");
    }

    void DoExpirationWithDelta()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaEx.create, 1);
      }
      catch (IllegalStateException)
      {
        //do nothig.
      }

      DeltaEx val1 = new DeltaEx();
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");
      reg[1] = val1;
      // Sleep 10 seconds to allow expiration of entry in client 2
      Thread.Sleep(10000);
      val1.SetDelta(true);
      reg[1] = val1;
      DeltaEx.ToDeltaCount = 0;
      DeltaEx.ToDataCount = 0;
    }

    void DoCqWithDelta()
    {
      string cKey1 = "key1";
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");
      DeltaTestImpl value = new DeltaTestImpl();
      reg[cKey1] = value;
      value.SetIntVar(5);
      value.SetDelta(true);
      reg[cKey1] = value;
    }

    void initializeDeltaClientAD()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaTestAD.Create, 151);
      }
      catch (IllegalStateException)
      {
        //do nothng
      }
    }

    void DoDeltaAD_C1_1()
    {
      DeltaTestAD val = new DeltaTestAD();
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");
      reg.GetSubscriptionService().RegisterAllKeys();
      Util.Log("clientAD1 put");
      reg[1] = val;
      Util.Log("clientAD1 put done");
    }

    void DoDeltaAD_C2_1()
    {
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");

      Util.Log("clientAD2 get");
      DeltaTestAD val = (DeltaTestAD)reg[1];

      Assert.AreEqual(2, val.DeltaUpdate);
      Util.Log("clientAD2 get done");
      reg[1] = val;
      Util.Log("clientAD2 put done");

      javaobject.PdxDelta pd = new javaobject.PdxDelta(1001);
      for (int i = 0; i < 10; i++)
      {
        reg["pdxdelta"] = pd;
      }
    }

    void DoDeltaAD_C1_afterC2Put()
    {
      Thread.Sleep(15000);
      DeltaTestAD val = null;
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");
      Util.Log("client fetching entry from local cache");
      val = (DeltaTestAD)reg.GetEntry(1).Value;
      Assert.IsNotNull(val);
      Assert.AreEqual(3, val.DeltaUpdate);
      Util.Log("done");

      System.Threading.Thread.Sleep(5000);
      //Assert.Greater(javaobject.PdxDelta.GotDelta, 7, "this should have recieve delta");
      javaobject.PdxDelta pd = (javaobject.PdxDelta)(reg.GetLocalView()["pdxdelta"]);
      Assert.Greater(pd.Delta, 7, "this should have recieve delta");
    }

    void runDeltaWithAppdomian(bool cloningenable)
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_with_deltaAD.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC1");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS5", 1);
      string regionName = "DistRegionAck";
     // if (usePools)
      {
        //CacheHelper.CreateTCRegion_Pool_AD("DistRegionAck", false, false, null, null, CacheHelper.Locators, "__TEST_POOL1__", false, false, false);
        m_client1.Call(CacheHelper.CreateTCRegion_Pool_AD1, regionName, false, true, CacheHelper.Locators, (string)"__TEST_POOL1__", true, cloningenable);
        m_client2.Call(CacheHelper.CreateTCRegion_Pool_AD1, regionName, false, true, CacheHelper.Locators, (string)"__TEST_POOL1__", false, cloningenable);

        // m_client1.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, false);
        // m_client1.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");
      }


      m_client1.Call(initializeDeltaClientAD);
      m_client2.Call(initializeDeltaClientAD);

      m_client1.Call(DoDeltaAD_C1_1);
      m_client2.Call(DoDeltaAD_C2_1);
      m_client1.Call(DoDeltaAD_C1_afterC2Put);
      m_client1.Call(Close);
      m_client2.Call(Close);

      CacheHelper.StopJavaServer(1);
      CacheHelper.StopJavaLocator(1);
      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    void runPutWithDelta()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_with_delta.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC1");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS5", 1);
      m_client1.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, false);
      m_client1.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");

      m_client1.Call(DoPutWithDelta);
      m_client1.Call(Close);

      CacheHelper.StopJavaServer(1);
      CacheHelper.StopJavaLocator(1);
      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    void runPut_Contains_Remove_WithDelta()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_with_delta.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC1");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS5", 1);
      m_client1.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, false);
      m_client1.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");

      m_client1.Call(Do_Put_Contains_Remove_WithDelta);
      m_client1.Call(Close);

      CacheHelper.StopJavaServer(1);
      CacheHelper.StopJavaLocator(1);
      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    void registerClassCl2()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaEx.create, 1);
      }
      catch (IllegalStateException)
      {
        //do nothing
      }
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");

      reg.GetSubscriptionService().RegisterRegex(".*");
      AttributesMutator<object, object> attrMutator = reg.AttributesMutator;
      attrMutator.SetCacheListener(new SimpleCacheListener<object, object>());
    }

    void registerClassDeltaTestImpl()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(DeltaTestImpl.CreateDeserializable, 0x1E);
      }
      catch (IllegalStateException)
      {
        // ARB: ignore exception caused by type reregistration.
      }
      DeltaTestImpl.ResetDataCount();

      Thread.Sleep(2000);
      IRegion<object, object> reg = CacheHelper.GetRegion<object, object>("DistRegionAck");
      try
      {
        reg.GetSubscriptionService().RegisterRegex(".*");
      }
      catch (Exception)
      {
        // ARB: ignore regex exception for missing notification channel.
      }
    }

    void registerCq()
    {
      Pool thePool = CacheHelper.DCache.GetPoolManager().Find("__TEST_POOL1__");
      QueryService cqService = null;
      cqService = thePool.GetQueryService();
      CqAttributesFactory<object, DeltaTestImpl> attrFac = new CqAttributesFactory<object, DeltaTestImpl>();
      myCqListener = new CqDeltaListener<object, DeltaTestImpl>();
      attrFac.AddCqListener(myCqListener);
      CqAttributes<object, DeltaTestImpl> cqAttr = attrFac.Create();
      CqQuery<object, DeltaTestImpl> theQuery = cqService.NewCq("select * from /DistRegionAck d where d.intVar > 4", cqAttr, false);
      theQuery.Execute();
    }

    void VerifyCqDeltaCount()
    {
      // Wait for Cq event processing in listener
      Thread.Sleep(1000);
      if (myCqListener.GetDeltaCount() != 1)
      {
        Assert.Fail("Delta from CQ event does not have expected value");
      }
      if (myCqListener.GetValueCount() != 1)
      {
        Assert.Fail("Value from CQ event is incorrect");
      }
    }

    void VerifyExpirationDeltaCount()
    {
      Thread.Sleep(1000);
      if (DeltaEx.FromDataCount != 2)
        Assert.Fail("Count should have been 2.");
      if (DeltaEx.FromDeltaCount != 0)
        Assert.Fail("Count should have been 0.");
      DeltaEx.FromDataCount = 0;
      DeltaEx.FromDeltaCount = 0;
    }

    void runCqWithDelta()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_with_delta_test_impl.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC1");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS5", 1);

      m_client1.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, true);
      m_client1.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");

      m_client2.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, true);
      m_client2.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");

      m_client1.Call(registerClassDeltaTestImpl);
      m_client2.Call(registerClassDeltaTestImpl);
      m_client2.Call(registerCq);

      m_client1.Call(DoCqWithDelta);
      m_client2.Call(VerifyCqDeltaCount);
      m_client1.Call(Close);
      m_client2.Call(Close);

      CacheHelper.StopJavaServer(1);
      CacheHelper.StopJavaLocator(1);
      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    void runExpirationWithDelta()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_with_delta.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC1");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS5", 1);

      m_client1.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, true);
      m_client1.Call(createRegionAndAttachPool, "DistRegionAck", "__TEST_POOL1__");

      m_client2.Call(createPool, "__TEST_POOL1__", CacheHelper.Locators, (string)null, 0, true);
      m_client2.Call(createExpirationRegion, "DistRegionAck", "__TEST_POOL1__");

      m_client2.Call(registerClassCl2);

      m_client1.Call(DoExpirationWithDelta);
      m_client2.Call(VerifyExpirationDeltaCount);
      m_client1.Call(Close);
      m_client2.Call(Close);

      CacheHelper.StopJavaServer(1);
      CacheHelper.StopJavaLocator(1);
      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    //#region Tests

    [Test]
    public void PutWithDeltaAD()
    {
      runDeltaWithAppdomian(false);
    }

    [Test]
    public void PutWithDelta()
    {
      runPutWithDelta();
    }

    [Test]
    public void Put_Contains_Remove_WithDelta()
    {
      runPut_Contains_Remove_WithDelta();
    }

    [Test]
    public void CqWithDelta()
    {
      runCqWithDelta();
    }

    [Test]
    public void ExpirationWithDelta()
    {
      runExpirationWithDelta();
    }

    //#endregion
  }
}

