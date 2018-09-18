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

  using QueryStatics = Apache.Geode.Client.Tests.QueryStatics;
  using QueryCategory = Apache.Geode.Client.Tests.QueryCategory;
  using QueryStrings = Apache.Geode.Client.Tests.QueryStrings;

  [TestFixture]
  [Category("group1")]
  [Category("unicast_only")]
  [Category("generics")]
  public class ThinClientRemoteQueryFailoverTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1;
    private UnitProcess m_client2;
    private static string[] QueryRegionNames = { "Portfolios", "Positions", "Portfolios2",
      "Portfolios3" };
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
    }

    [TearDown]
    public override void EndTest()
    {
      m_client1.Call(Close);
      m_client2.Call(Close);
      CacheHelper.StopJavaServers();
      base.EndTest();
    }

    [SetUp]
    public override void InitTest()
    {
      m_client1.Call(InitClient);
      m_client2.Call(InitClient);
    }

    #region Functions invoked by the tests

    public void InitClient()
    {
      CacheHelper.Init();
      CacheHelper.DCache.TypeRegistry.RegisterType(Portfolio.CreateDeserializable, 8);
      CacheHelper.DCache.TypeRegistry.RegisterType(Position.CreateDeserializable, 7);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PortfolioPdx.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(PositionPdx.CreateDeserializable);
    }


    public void KillServer()
    {
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
    }

    public delegate void KillServerDelegate();

    public void StepOneFailover(bool isPdx)
    {
      m_isPdx = isPdx;
      // This is here so that Client1 registers information of the cacheserver
      // that has been already started
      CacheHelper.SetupJavaServers(true,
        "cacheserver_remoteoqlN.xml",
        "cacheserver_remoteoql2N.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[0], true, true, null,
        CacheHelper.Locators, "__TESTPOOL1_", true);

      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(QueryRegionNames[0]);
      if (!m_isPdx)
      {
        Portfolio p1 = new Portfolio(1, 100);
        Portfolio p2 = new Portfolio(2, 200);
        Portfolio p3 = new Portfolio(3, 300);
        Portfolio p4 = new Portfolio(4, 400);

        region["1"] = p1;
        region["2"] = p2;
        region["3"] = p3;
        region["4"] = p4;
      }
      else
      {
        PortfolioPdx p1 = new PortfolioPdx(1, 100);
        PortfolioPdx p2 = new PortfolioPdx(2, 200);
        PortfolioPdx p3 = new PortfolioPdx(3, 300);
        PortfolioPdx p4 = new PortfolioPdx(4, 400);

        region["1"] = p1;
        region["2"] = p2;
        region["3"] = p3;
        region["4"] = p4;
      }
    }

    public void StepTwoFailover()
    {
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");

      IAsyncResult killRes = null;
      KillServerDelegate ksd = new KillServerDelegate(KillServer);

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      for (int i = 0; i < 10000; i++)
      {
        Query<object> qry = qs.NewQuery<object>("select distinct * from /" + QueryRegionNames[0]);

        ISelectResults<object> results = qry.Execute();

        if (i == 10)
        {
          killRes = ksd.BeginInvoke(null, null);
        }

        var resultSize = results.Size;

        if (i % 100 == 0)
        {
          Util.Log("Iteration upto {0} done, result size is {1}", i, resultSize);
        }

        Assert.AreEqual(4, resultSize, "Result size is not 4!");
      }

      killRes.AsyncWaitHandle.WaitOne();
      ksd.EndInvoke(killRes);
    }

    #endregion

    void runRemoteQueryFailover()
    {
      try
      {
        m_client1.Call(StepOneFailover, m_isPdx);
        Util.Log("StepOneFailover complete.");

        m_client1.Call(StepTwoFailover);
        Util.Log("StepTwoFailover complete.");

        m_client1.Call(Close);
        Util.Log("Client closed");
      }
      finally
      {
        m_client1.Call(CacheHelper.StopJavaServers);
        m_client1.Call(CacheHelper.StopJavaLocator, 1);
      }
    }

    static bool m_isPdx = false;

    [Test]
    public void RemoteQueryFailoverWithPdx()
    {
      m_isPdx = true;
      runRemoteQueryFailover();
    }

    [Test]
    public void RemoteQueryFailoverWithoutPdx()
    {
      m_isPdx = false;
      runRemoteQueryFailover();
    }
  }
}
