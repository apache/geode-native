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
  public class ThinClientRemoteQueryStructSetTests : ThinClientRegionSteps
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
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(Apache.Geode.Client.Tests.PortfolioPdx.CreateDeserializable);
      CacheHelper.DCache.TypeRegistry.RegisterPdxType(Apache.Geode.Client.Tests.PositionPdx.CreateDeserializable);
    }


    public void StepOne(string locators, bool isPdx)
    {
      m_isPdx = isPdx;
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[0], true, true,
      null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[1], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[2], true, true,
        null, locators, "__TESTPOOL1_", true);
      CacheHelper.CreateTCRegion_Pool<object, object>(QueryRegionNames[3], true, true,
        null, locators, "__TESTPOOL1_", true);

      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);
      Apache.Geode.Client.RegionAttributes<object, object> regattrs = region.Attributes;
      region.CreateSubRegion(QueryRegionNames[1], regattrs);
    }

    public void StepTwo(bool isPdx)
    {
      m_isPdx = isPdx;
      IRegion<object, object> region0 = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);
      IRegion<object, object> subRegion0 = (IRegion<object, object>)region0.GetSubRegion(QueryRegionNames[1]);
      IRegion<object, object> region1 = CacheHelper.GetRegion<object, object>(QueryRegionNames[1]);
      IRegion<object, object> region2 = CacheHelper.GetRegion<object, object>(QueryRegionNames[2]);
      IRegion<object, object> region3 = CacheHelper.GetRegion<object, object>(QueryRegionNames[3]);

      QueryHelper<object, object> qh = QueryHelper<object, object>.GetHelper(CacheHelper.DCache);
      Util.Log("SetSize {0}, NumSets {1}.", qh.PortfolioSetSize,
        qh.PortfolioNumSets);

      if (!m_isPdx)
      {
        qh.PopulatePortfolioData(region0, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePositionData(subRegion0, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePositionData(region1, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePortfolioData(region2, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePortfolioData(region3, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
      }
      else
      {
        qh.PopulatePortfolioPdxData(region0, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePositionPdxData(subRegion0, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePositionPdxData(region1, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePortfolioPdxData(region2, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
        qh.PopulatePortfolioPdxData(region3, qh.PortfolioSetSize,
          qh.PortfolioNumSets);
      }
    }

    public void StepThreeSS()
    {
      bool ErrorOccurred = false;

      QueryHelper<object, object> qh = QueryHelper<object, object>.GetHelper(CacheHelper.DCache);

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.StructSetQueries)
      {
        if (qrystr.Category == QueryCategory.Unsupported)
        {
          Util.Log("Skipping query index {0} because it is unsupported.", qryIdx);
          qryIdx++;
          continue;
        }

        if (m_isPdx == true)
        {
          if (qryIdx == 12 || qryIdx == 4 || qryIdx == 7 || qryIdx == 22 || qryIdx == 30 || qryIdx == 34)
          {
            Util.Log("Skipping query index {0} for pdx because it has function.", qryIdx);
            qryIdx++;
            continue;
          }
        }

        Util.Log("Evaluating query index {0}. {1}", qryIdx, qrystr.Query);

        Query<object> query = qs.NewQuery<object>(qrystr.Query);

        ISelectResults<object> results = query.Execute();

        int expectedRowCount = qh.IsExpectedRowsConstantSS(qryIdx) ?
          QueryStatics.StructSetRowCounts[qryIdx] : QueryStatics.StructSetRowCounts[qryIdx] * qh.PortfolioNumSets;

        if (!qh.VerifySS(results, expectedRowCount, QueryStatics.StructSetFieldCounts[qryIdx]))
        {
          ErrorOccurred = true;
          Util.Log("Query verify failed for query index {0}.", qryIdx);
          qryIdx++;
          continue;
        }

        StructSet<object> ss = results as StructSet<object>;
        if (ss == null)
        {
          Util.Log("Zero records found for query index {0}, continuing.", qryIdx);
          qryIdx++;
          continue;
        }

        uint rows = 0;
        Int32 fields = 0;
        foreach (Struct si in ss)
        {
          rows++;
          fields = (Int32)si.Count;
        }

        Util.Log("Query index {0} has {1} rows and {2} fields.", qryIdx, rows, fields);

        qryIdx++;
      }

      Assert.IsFalse(ErrorOccurred, "One or more query validation errors occurred.");
    }

    public void StepFourSS()
    {
      bool ErrorOccurred = false;

      QueryHelper<object, object> qh = QueryHelper<object, object>.GetHelper(CacheHelper.DCache);

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.StructSetQueries)
      {
        if (qrystr.Category != QueryCategory.Unsupported)
        {
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating unsupported query index {0}.", qryIdx);

        Query<object> query = qs.NewQuery<object>(qrystr.Query);

        try
        {
          ISelectResults<object> results = query.Execute();

          Util.Log("Query exception did not occur for index {0}.", qryIdx);
          ErrorOccurred = true;
          qryIdx++;
        }
        catch (GeodeException)
        {
          // ok, exception expected, do nothing.
          qryIdx++;
        }
        catch (Exception)
        {
          Util.Log("Query unexpected exception occurred for index {0}.", qryIdx);
          ErrorOccurred = true;
          qryIdx++;
        }
      }

      Assert.IsFalse(ErrorOccurred, "Query expected exceptions did not occur.");
    }

    public void KillServer()
    {
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
    }

    public delegate void KillServerDelegate();

    #endregion

    void runRemoteQuerySS()
    {
      CacheHelper.SetupJavaServers(true, "remotequeryN.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client2.Call(StepOne, CacheHelper.Locators, m_isPdx);
      Util.Log("StepOne complete.");

      m_client2.Call(StepTwo, m_isPdx);
      Util.Log("StepTwo complete.");

      m_client2.Call(StepThreeSS);
      Util.Log("StepThree complete.");

      m_client2.Call(StepFourSS);
      Util.Log("StepFour complete.");

      //m_client2.Call(GetAllRegionQuery);

      m_client2.Call(Close);

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }

    static bool m_isPdx = false;


    [Test]
    public void RemoteQuerySSWithPdx()
    {
      m_isPdx = true;
      runRemoteQuerySS();
    }

    [Test]
    public void RemoteQuerySSWithoutPdx()
    {
      m_isPdx = false;
      runRemoteQuerySS();
    }

  }
}
