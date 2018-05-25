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

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using DUnitFramework;
  using Client;

  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  public class RegionFailoverTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess _mClient1, _mClient2;

    #endregion

    protected override ClientBase[] GetClients()
    {
      _mClient1 = new UnitProcess();
      _mClient2 = new UnitProcess();
      return new ClientBase[] { _mClient1, _mClient2 };
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
        _mClient1.Call(DestroyRegions);
        _mClient2.Call(DestroyRegions);
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

    private void RunFailover()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _mClient1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _mClient2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _mClient1.Call(StepThree);
      Util.Log("StepThree complete.");

      _mClient2.Call(StepFour);
      Util.Log("StepFour complete.");

      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      _mClient1.Call(StepFiveFailover);
      Util.Log("StepFive complete.");

      _mClient2.Call(StepSix, false);
      Util.Log("StepSix complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunFailover2()
    {
      // This test is for client failover with client notification.
      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _mClient1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepOne (pool locators) complete.");

      _mClient2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepTwo (pool locators) complete.");

      _mClient1.Call(RegisterAllKeysR0WithoutValues);
      _mClient1.Call(RegisterAllKeysR1WithoutValues);

      _mClient2.Call(RegisterAllKeysR0WithoutValues);
      _mClient2.Call(RegisterAllKeysR1WithoutValues);

      _mClient1.Call(StepThree);
      Util.Log("StepThree complete.");

      _mClient2.Call(StepFour);
      Util.Log("StepFour complete.");

      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      _mClient1.Call(CheckServerKeys);
      _mClient1.Call(StepFive, false);
      Util.Log("StepFive complete.");

      _mClient2.Call(StepSixNotify, false);
      Util.Log("StepSix complete.");

      _mClient1.Call(StepSevenNotify, false);
      Util.Log("StepSeven complete.");

      _mClient2.Call(StepEightNotify, false);
      Util.Log("StepEight complete.");

      _mClient1.Call(StepNineNotify, false);
      Util.Log("StepNine complete.");

      _mClient2.Call(StepTen);
      Util.Log("StepTen complete.");

      _mClient1.Call(StepEleven);
      Util.Log("StepEleven complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunFailover3()
    {
      CacheHelper.SetupJavaServers(true,
        "cacheserver.xml", "cacheserver2.xml", "cacheserver3.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _mClient1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _mClient2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _mClient1.Call(StepThree);
      Util.Log("StepThree complete.");

      _mClient2.Call(StepFour);
      Util.Log("StepFour complete.");

      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      _mClient1.Call(StepFive, false);
      Util.Log("StepFive complete.");

      _mClient2.Call(StepSix, false);
      Util.Log("StepSix complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");
      CacheHelper.StopJavaServer(3);
      Util.Log("Cacheserver 3 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunFailoverInterestAll()
    {
      RunFailoverInterestAll(false);
    }

    private void RunFailoverInterestAll(bool ssl)
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_notify_subscription.xml",
        "cacheserver_notify_subscription2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC", null, ssl);
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1, ssl);
      Util.Log("Cacheserver 1 started.");

      _mClient1.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", true, ssl);

      _mClient1.Call(StepThree);
      Util.Log("StepThree complete.");

      _mClient2.Call(CreateTCRegions_Pool, RegionNames,
          CacheHelper.Locators, "__TESTPOOL1_", true, ssl);

      Util.Log("CreateTCRegions complete.");

      _mClient2.Call(RegexInterestAllStep2);
      Util.Log("RegexInterestAllStep2 complete.");

      _mClient2.Call(RegexInterestAllStep3, CacheHelper.Locators);
      Util.Log("RegexInterestAllStep3 complete.");

      _mClient1.Call(RegexInterestAllStep4);
      Util.Log("RegexInterestAllStep4 complete.");

      _mClient2.Call(RegexInterestAllStep5);
      Util.Log("RegexInterestAllStep5 complete.");

      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1, ssl);

      CacheHelper.StopJavaServer(1); //failover happens
      Util.Log("Cacheserver 2 started and failover forced");

      _mClient1.Call(RegexInterestAllStep6);
      Util.Log("RegexInterestAllStep6 complete.");

      System.Threading.Thread.Sleep(5000); // sleep to let updates arrive

      _mClient2.Call(RegexInterestAllStep7);
      Util.Log("RegexInterestAllStep7 complete.");

      _mClient2.Call(RegexInterestAllStep8);
      Util.Log("RegexInterestAllStep8 complete.");

      _mClient1.Call(RegexInterestAllStep9);
      Util.Log("RegexInterestAllStep9 complete.");

      _mClient2.Call(RegexInterestAllStep10);
      Util.Log("RegexInterestAllStep10 complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1, true, ssl);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void Failover()
    {
      RunFailover();
    }

    [Test]
    public void Failover2()
    {
      RunFailover2();
    }

    [Test]
    public void Failover3()
    {
      RunFailover3();
    }

    [Test]
    public void FailOverInterestAll()
    {
      RunFailoverInterestAll();
    }

    #endregion
  }
}
