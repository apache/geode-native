//=========================================================================
// Copyright (c) 2002-2014 Pivotal Software, Inc. All Rights Reserved.
// This product is protected by U.S. and international copyright
// and intellectual property laws. Pivotal products are covered by
// more patents listed at http://www.pivotal.io/patents.
//========================================================================

using System;
using System.Collections.Generic;
using System.Collections;
using System.Threading;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  using Apache.Geode.Client.Tests;

  using Apache.Geode.Client;
  using Region = Apache.Geode.Client.IRegion<Object, Object>;

  public class MyAppDomainResultCollector<TResult> : IResultCollector<TResult>
  {
    #region Private members
    private bool m_resultReady = false;
    ICollection<TResult> m_results = null;
    private int m_addResultCount = 0;
    private int m_getResultCount = 0;
    private int m_endResultCount = 0;
    #endregion
    public int GetAddResultCount()
    {
      return m_addResultCount;
    }
    public int GetGetResultCount()
    {
      return m_getResultCount;
    }
    public int GetEndResultCount()
    {
      return m_endResultCount;
    }
    public MyAppDomainResultCollector()
    {
      m_results = new List<TResult>();
    }
    public void AddResult(TResult result)
    {
      Util.Log("MyAppDomainResultCollector " + result + " :  " + result.GetType());
      m_addResultCount++;
      m_results.Add(result);
    }
    public ICollection<TResult> GetResult()
    {
      return GetResult(TimeSpan.FromSeconds(50));
    }

    public ICollection<TResult> GetResult(TimeSpan timeout)
    {
      m_getResultCount++;

      lock (this) {
        if (!m_resultReady) {
          if (timeout > TimeSpan.Zero) {
            if (!Monitor.Wait(this, timeout)) {
              throw new FunctionExecutionException("Timeout waiting for result.");
            }
          } else {
            throw new FunctionExecutionException("Results not ready.");
          }
        }
      }

      return m_results;
    }

    public void EndResults()
    {
      m_endResultCount++;

      lock (this) {
        m_resultReady = true;
        Monitor.Pulse(this);
      }
    }

    public void ClearResults(/*bool unused*/)
    {
      m_results.Clear();
      m_addResultCount = 0;
      m_getResultCount = 0;
      m_endResultCount = 0;
      m_resultReady = false;
    }
  }


  [TestFixture]
  [Category("group3")]
  [Category("unicast_only")]
  [Category("generics")]
  public class ThinClientAppDomainFunctionExecutionTests : ThinClientRegionSteps
  {
    #region Private members

    private static string[] FunctionExecutionRegionNames = { "partition_region", "partition_region1" };
    private static string poolName = "__TEST_POOL1__";
    private static string serverGroup = "ServerGroup1";
    private static string QERegionName = "partition_region";
    private static string OnServerHAExceptionFunction = "OnServerHAExceptionFunction";
    private static string OnServerHAShutdownFunction = "OnServerHAShutdownFunction";

    #endregion

    protected override ClientBase[] GetClients()
    {
      return new ClientBase[] { };
    }

    [TestFixtureSetUp]
    public override void InitTests()
    {
      Util.Log("InitTests: AppDomain: " + AppDomain.CurrentDomain.Id);
      CacheHelper.Init();
    }

    [TearDown]
    public override void EndTest()
    {
      Util.Log("EndTest: AppDomain: " + AppDomain.CurrentDomain.Id);
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
      CacheHelper.CreateTCRegion_Pool<object, object>(regionName, true, true, null, null, poolName, false, serverGroup);
    }
    public void createPool(string name, string locators, string serverGroup,
      int redundancy, bool subscription, bool prSingleHop, bool threadLocal = false)
    {
      CacheHelper.CreatePool<object, object>(name, locators, serverGroup, redundancy, subscription, prSingleHop, threadLocal);
    }
 
    public void OnServerHAStepOne()
    {

      Region region = CacheHelper.GetVerifyRegion<object, object>(QERegionName);
      for (int i = 0; i < 34; i++)
      {
        region["KEY--" + i] = "VALUE--" + i;
      }

      object[] routingObj = new object[17];

      ArrayList args1 = new ArrayList();

      int j = 0;
      for (int i = 0; i < 34; i++)
      {
        if (i % 2 == 0) continue;
        routingObj[j] = "KEY--" + i;
        j++;
      }
      Util.Log("routingObj count= {0}.", routingObj.Length);

      for (int i = 0; i < routingObj.Length; i++)
      {
        Console.WriteLine("routingObj[{0}]={1}.", i, (string)routingObj[i]);
        args1.Add(routingObj[i]);
      }

      //test data independant function execution with result onServer
      Pool/*<TKey, TValue>*/ pool = CacheHelper.DCache.GetPoolManager().Find(poolName);

      Apache.Geode.Client.Execution<object> exc = FunctionService<object>.OnServer(pool);
      Assert.IsTrue(exc != null, "onServer Returned NULL");

      IResultCollector<object> rc = exc.WithArgs<ArrayList>(args1).Execute(OnServerHAExceptionFunction, TimeSpan.FromSeconds(15));

      ICollection<object> executeFunctionResult = rc.GetResult();

      List<object> resultList = new List<object>();

      Console.WriteLine("executeFunctionResult.Length = {0}", executeFunctionResult.Count);

      foreach (List<object> item in executeFunctionResult)
      {
        foreach (object item2 in item)
        {
          resultList.Add(item2);
        }
      }

      Util.Log("on region: result count= {0}.", resultList.Count);
      Assert.IsTrue(resultList.Count == 17, "result count check failed");
      for (int i = 0; i < resultList.Count; i++)
      {
        Util.Log("on region:get:result[{0}]={1}.", i, (string)resultList[i]);
        Assert.IsTrue(((string)resultList[i]) != null, "onServer Returned NULL");
      }

      rc = exc.WithArgs<ArrayList>(args1).Execute(OnServerHAShutdownFunction, TimeSpan.FromSeconds(15));

      ICollection<object> executeFunctionResult1 = rc.GetResult();

      List<object> resultList1 = new List<object>();

      foreach (List<object> item in executeFunctionResult1)
      {
        foreach (object item2 in item)
        {
          resultList1.Add(item2);
        }
      }

      Util.Log("on region: result count= {0}.", resultList1.Count);

      Console.WriteLine("resultList1.Count = {0}", resultList1.Count);

      Assert.IsTrue(resultList1.Count == 17, "result count check failed");
      for (int i = 0; i < resultList1.Count; i++)
      {
        Util.Log("on region:get:result[{0}]={1}.", i, (string)resultList1[i]);
        Assert.IsTrue(((string)resultList1[i]) != null, "onServer Returned NULL");
      }

      // Bring down the region
      //region.LocalDestroyRegion();
    }

    [Test]
    public void OnServerHAExecuteFunction()
    {
      Util.Log("OnServerHAExecuteFunction: AppDomain: " + AppDomain.CurrentDomain.Id);
      CacheHelper.SetupJavaServers(true, "func_cacheserver1_pool.xml",
      "func_cacheserver2_pool.xml", "func_cacheserver3_pool.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      createPool(poolName, CacheHelper.Locators, serverGroup, 1, true, true, /*threadLocal*/true);
      createRegionAndAttachPool(QERegionName, poolName);
      Util.Log("Client 1 (pool locator) regions created");

      OnServerHAStepOne();

      Close();
      Util.Log("Client 1 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaServer(3);
      Util.Log("Cacheserver 3 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }
  }
}
