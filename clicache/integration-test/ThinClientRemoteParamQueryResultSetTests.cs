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
  public class ThinClientRemoteParamQueryResultSetTests : ThinClientRegionSteps
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

    public void StepThreePQRS()
    {
      bool ErrorOccurred = false;

      QueryHelper<object, object> qh = QueryHelper<object, object>.GetHelper(CacheHelper.DCache);

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      int qryIdx = 0;

      foreach (QueryStrings paramqrystr in QueryStatics.ResultSetParamQueries)
      {
        if (paramqrystr.Category == QueryCategory.Unsupported)
        {
          Util.Log("Skipping query index {0} because it is unsupported.", qryIdx);
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating query index {0}. {1}", qryIdx, paramqrystr.Query);

        Query<object> query = qs.NewQuery<object>(paramqrystr.Query);

        //Populate the parameter list (paramList) for the query.
        object[] paramList = new object[QueryStatics.NoOfQueryParam[qryIdx]];
        int numVal = 0;
        for (int ind = 0; ind < QueryStatics.NoOfQueryParam[qryIdx]; ind++)
        {
          //Util.Log("NIL::PQRS:: QueryStatics.QueryParamSet[{0},{1}] = {2}", qryIdx, ind, QueryStatics.QueryParamSet[qryIdx][ind]);

          try
          {
            numVal = Convert.ToInt32(QueryStatics.QueryParamSet[qryIdx][ind]);
            paramList[ind] = numVal;
            //Util.Log("NIL::PQRS::361 Interger Args:: paramList[0] = {1}", ind, paramList[ind]);
          }
          catch (FormatException)
          {
            //Console.WriteLine("Param string is not a sequence of digits.");
            paramList[ind] = (System.String)QueryStatics.QueryParamSet[qryIdx][ind];
            //Util.Log("NIL::PQRS:: Interger Args:: routingObj[0] = {1}", ind, routingObj[ind].ToString());
          }
        }

        ISelectResults<object> results = query.Execute(paramList);

        //Varify the result
        int expectedRowCount = qh.IsExpectedRowsConstantPQRS(qryIdx) ?
        QueryStatics.ResultSetPQRowCounts[qryIdx] : QueryStatics.ResultSetPQRowCounts[qryIdx] * qh.PortfolioNumSets;

        if (!qh.VerifyRS(results, expectedRowCount))
        {
          ErrorOccurred = true;
          Util.Log("Query verify failed for query index {0}.", qryIdx);
          qryIdx++;
          continue;
        }

        ResultSet<object> rs = results as ResultSet<object>;

        foreach (object item in rs)
        {
          if (!m_isPdx)
          {
            Portfolio port = item as Portfolio;
            if (port == null)
            {
              Position pos = item as Position;
              if (pos == null)
              {
                string cs = item as string;
                if (cs == null)
                {
                  Util.Log("Query got other/unknown object.");
                }
                else
                {
                  Util.Log("Query got string : {0}.", cs);
                }
              }
              else
              {
                Util.Log("Query got Position object with secId {0}, shares {1}.", pos.SecId, pos.SharesOutstanding);
              }
            }
            else
            {
              Util.Log("Query got Portfolio object with ID {0}, pkid {1}.", port.ID, port.Pkid);
            }
          }
          else
          {
            PortfolioPdx port = item as PortfolioPdx;
            if (port == null)
            {
              PositionPdx pos = item as PositionPdx;
              if (pos == null)
              {
                string cs = item as string;
                if (cs == null)
                {
                  Util.Log("Query got other/unknown object.");
                }
                else
                {
                  Util.Log("Query got string : {0}.", cs);
                }
              }
              else
              {
                Util.Log("Query got PositionPdx object with secId {0}, shares {1}.", pos.secId, pos.getSharesOutstanding);
              }
            }
            else
            {
              Util.Log("Query got PortfolioPdx object with ID {0}, pkid {1}.", port.ID, port.Pkid);
            }
          }
        }

        qryIdx++;
      }

      Assert.IsFalse(ErrorOccurred, "One or more query validation errors occurred.");
    }

    public void StepFourPQRS()
    {
      bool ErrorOccurred = false;

      QueryHelper<object, object> qh = QueryHelper<object, object>.GetHelper(CacheHelper.DCache);

      var qs = CacheHelper.DCache.GetPoolManager().Find("__TESTPOOL1_").GetQueryService();

      int qryIdx = 0;

      foreach (QueryStrings qrystr in QueryStatics.ResultSetParamQueries)
      {
        if (qrystr.Category != QueryCategory.Unsupported)
        {
          qryIdx++;
          continue;
        }

        Util.Log("Evaluating unsupported query index {0}.", qryIdx);

        Query<object> query = qs.NewQuery<object>(qrystr.Query);

        object[] paramList = new object[QueryStatics.NoOfQueryParam[qryIdx]];

        Int32 numVal = 0;
        for (Int32 ind = 0; ind < QueryStatics.NoOfQueryParam[qryIdx]; ind++)
        {
          //Util.Log("NIL::PQRS:: QueryStatics.QueryParamSet[{0},{1}] = {2}", qryIdx, ind, QueryStatics.QueryParamSet[qryIdx, ind]);

          try
          {
            numVal = Convert.ToInt32(QueryStatics.QueryParamSet[qryIdx][ind]);
            paramList[ind] = numVal;
            //Util.Log("NIL::PQRS:: Interger Args:: paramList[0] = {1}", ind, paramList[ind]);
          }
          catch (FormatException)
          {
            //Console.WriteLine("Param string is not a sequence of digits.");
            paramList[ind] = (System.String)QueryStatics.QueryParamSet[qryIdx][ind];
            //Util.Log("NIL::PQRS:: Interger Args:: paramList[0] = {1}", ind, paramList[ind].ToString());
          }
        }

        try
        {
          ISelectResults<object> results = query.Execute(paramList);

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

    void runRemoteParamQueryRS()
    {
      CacheHelper.SetupJavaServers(true, "remotequeryN.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator started");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(StepOne, CacheHelper.Locators, m_isPdx);
      Util.Log("StepOne complete.");

      m_client1.Call(StepTwo, m_isPdx);
      Util.Log("StepTwo complete.");

      m_client1.Call(StepThreePQRS);
      Util.Log("StepThree complete.");

      m_client1.Call(StepFourPQRS);
      Util.Log("StepFour complete.");

      m_client1.Call(Close);

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator stopped");
    }

    static bool m_isPdx = false;

    [Test]
    public void RemoteParamQueryRSWithPdx()
    {
      m_isPdx = true;
      runRemoteParamQueryRS();
    }

    [Test]
    public void RemoteParamQueryRSWithoutPdx()
    {
      m_isPdx = false;
      runRemoteParamQueryRS();
    }
  }
}
