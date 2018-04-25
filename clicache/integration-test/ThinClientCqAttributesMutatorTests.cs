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

  public class ThinClientCqAttributesMutatorTests : ThinClientRegionSteps
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

    public void ProcessCQ(string locators)
    {
      CacheHelper.CreateTCRegion_Pool<object, object>(QERegionName, true, true,
      null, locators, "__TESTPOOL1_", true);

      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(QERegionName);
      Portfolio p1 = new Portfolio(1, 100);
      Portfolio p2 = new Portfolio(2, 100);
      Portfolio p3 = new Portfolio(3, 100);
      Portfolio p4 = new Portfolio(4, 100);

      region["1"] = p1;
      region["2"] = p2;
      region["3"] = p3;
      region["4"] = p4;

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();
      
      CqAttributesFactory<object, object> cqFac = new CqAttributesFactory<object, object>();
      ICqListener<object, object> cqLstner = new MyCqListener<object, object>();
      ICqStatusListener<object, object> cqStatusLstner = new MyCqStatusListener<object, object>(1);

      ICqListener<object, object>[] v = new ICqListener<object, object>[2];
      cqFac.AddCqListener(cqLstner);
      v[0] = cqLstner;
      v[1] = cqStatusLstner;
      cqFac.InitCqListeners(v);
      Util.Log("InitCqListeners called");
      CqAttributes<object, object> cqAttr = cqFac.Create();
      CqQuery<object, object> qry1 = qs.NewCq("CQ1", "select * from /" + QERegionName + "  p where p.ID >= 1", cqAttr, false);
      qry1.Execute();

      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete
      region["4"] = p1;
      region["3"] = p2;
      region["2"] = p3;
      region["1"] = p4;
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete

      qry1 = qs.GetCq<object, object>("CQ1");
      cqAttr = qry1.GetCqAttributes();
      ICqListener<object, object>[] vl = cqAttr.getCqListeners();
      Assert.IsNotNull(vl);
      Assert.AreEqual(2, vl.Length);
      cqLstner = vl[0];
      Assert.IsNotNull(cqLstner);
      MyCqListener<object, object> myLisner = (MyCqListener<object, object>)cqLstner;// as MyCqListener<object, object>;
      Util.Log("event count:{0}, error count {1}.", myLisner.getEventCountBefore(), myLisner.getErrorCountBefore());
      Assert.AreEqual(4, myLisner.getEventCountBefore());

      cqStatusLstner = (ICqStatusListener<object, object>)vl[1];
      Assert.IsNotNull(cqStatusLstner);
      MyCqStatusListener<object, object> myStatLisner = (MyCqStatusListener<object, object>)cqStatusLstner;// as MyCqStatusListener<object, object>;
      Util.Log("event count:{0}, error count {1}.", myStatLisner.getEventCountBefore(), myStatLisner.getErrorCountBefore());
      Assert.AreEqual(1, myStatLisner.getCqConnectedCount());
      Assert.AreEqual(4, myStatLisner.getEventCountBefore());

      CqAttributesMutator<object, object> mutator = qry1.GetCqAttributesMutator();
      mutator.RemoveCqListener(cqLstner);
      cqAttr = qry1.GetCqAttributes();
      Util.Log("cqAttr.getCqListeners().Length = {0}", cqAttr.getCqListeners().Length);
      Assert.AreEqual(1, cqAttr.getCqListeners().Length);

      mutator.RemoveCqListener(cqStatusLstner);
      cqAttr = qry1.GetCqAttributes();
      Util.Log("1 cqAttr.getCqListeners().Length = {0}", cqAttr.getCqListeners().Length);
      Assert.AreEqual(0, cqAttr.getCqListeners().Length);
      
      ICqListener<object, object>[] v2 = new ICqListener<object, object>[2];
      v2[0] = cqLstner;
      v2[1] = cqStatusLstner;
      MyCqListener<object, object> myLisner2 = (MyCqListener<object, object>)cqLstner;
      myLisner2.Clear();
      MyCqStatusListener<object, object> myStatLisner2 = (MyCqStatusListener<object, object>)cqStatusLstner;
      myStatLisner2.Clear();
      mutator.SetCqListeners(v2);
      cqAttr = qry1.GetCqAttributes();
      Assert.AreEqual(2, cqAttr.getCqListeners().Length);

      region["4"] = p1;
      region["3"] = p2;
      region["2"] = p3;
      region["1"] = p4;
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete

      qry1 = qs.GetCq<object, object>("CQ1");
      cqAttr = qry1.GetCqAttributes();
      ICqListener<object, object>[] v3 = cqAttr.getCqListeners();
      Assert.IsNotNull(v3);
      Assert.AreEqual(2, vl.Length);
      cqLstner = v3[0];
      Assert.IsNotNull(cqLstner);
      myLisner2 = (MyCqListener<object, object>)cqLstner;// as MyCqListener<object, object>;
      Util.Log("event count:{0}, error count {1}.", myLisner2.getEventCountBefore(), myLisner2.getErrorCountBefore());
      Assert.AreEqual(4, myLisner2.getEventCountBefore());

      cqStatusLstner = (ICqStatusListener<object, object>)v3[1];
      Assert.IsNotNull(cqStatusLstner);
      myStatLisner2 = (MyCqStatusListener<object, object>)cqStatusLstner;// as MyCqStatusListener<object, object>;
      Util.Log("event count:{0}, error count {1}.", myStatLisner2.getEventCountBefore(), myStatLisner2.getErrorCountBefore());
      Assert.AreEqual(0, myStatLisner2.getCqConnectedCount());
      Assert.AreEqual(4, myStatLisner2.getEventCountBefore());

      mutator = qry1.GetCqAttributesMutator();
      mutator.RemoveCqListener(cqLstner);
      cqAttr = qry1.GetCqAttributes();
      Util.Log("cqAttr.getCqListeners().Length = {0}", cqAttr.getCqListeners().Length);
      Assert.AreEqual(1, cqAttr.getCqListeners().Length);

      mutator.RemoveCqListener(cqStatusLstner);
      cqAttr = qry1.GetCqAttributes();
      Util.Log("1 cqAttr.getCqListeners().Length = {0}", cqAttr.getCqListeners().Length);
      Assert.AreEqual(0, cqAttr.getCqListeners().Length);

      region["4"] = p1;
      region["3"] = p2;
      region["2"] = p3;
      region["1"] = p4;
      Thread.Sleep(18000); // sleep 0.3min to allow server c query to complete

      qry1 = qs.GetCq<object, object>("CQ1");
      cqAttr = qry1.GetCqAttributes();
      ICqListener<object, object>[] v4 = cqAttr.getCqListeners();      
      Assert.IsNotNull(v4);      
      Assert.AreEqual(0, v4.Length);
      Util.Log("cqAttr.getCqListeners() done");
    }

    [Test]
    public void CqQueryAttributeMutatorTest()
    {
      CacheHelper.SetupJavaServers(true, "remotequeryN.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(ProcessCQ, CacheHelper.Locators);
      Util.Log("ProcessCQ complete.");

      m_client1.Call(Close);

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }

  }
}
