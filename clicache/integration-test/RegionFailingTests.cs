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
using System.Collections.ObjectModel;
using System.IO;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using DUnitFramework;
  using Client;

  [Ignore("broken")]
  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  public class RegionFailingTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess _client1, _client2;

    #endregion

    protected override ClientBase[] GetClients()
    {
      _client1 = new UnitProcess();
      _client2 = new UnitProcess();
      return new ClientBase[] { _client1, _client2 };
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
        _client1.Call(DestroyRegions);
        _client2.Call(DestroyRegions);
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

    private void RunCheckPutGet()
    {
      CacheHelper.SetupJavaServers(true, "cacheServer_pdxreadserialized.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      var putGetTest = new PutGetTests();

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) regions created");
      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 2 (pool locators) regions created");

      _client1.Call(putGetTest.SetRegion, RegionNames[0]);
      _client2.Call(putGetTest.SetRegion, RegionNames[0]);

      var dtTicks = DateTime.Now.Ticks;
      CacheableHelper.RegisterBuiltins(dtTicks);

      putGetTest.TestAllKeyValuePairs(_client1, _client2,
        RegionNames[0], true, dtTicks);

      _client1.Call(Close);
      Util.Log("Client 1 closed");
      _client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunCheckPutGetWithAppDomain()
    {
      CacheHelper.SetupJavaServers(true, "cacheServer_pdxreadserialized.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      var putGetTest = new PutGetTests();

      _client1.Call(InitializeAppDomain);
      var dtTime = DateTime.Now.Ticks;
      _client1.Call(CreateTCRegions_Pool_AD, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, dtTime);
      Util.Log("Client 1 (pool locators) regions created");

      _client1.Call(SetRegionAD, RegionNames[0]);
      //m_client2.Call(putGetTest.SetRegion, RegionNames[0]);

      // CacheableHelper.RegisterBuiltins();

      //putGetTest.TestAllKeyValuePairs(m_client1, m_client2,
      //RegionNames[0], true, pool);
      _client1.Call(TestAllKeyValuePairsAD, RegionNames[0], true, dtTime);
      //m_client1.Call(CloseCacheAD);
      Util.Log("Client 1 closed");

      CacheHelper.CloseCache();

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunPdxAppDomainTest(bool caching, bool readPdxSerialized)
    {
      CacheHelper.SetupJavaServers(true, "cacheServer_pdxreadserialized.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");


      _client1.Call(InitializeAppDomain);

      _client1.Call(CreateTCRegions_Pool_AD2, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, caching, readPdxSerialized);
      Util.Log("Client 1 (pool locators) regions created");

      _client1.Call(SetRegionAD, RegionNames[0]);

      _client1.Call(pdxPutGetTest, caching, readPdxSerialized);
      _client1.Call(pdxGetPutTest, caching, readPdxSerialized);
      //m_client2.Call(putGetTest.SetRegion, RegionNames[0]);

      // CacheableHelper.RegisterBuiltins();

      //putGetTest.TestAllKeyValuePairs(m_client1, m_client2,
      //RegionNames[0], true, pool);
      // m_client1.Call(TestAllKeyValuePairsAD, RegionNames[0], true, pool);
      _client1.Call(CloseCacheAD);

      Util.Log("Client 1 closed");

      //CacheHelper.CloseCache();

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunCheckPut()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_hashcode.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      var putGetTest = new PutGetTests();

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) regions created");
      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 2 (pool locators) regions created");

      _client1.Call(putGetTest.SetRegion, RegionNames[0]);
      _client2.Call(putGetTest.SetRegion, RegionNames[0]);
      var dtTime = DateTime.Now.Ticks;
      CacheableHelper.RegisterBuiltinsJavaHashCode(dtTime);
      putGetTest.TestAllKeys(_client1, _client2, RegionNames[0], dtTime);

      _client1.Call(Close);
      Util.Log("Client 1 closed");

      _client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunFixedPartitionResolver()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver1_fpr.xml",
          "cacheserver2_fpr.xml", "cacheserver3_fpr.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      var putGetTest = new PutGetTests();

      _client1.Call(CreateTCRegions_Pool1, PartitionRegion1,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) PartitionRegion1 created");

      _client1.Call(CreateTCRegions_Pool1, PartitionRegion2,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) PartitionRegion2 created");

      _client1.Call(CreateTCRegions_Pool1, PartitionRegion3,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) PartitionRegion3 created");

      _client1.Call(putGetTest.SetRegion, PartitionRegion1);
      putGetTest.DoPRSHFixedPartitionResolverTasks(_client1, PartitionRegion1);

      _client1.Call(putGetTest.SetRegion, PartitionRegion2);
      putGetTest.DoPRSHFixedPartitionResolverTasks(_client1, PartitionRegion2);

      _client1.Call(putGetTest.SetRegion, PartitionRegion3);
      putGetTest.DoPRSHFixedPartitionResolverTasks(_client1, PartitionRegion3);

      _client1.Call(Close);
      Util.Log("Client 1 closed");

      _client2.Call(Close);
      Util.Log("Client 2 closed");

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

    private void RegisterOtherType()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(OtherType.CreateDeserializable, 0);
      }
      catch (IllegalStateException)
      {
        // ignored since we run multiple times for pool and non pool cases.
      }
    }

    private void DoPutsOtherTypeWithEx(OtherType.ExceptionType exType)
    {
      var region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      for (var keyNum = 1; keyNum <= 10; ++keyNum)
      {
        try
        {
          region["key-" + keyNum] = new OtherType(keyNum, keyNum * keyNum, exType);
          if (exType != OtherType.ExceptionType.None)
          {
            Assert.Fail("Expected an exception in Put");
          }
        }
        catch (GeodeIOException ex)
        {
          if (exType == OtherType.ExceptionType.Geode)
          {
            // Successfully changed exception back and forth
            Util.Log("Got expected exception in Put: " + ex);
          }
          else if (exType == OtherType.ExceptionType.GeodeGeode)
          {
            if (ex.InnerException is CacheServerException)
            {
              // Successfully changed exception back and forth
              Util.Log("Got expected exception in Put: " + ex);
            }
            else
            {
              throw;
            }
          }
          else
          {
            throw;
          }
        }
        catch (CacheServerException ex)
        {
          if (exType == OtherType.ExceptionType.GeodeSystem)
          {
            if (ex.InnerException is IOException)
            {
              // Successfully changed exception back and forth
              Util.Log("Got expected exception in Put: " + ex);
            }
            else
            {
              throw;
            }
          }
          else
          {
            throw;
          }
        }
        catch (IOException ex)
        {
          if (exType == OtherType.ExceptionType.System)
          {
            // Successfully changed exception back and forth
            Util.Log("Got expected system exception in Put: " + ex);
          }
          else
          {
            throw;
          }
        }
        catch (ApplicationException ex)
        {
          if (exType == OtherType.ExceptionType.SystemGeode)
          {
            if (ex.InnerException is CacheServerException)
            {
              // Successfully changed exception back and forth
              Util.Log("Got expected system exception in Put: " + ex);
            }
            else
            {
              throw;
            }
          }
          else if (exType == OtherType.ExceptionType.SystemSystem)
          {
            if (ex.InnerException is IOException)
            {
              // Successfully changed exception back and forth
              Util.Log("Got expected system exception in Put: " + ex);
            }
            else
            {
              throw;
            }
          }
          else
          {
            throw;
          }
        }
      }
    }

    private void RunCheckNativeException()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(RegisterOtherType);

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.None);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.Geode);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.System);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.GeodeGeode);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.GeodeSystem);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.SystemGeode);
      _client1.Call(DoPutsOtherTypeWithEx, OtherType.ExceptionType.SystemSystem);

      _client1.Call(Close);
      Util.Log("Client 1 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }
    
    private void RunRemoveAllWithSingleHop()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver1_pr.xml",
        "cacheserver2_pr.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("Client 1 (pool locators) regions created");
      Util.Log("Region creation complete.");

      _client1.Call(RemoveAllSingleHopStep);
      Util.Log("RemoveAllSingleHopStep complete.");

      _client1.Call(Close);
      Util.Log("Client 1 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    protected virtual void RemoveAllSingleHopStep()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      ICollection<object> keys = new Collection<object>();
      for (var y = 0; y < 1000; y++)
      {
        Util.Log("put:{0}", y);
        region0[y] = y;
        keys.Add(y);
      }
      region0.RemoveAll(keys);
      Assert.AreEqual(0, region0.Count);
      Util.Log("RemoveAllSingleHopStep completed");
    }

    private void RunRemoveOps()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _client1.Call(StepThree);
      Util.Log("StepThree complete.");

      _client1.Call(RemoveStepFive);
      Util.Log("RemoveStepFive complete.");

      _client2.Call(RemoveStepSix);
      Util.Log("RemoveStepSix complete.");

      _client1.Call(Close);
      Util.Log("Client 1 closed");
      _client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RegexInterestAllStep2() //client 2 //pxr2
    {
      Util.Log("RegexInterestAllStep2 Enters.");
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      //CreateEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      //CreateEntry(m_regionNames[1], m_keys[1], m_vals[1]);
      region0.GetSubscriptionService().RegisterAllKeys(false, true);
      region1.GetSubscriptionService().RegisterAllKeys(false, true);
      if (region0.Count != 1 || region1.Count != 1)
      {
        Assert.Fail("Expected one entry in region");
      }
      Util.Log("RegexInterestAllStep2 complete.");
    }

    private void RegexInterestAllStep3(string locators)
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region0.GetSubscriptionService().UnregisterAllKeys();
      region1.GetSubscriptionService().UnregisterAllKeys();
      region0.GetLocalView().DestroyRegion();
      region1.GetLocalView().DestroyRegion();
      CreateTCRegions_Pool(RegionNames, locators, "__TESTPOOL1_", true);
      region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      CreateEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      region0.GetSubscriptionService().RegisterRegex(".*", false, true);
      region1.GetSubscriptionService().RegisterRegex(".*", false, true);
      if (region0.Count != 1)
      {
        Assert.Fail("Expected one entry in region");
      }
      if (region1.Count != 1)
      {
        Assert.Fail("Expected one entry in region");
      }
      VerifyCreated(m_regionNames[0], m_keys[0]);
      VerifyCreated(m_regionNames[1], m_keys[2]);
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
      Util.Log("RegexInterestAllStep3 complete.");
    }

    private void RegexInterestAllStep4()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0.GetSubscriptionService().RegisterAllKeys(false, false);
      if (region0.Count != 1)
      {
        Assert.Fail("Expected one entry in region");
      }
      if (!region0.ContainsKey(m_keys[0]))
      {
        Assert.Fail("Expected region to contain the key");
      }

      if (region0.ContainsValueForKey(m_keys[0]))
      {
        Assert.Fail("Expected region to not contain the value");
      }

      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region1.GetSubscriptionService().RegisterRegex(".*", false, false);

      if (region1.Count != 1)
      {
        Assert.Fail("Expected one entry in region");
      }

      if (!region1.ContainsKey(m_keys[2]))
      {
        Assert.Fail("Expected region to contain the key");
      }

      if (region1.ContainsValueForKey(m_keys[2]))
      {
        Assert.Fail("Expected region to not contain the value");
      }
      CreateEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      UpdateEntry(m_regionNames[0], m_keys[0], m_vals[0], false);
      CreateEntry(m_regionNames[1], m_keys[3], m_vals[3]);
    }

    private void RegexInterestAllStep5()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      if (region1.Count != 2 || region0.Count != 2)
      {
        Assert.Fail("Expected two entry in region");
      }
      VerifyCreated(m_regionNames[0], m_keys[0]);
      VerifyCreated(m_regionNames[1], m_keys[2]);
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);

    }

    private void RegexInterestAllStep6()
    {
      UpdateEntry(m_regionNames[0], m_keys[0], m_nvals[0], false);
      UpdateEntry(m_regionNames[1], m_keys[2], m_nvals[2], false);
    }

    private void RegexInterestAllStep7() //client 2 
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1], false);
      VerifyEntry(m_regionNames[1], m_keys[2], m_nvals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3], false);
    }

    private void RegexInterestAllStep8()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region0.GetSubscriptionService().UnregisterAllKeys();
      region1.GetSubscriptionService().UnregisterAllKeys();
    }

    private void RegexInterestAllStep9()
    {
      UpdateEntry(m_regionNames[0], m_keys[0], m_vals[0], false);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1], false);
      UpdateEntry(m_regionNames[1], m_keys[2], m_vals[2], false);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3], false);
    }

    private void RegexInterestAllStep10()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_nvals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);
    }

    private void RunFailoverInterestAll(bool ssl)
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_notify_subscription.xml",
        "cacheserver_notify_subscription2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC", null, ssl);
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1, ssl);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", true, ssl);

      _client1.Call(StepThree);
      Util.Log("StepThree complete.");

      _client2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", true, ssl);

      Util.Log("CreateTCRegions complete.");

      _client2.Call(RegexInterestAllStep2);
      Util.Log("RegexInterestAllStep2 complete.");

      _client2.Call(RegexInterestAllStep3, CacheHelper.Locators);
      Util.Log("RegexInterestAllStep3 complete.");

      _client1.Call(RegexInterestAllStep4);
      Util.Log("RegexInterestAllStep4 complete.");

      _client2.Call(RegexInterestAllStep5);
      Util.Log("RegexInterestAllStep5 complete.");

      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1, ssl);

      CacheHelper.StopJavaServer(1); //failover happens
      Util.Log("Cacheserver 2 started and failover forced");

      _client1.Call(RegexInterestAllStep6);
      Util.Log("RegexInterestAllStep6 complete.");

      System.Threading.Thread.Sleep(5000); // sleep to let updates arrive

      _client2.Call(RegexInterestAllStep7);
      Util.Log("RegexInterestAllStep7 complete.");

      _client2.Call(RegexInterestAllStep8);
      Util.Log("RegexInterestAllStep8 complete.");

      _client1.Call(RegexInterestAllStep9);
      Util.Log("RegexInterestAllStep9 complete.");

      _client2.Call(RegexInterestAllStep10);
      Util.Log("RegexInterestAllStep10 complete.");

      _client1.Call(Close);
      Util.Log("Client 1 closed");
      _client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1, true, ssl);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunIdictionaryOps()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames2,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _client1.Call(IdictionaryRegionOperations, "DistRegionAck");
      Util.Log("IdictionaryRegionOperations complete.");

      _client1.Call(IdictionaryRegionNullKeyOperations, "DistRegionAck");
      Util.Log("IdictionaryRegionNullKeyOperations complete.");

      _client1.Call(IdictionaryRegionArrayOperations, "DistRegionAck");
      Util.Log("IdictionaryRegionArrayOperations complete.");

      _client1.Call(Close);
      Util.Log("Client 1 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void CheckPutGet()
    {
      RunCheckPutGet();
    }

    [Test]
    public void CheckPutGetWithAppDomain()
    {
      RunCheckPutGetWithAppDomain();
    }

    [Test]
    public void PdxAppDomainTest()
    {
      RunPdxAppDomainTest(false, true); // pool with locators
      RunPdxAppDomainTest(true, true); // pool with locators
      RunPdxAppDomainTest(false, false); // pool with locators
      RunPdxAppDomainTest(true, false); // pool with locators
    }

    [Test]
    public void JavaHashCode()
    {
      RunCheckPut();
    }

    [Test]
    public void CheckFixedPartitionResolver()
    {
      RunFixedPartitionResolver();
    }

    [Test]
    public void CheckNativeException()
    {
      RunCheckNativeException();
    }

    [Test]
    public void RemoveAllWithSingleHop()
    {
      RunRemoveAllWithSingleHop();
    }

    [Test]
    public void RemoveOps()
    {
      RunRemoveOps();
    }

    [Test]
    public void FailOverInterestAllWithSsl()
    {
      RunFailoverInterestAll(true);
    }

    [Test]
    public void IdictionaryOps()
    {
      RunIdictionaryOps();
    }

    #endregion
  }
}
