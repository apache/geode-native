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
  public class ThinClientRegionQueryTests : ThinClientRegionSteps
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

    public void KillServer()
    {
      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
    }

    public delegate void KillServerDelegate();

    public void StepThreeRQ()
    {
      bool ErrorOccurred = false;

      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.RegionQueries)
      {
        if (qrystr.Category == QueryCategory.Unsupported)
        {
          Util.Log("Skipping query index {0} because it is unsupported.", qryIdx);
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating query index {0}. {1}", qryIdx, qrystr.Query);

        if (m_isPdx)
        {
          if (qryIdx == 18)
          {
            Util.Log("Skipping query index {0} because it is unsupported for pdx type.", qryIdx);
            qryIdx++;
            continue;
          }
        }

        ISelectResults<object> results = region.Query<object>(qrystr.Query);

        if (results.Size != (ulong) QueryStatics.RegionQueryRowCounts[qryIdx])
        {
          ErrorOccurred = true;
          Util.Log("FAIL: Query # {0} expected result size is {1}, actual is {2}", qryIdx,
            QueryStatics.RegionQueryRowCounts[qryIdx], results.Size);
          qryIdx++;
          continue;
        }
        qryIdx++;
      }

      Assert.IsFalse(ErrorOccurred, "One or more query validation errors occurred.");

      try
      {
        ISelectResults<object> results = region.Query<object>("");
        Assert.Fail("Expected IllegalArgumentException exception for empty predicate");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for empty predicate:");
        Util.Log(ex.Message);
      }


      try
      {
        ISelectResults<object> results = region.Query<object>(QueryStatics.RegionQueries[0].Query, TimeSpan.FromSeconds(2200000));
        Assert.Fail("Expected IllegalArgumentException exception for invalid timeout");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for invalid timeout:");
        Util.Log(ex.Message);
      }


      try
      {
        ISelectResults<object> results = region.Query<object>("bad predicate");
        Assert.Fail("Expected QueryException exception for wrong predicate");
      }
      catch (QueryException ex)
      {
        Util.Log("got expected QueryException exception for wrong predicate:");
        Util.Log(ex.Message);
      }
    }

    public void StepFourRQ()
    {
      bool ErrorOccurred = false;

      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.RegionQueries)
      {
        if (qrystr.Category == QueryCategory.Unsupported)
        {
          Util.Log("Skipping query index {0} because it is unsupported.", qryIdx);
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating query index {0}.{1}", qryIdx, qrystr.Query);

        bool existsValue = region.ExistsValue(qrystr.Query);
        bool expectedResult = QueryStatics.RegionQueryRowCounts[qryIdx] > 0 ? true : false;

        if (existsValue != expectedResult)
        {
          ErrorOccurred = true;
          Util.Log("FAIL: Query # {0} existsValue expected is {1}, actual is {2}", qryIdx,
            expectedResult ? "true" : "false", existsValue ? "true" : "false");
          qryIdx++;
          continue;
        }

        qryIdx++;
      }

      Assert.IsFalse(ErrorOccurred, "One or more query validation errors occurred.");
      try
      {
        bool existsValue = region.ExistsValue("");
        Assert.Fail("Expected IllegalArgumentException exception for empty predicate");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for empty predicate:");
        Util.Log(ex.Message);
      }


      try
      {
        bool existsValue = region.ExistsValue(QueryStatics.RegionQueries[0].Query, TimeSpan.FromSeconds(2200000));
        Assert.Fail("Expected IllegalArgumentException exception for invalid timeout");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for invalid timeout:");
        Util.Log(ex.Message);
      }


      try
      {
        bool existsValue = region.ExistsValue("bad predicate");
        Assert.Fail("Expected QueryException exception for wrong predicate");
      }
      catch (QueryException ex)
      {
        Util.Log("got expected QueryException exception for wrong predicate:");
        Util.Log(ex.Message);
      }
    }

    public void StepFiveRQ()
    {
      bool ErrorOccurred = false;

      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.RegionQueries)
      {
        if (qrystr.Category == QueryCategory.Unsupported)
        {
          Util.Log("Skipping query index {0} because it is unsupported.", qryIdx);
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating query index {0}.", qryIdx);

        try
        {
          Object result = region.SelectValue(qrystr.Query);

          if (!(QueryStatics.RegionQueryRowCounts[qryIdx] == 0 ||
            QueryStatics.RegionQueryRowCounts[qryIdx] == 1))
          {
            ErrorOccurred = true;
            Util.Log("FAIL: Query # {0} expected query exception did not occur", qryIdx);
            qryIdx++;
            continue;
          }
        }
        catch (QueryException)
        {
          if (QueryStatics.RegionQueryRowCounts[qryIdx] == 0 ||
            QueryStatics.RegionQueryRowCounts[qryIdx] == 1)
          {
            ErrorOccurred = true;
            Util.Log("FAIL: Query # {0} unexpected query exception occured", qryIdx);
            qryIdx++;
            continue;
          }
        }
        catch (Exception)
        {
          ErrorOccurred = true;
          Util.Log("FAIL: Query # {0} unexpected exception occured", qryIdx);
          qryIdx++;
          continue;
        }

        qryIdx++;
      }

      Assert.IsFalse(ErrorOccurred, "One or more query validation errors occurred.");

      try
      {
        Object result = region.SelectValue("");
        Assert.Fail("Expected IllegalArgumentException exception for empty predicate");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for empty predicate:");
        Util.Log(ex.Message);
      }


      try
      {
        Object result = region.SelectValue(QueryStatics.RegionQueries[0].Query, TimeSpan.FromSeconds(2200000));
        Assert.Fail("Expected IllegalArgumentException exception for invalid timeout");
      }
      catch (IllegalArgumentException ex)
      {
        Util.Log("got expected IllegalArgumentException exception for invalid timeout:");
        Util.Log(ex.Message);
      }

      try
      {
        Object result = region.SelectValue("bad predicate");
        Assert.Fail("Expected QueryException exception for wrong predicate");
      }
      catch (QueryException ex)
      {
        Util.Log("got expected QueryException exception for wrong predicate:");
        Util.Log(ex.Message);
      }
    }

    public void StepSixRQ()
    {
      bool ErrorOccurred = false;

      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(QueryRegionNames[0]);

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.RegionQueries)
      {
        if ((qrystr.Category != QueryCategory.Unsupported) || (qryIdx == 3))
        {
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating unsupported query index {0}.", qryIdx);

        try
        {
          ISelectResults<object> results = region.Query<object>(qrystr.Query);

          Util.Log("Query # {0} expected exception did not occur", qryIdx);
          ErrorOccurred = true;
          qryIdx++;
        }
        catch (QueryException)
        {
          // ok, exception expected, do nothing.
          qryIdx++;
        }
        catch (Exception)
        {
          ErrorOccurred = true;
          Util.Log("FAIL: Query # {0} unexpected exception occured", qryIdx);
          qryIdx++;
        }
      }

      Assert.IsFalse(ErrorOccurred, "Query expected exceptions did not occur.");
    }

    #endregion

    void runRegionQuery()
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

      //Extra Step
      //m_client1.Call(StepExtra);

      m_client2.Call(StepThreeRQ);
      Util.Log("StepThree complete.");

      m_client2.Call(StepFourRQ);
      Util.Log("StepFour complete.");

      m_client2.Call(StepFiveRQ);
      Util.Log("StepFive complete.");

      m_client2.Call(StepSixRQ);
      Util.Log("StepSix complete.");

      m_client2.Call(Close);
      Util.Log("Client closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }

    static bool m_isPdx = false;

    [Test]
    public void RegionQueryWithPdx()
    {
      m_isPdx = true;
      runRegionQuery();
    }

    [Test]
    public void RegionQueryWithoutPdx()
    {
      m_isPdx = false;
      runRegionQuery();
    }

  }
}
