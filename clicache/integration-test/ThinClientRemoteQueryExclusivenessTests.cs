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
  public class ThinClientRemoteQueryExclusivenessTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1;
    private UnitProcess m_client2;
    private static string QERegionName = "Portfolios";
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

    public void StepOneQE(string locators, bool isPdx)
    {
      m_isPdx = isPdx;
      try
      {
				var poolFail = CacheHelper.DCache.GetPoolManager().CreateFactory().Create("_TESTFAILPOOL_");
				var qsFail = poolFail.GetQueryService();
        var qryFail = qsFail.NewQuery<object>("select distinct * from /" + QERegionName);
        var resultsFail = qryFail.Execute();
        Assert.Fail("Since no endpoints defined, so exception expected");
      }
      catch (IllegalStateException ex)
      {
        Util.Log("Got expected exception: {0}", ex);
      }
			catch (Exception e) {
				Util.Log("Caught unexpected exception: {0}", e);
				throw e;
			}

      CacheHelper.CreateTCRegion_Pool<object, object>(QERegionName, true, true,
        null, locators, "__TESTPOOL1_", true);
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(QERegionName);
      if (!m_isPdx)
      {
        Portfolio p1 = new Portfolio(1, 100);
        Portfolio p2 = new Portfolio(2, 100);
        Portfolio p3 = new Portfolio(3, 100);
        Portfolio p4 = new Portfolio(4, 100);

        region["1"] = p1;
        region["2"] = p2;
        region["3"] = p3;
        region["4"] = p4;
      }
      else
      {
        PortfolioPdx p1 = new PortfolioPdx(1, 100);
        PortfolioPdx p2 = new PortfolioPdx(2, 100);
        PortfolioPdx p3 = new PortfolioPdx(3, 100);
        PortfolioPdx p4 = new PortfolioPdx(4, 100);

        region["1"] = p1;
        region["2"] = p2;
        region["3"] = p3;
        region["4"] = p4;
      }

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      Query<object> qry = qs.NewQuery<object>("select distinct * from /" + QERegionName);
      ISelectResults<object> results = qry.Execute();
      var count = results.Size;
      Assert.AreEqual(4, count, "Expected 4 as number of portfolio objects.");

      // Bring down the region
      region.GetLocalView().DestroyRegion();
    }

    public void StepTwoQE()
    {
      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();
      Util.Log("Going to execute the query");
      Query<object> qry = qs.NewQuery<object>("select distinct * from /" + QERegionName);
      ISelectResults<object> results = qry.Execute();
      var count = results.Size;
      Assert.AreEqual(4, count, "Expected 4 as number of portfolio objects.");
    }

    public void KillServer()
    {
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
    }

    public delegate void KillServerDelegate();

    #endregion

    void runQueryExclusiveness()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_remoteoqlN.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(StepOneQE, CacheHelper.Locators, m_isPdx);
      Util.Log("StepOne complete.");

      m_client1.Call(StepTwoQE);
      Util.Log("StepTwo complete.");

      m_client1.Call(Close);
      Util.Log("Client closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }


    static bool m_isPdx = false;

    [Test]
    public void QueryExclusivenessWithoutPdx()
    {
      m_isPdx = false;
      runQueryExclusiveness();
    }

    [Test]
    public void QueryExclusivenessWithPdx()
    {
      m_isPdx = true;
      runQueryExclusiveness();
    }
  }
}
