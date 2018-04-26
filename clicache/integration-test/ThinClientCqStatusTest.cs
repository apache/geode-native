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
using System.Threading;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  using Apache.Geode.Client.Tests;
  using Apache.Geode.Client;

  [TestFixture]
  [Category("group3")]
  [Category("unicast_only")]
  [Category("generics")]

  public class ThinClientCqStatusTest : ThinClientRegionSteps
  {
    public class MyCqListener<TKey, TResult> : ICqListener<TKey, TResult>
    {
      #region Private members
      private bool m_failedOver = false;
      private UInt32 m_eventCountBefore = 0;
      private UInt32 m_errorCountBefore = 0;
      private UInt32 m_eventCountAfter = 0;
      private UInt32 m_errorCountAfter = 0;

      #endregion

      #region Public accessors

      public void failedOver()
      {
        m_failedOver = true;
      }
      public UInt32 getEventCountBefore()
      {
        return m_eventCountBefore;
      }
      public UInt32 getErrorCountBefore()
      {
        return m_errorCountBefore;
      }
      public UInt32 getEventCountAfter()
      {
        return m_eventCountAfter;
      }
      public UInt32 getErrorCountAfter()
      {
        return m_errorCountAfter;
      }
      #endregion

      public virtual void OnEvent(CqEvent<TKey, TResult> ev)
      {
        Util.Log("MyCqListener::OnEvent called");
        if (m_failedOver == true)
          m_eventCountAfter++;
        else
          m_eventCountBefore++;

        //IGeodeSerializable val = ev.getNewValue();
        //ICacheableKey key = ev.getKey();

        TResult val = (TResult)ev.getNewValue();
        /*ICacheableKey*/
        TKey key = ev.getKey();

        CqOperation opType = ev.getQueryOperation();
        //CacheableString keyS = key as CacheableString;
        string keyS = key.ToString(); //as string;
        Portfolio pval = val as Portfolio;
        PortfolioPdx pPdxVal = val as PortfolioPdx;
        Assert.IsTrue((pPdxVal != null) || (pval != null));
        //string opStr = "DESTROY";
        /*if (opType == CqOperation.OP_TYPE_CREATE)
          opStr = "CREATE";
        else if (opType == CqOperation.OP_TYPE_UPDATE)
          opStr = "UPDATE";*/

        //Util.Log("key {0}, value ({1},{2}), op {3}.", keyS,
        //  pval.ID, pval.Pkid, opStr);
      }
      public virtual void OnError(CqEvent<TKey, TResult> ev)
      {
        Util.Log("MyCqListener::OnError called");
        if (m_failedOver == true)
          m_errorCountAfter++;
        else
          m_errorCountBefore++;
      }
      public virtual void Close()
      {
        Util.Log("MyCqListener::close called");
      }
      public virtual void Clear()
      {
        Util.Log("MyCqListener::Clear called");
        m_eventCountBefore = 0;
        m_errorCountBefore = 0;
        m_eventCountAfter = 0;
        m_errorCountAfter = 0;
      }
    }

    public class MyCqListener1<TKey, TResult> : ICqListener<TKey, TResult>
    {
      public static UInt32 m_cntEvents = 0;

      public virtual void OnEvent(CqEvent<TKey, TResult> ev)
      {
        m_cntEvents++;
        Util.Log("MyCqListener1::OnEvent called");
        Object val = (Object)ev.getNewValue();
        Object pkey = (Object)ev.getKey();
        int value = (int)val;
        int key = (int)pkey;
        CqOperation opType = ev.getQueryOperation();
        String opStr = "Default";
        if (opType == CqOperation.OP_TYPE_CREATE)
          opStr = "CREATE";
        else if (opType == CqOperation.OP_TYPE_UPDATE)
          opStr = "UPDATE";

        Util.Log("MyCqListener1::OnEvent called with {0} , key = {1}, value = {2} ",
        opStr, key, value);
      }
      public virtual void OnError(CqEvent<TKey, TResult> ev)
      {
        Util.Log("MyCqListener1::OnError called");
      }
      public virtual void Close()
      {
        Util.Log("MyCqListener1::close called");
      }
    }

    public class MyCqStatusListener<TKey, TResult> : ICqStatusListener<TKey, TResult>
    {
      #region Private members
      private bool m_failedOver = false;
      private UInt32 m_eventCountBefore = 0;
      private UInt32 m_errorCountBefore = 0;
      private UInt32 m_eventCountAfter = 0;
      private UInt32 m_errorCountAfter = 0;
      private UInt32 m_CqConnectedCount = 0;
      private UInt32 m_CqDisConnectedCount = 0;

      #endregion

      #region Public accessors

      public MyCqStatusListener(int id)
      {
      }

      public void failedOver()
      {
        m_failedOver = true;
      }
      public UInt32 getEventCountBefore()
      {
        return m_eventCountBefore;
      }
      public UInt32 getErrorCountBefore()
      {
        return m_errorCountBefore;
      }
      public UInt32 getEventCountAfter()
      {
        return m_eventCountAfter;
      }
      public UInt32 getErrorCountAfter()
      {
        return m_errorCountAfter;
      }
      public UInt32 getCqConnectedCount()
      {
        return m_CqConnectedCount;
      }
      public UInt32 getCqDisConnectedCount()
      {
        return m_CqDisConnectedCount;
      }
      #endregion

      public virtual void OnEvent(CqEvent<TKey, TResult> ev)
      {
        Util.Log("MyCqStatusListener::OnEvent called");
        if (m_failedOver == true)
          m_eventCountAfter++;
        else
          m_eventCountBefore++;

        TResult val = (TResult)ev.getNewValue();
        TKey key = ev.getKey();

        CqOperation opType = ev.getQueryOperation();
        string keyS = key.ToString(); //as string;      
      }
      public virtual void OnError(CqEvent<TKey, TResult> ev)
      {
        Util.Log("MyCqStatusListener::OnError called");
        if (m_failedOver == true)
          m_errorCountAfter++;
        else
          m_errorCountBefore++;
      }
      public virtual void Close()
      {
        Util.Log("MyCqStatusListener::close called");
      }
      public virtual void OnCqConnected()
      {
        m_CqConnectedCount++;
        Util.Log("MyCqStatusListener::OnCqConnected called");
      }
      public virtual void OnCqDisconnected()
      {
        m_CqDisConnectedCount++;
        Util.Log("MyCqStatusListener::OnCqDisconnected called");
      }

      public virtual void Clear()
      {
        Util.Log("MyCqStatusListener::Clear called");
        m_eventCountBefore = 0;
        m_errorCountBefore = 0;
        m_eventCountAfter = 0;
        m_errorCountAfter = 0;
        m_CqConnectedCount = 0;
        m_CqDisConnectedCount = 0;
      }
    }


    #region Private members
    private static bool m_usePdxObjects = false;
    private UnitProcess m_client1;
    private UnitProcess m_client2;
    private static string[] QueryRegionNames = { "Portfolios", "Positions", "Portfolios2",
      "Portfolios3" };
    private static string QERegionName = "Portfolios";
    private static string CqName = "MyCq";

    private static string CqName1 = "testCQAllServersLeave";
    private static string CqName2 = "testCQAllServersLeave1";

    private static string CqQuery1 = "select * from /DistRegionAck";
    private static string CqQuery2 = "select * from /DistRegionAck1";
    //private static string CqName1 = "MyCq1";

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      return new ClientBase[] { m_client1, m_client2 };
    }

    [TestFixtureSetUp]
    public override void InitTests()
    {
      base.InitTests();
      m_client1.Call(InitClient);
      m_client2.Call(InitClient);
    }

    [TearDown]
    public override void EndTest()
    {
      CacheHelper.StopJavaServers();
      base.EndTest();
    }


    public void InitClient()
    {
      CacheHelper.Init();
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(Portfolio.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterTypeGeneric(Position.CreateDeserializable);
      }
      catch (IllegalStateException)
      {
        // ignore since we run multiple iterations for pool and non pool configs
      }
    }

    public void StepOne(string locators)
    {
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[0], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[1], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[2], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[3], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>("DistRegionAck", true, true,
        null, locators, "__TESTPOOL1_", true);
      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);
      Apache.Geode.Client.RegionAttributes<object, object> regattrs = region.Attributes;
      region.CreateSubRegion(QueryRegionNames[1], regattrs);
    }

    public void CreateAndExecuteCQ_StatusListener(string poolName, string cqName, string cqQuery, int id)
    {
      var qs = CacheHelper.DCache.GetPoolManager().Find(poolName).GetQueryService();
      CqAttributesFactory<object, object> cqFac = new CqAttributesFactory<object, object>();
      cqFac.AddCqListener(new MyCqStatusListener<object, object>(id));
      CqAttributes<object, object> cqAttr = cqFac.Create();
      CqQuery<object, object> qry = qs.NewCq(cqName, cqQuery, cqAttr, false);
      qry.Execute();
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete
    }

    public void CreateAndExecuteCQ_Listener(string poolName, string cqName, string cqQuery, int id)
    {
      var qs = CacheHelper.DCache.GetPoolManager().Find(poolName).GetQueryService();
      CqAttributesFactory<object, object> cqFac = new CqAttributesFactory<object, object>();
      cqFac.AddCqListener(new MyCqListener<object, object>(/*id*/));
      CqAttributes<object, object> cqAttr = cqFac.Create();
      CqQuery<object, object> qry = qs.NewCq(cqName, cqQuery, cqAttr, false);
      qry.Execute();
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete
    }

    public void CheckCQStatusOnConnect(string poolName, string cqName, int onCqStatusConnect)
    {      
      var qs = CacheHelper.DCache.GetPoolManager().Find(poolName).GetQueryService();
      CqQuery<object, object> query = qs.GetCq<object, object>(cqName);
      CqAttributes<object, object> cqAttr = query.GetCqAttributes();
      ICqListener<object, object>[] vl = cqAttr.getCqListeners();
      MyCqStatusListener<object, object> myCqStatusLstr = (MyCqStatusListener<object, object>) vl[0];
      Util.Log("CheckCQStatusOnConnect = {0} ", myCqStatusLstr.getCqConnectedCount());
      Assert.AreEqual(onCqStatusConnect, myCqStatusLstr.getCqConnectedCount());
    }

    public void CheckCQStatusOnDisConnect(string poolName, string cqName, int onCqStatusDisConnect)
    {
      var qs = CacheHelper.DCache.GetPoolManager().Find(poolName).GetQueryService();
      CqQuery<object, object> query = qs.GetCq<object, object>(cqName);
      CqAttributes<object, object> cqAttr = query.GetCqAttributes();
      ICqListener<object, object>[] vl = cqAttr.getCqListeners();
      MyCqStatusListener<object, object> myCqStatusLstr = (MyCqStatusListener<object, object>)vl[0];
      Util.Log("CheckCQStatusOnDisConnect = {0} ", myCqStatusLstr.getCqDisConnectedCount());
      Assert.AreEqual(onCqStatusDisConnect, myCqStatusLstr.getCqDisConnectedCount());
    }

    public void PutEntries(string regionName)
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(regionName);
      for (int i = 1; i <= 10; i++) {
        region["key-" + i] = "val-" + i;
      }
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete
    }

    public void CheckCQStatusOnPutEvent(string poolName, string cqName, int onCreateCount)
    {
      var qs = CacheHelper.DCache.GetPoolManager().Find(poolName).GetQueryService();
      CqQuery<object, object> query = qs.GetCq<object, object>(cqName);
      CqAttributes<object, object> cqAttr = query.GetCqAttributes();
      ICqListener<object, object>[] vl = cqAttr.getCqListeners();
      MyCqStatusListener<object, object> myCqStatusLstr = (MyCqStatusListener<object, object>)vl[0];
      Util.Log("CheckCQStatusOnPutEvent = {0} ", myCqStatusLstr.getEventCountBefore());
      Assert.AreEqual(onCreateCount, myCqStatusLstr.getEventCountBefore());
    }

    void runCqQueryStatusTest()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(StepOne, CacheHelper.Locators);
      Util.Log("StepOne complete.");

      m_client1.Call(CreateAndExecuteCQ_StatusListener, "__TESTPOOL1_", CqName1, CqQuery1, 100);
      Util.Log("CreateAndExecuteCQ complete.");

      m_client1.Call(CheckCQStatusOnConnect, "__TESTPOOL1_", CqName1, 1);
      Util.Log("CheckCQStatusOnConnect complete.");

      m_client1.Call(PutEntries, "DistRegionAck");
      Util.Log("PutEntries complete.");

      m_client1.Call(CheckCQStatusOnPutEvent, "__TESTPOOL1_", CqName1, 10);
      Util.Log("CheckCQStatusOnPutEvent complete.");

      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("start server 2 complete.");

      Thread.Sleep(20000);
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      Thread.Sleep(20000);
      m_client1.Call(CheckCQStatusOnDisConnect, "__TESTPOOL1_", CqName1, 0);
      Util.Log("CheckCQStatusOnDisConnect complete.");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");
      Thread.Sleep(20000);
      m_client1.Call(CheckCQStatusOnDisConnect, "__TESTPOOL1_", CqName1, 1);
      Util.Log("CheckCQStatusOnDisConnect complete.");

      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      Thread.Sleep(20000);

      m_client1.Call(CheckCQStatusOnConnect, "__TESTPOOL1_", CqName1, 2);
      Util.Log("CheckCQStatusOnConnect complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      Thread.Sleep(20000);

      m_client1.Call(CheckCQStatusOnDisConnect, "__TESTPOOL1_", CqName1, 2);
      Util.Log("CheckCQStatusOnDisConnect complete.");

      m_client1.Call(Close);

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }

    [Test]
    public void CqQueryStatusTest()
    {
      runCqQueryStatusTest();
    }
  }
}
