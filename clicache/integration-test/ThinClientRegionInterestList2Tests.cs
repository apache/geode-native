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
  using Apache.Geode.Client;

  [TestFixture]
  [Category("group1")]
  [Category("unicast_only")]
  [Category("generics")]
  public class ThinClientRegionInterestList2Tests : ThinClientRegionSteps
  {
    #region Private members and methods

    private UnitProcess m_client1, m_client2, m_client3, m_feeder;
    private static string[] m_regexes = { "Key-*1", "Key-*2",
      "Key-*3", "Key-*4" };
    private const string m_regex23 = "Key-[23]";
    private const string m_regexWildcard = "Key-.*";
    private const int m_numUnicodeStrings = 5;

    private static string[] m_keysNonRegex = { "key-1", "key-2", "key-3" };
    private static string[] m_keysForRegex = {"key-regex-1",
      "key-regex-2", "key-regex-3" };
    private static string[] RegionNamesForInterestNotify =
      { "RegionTrue", "RegionFalse", "RegionOther" };

    string GetUnicodeString(int index)
    {
      return new string('\x0905', 40) + index.ToString("D10");
    }

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      m_client3 = new UnitProcess();
      m_feeder  = new UnitProcess();
      return new ClientBase[] { m_client1, m_client2, m_client3, m_feeder };
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
        m_client1.Call(DestroyRegions);
        m_client2.Call(DestroyRegions);
        CacheHelper.ClearEndpoints();
      }
      finally
      {
        CacheHelper.StopJavaServers();
      }
      base.EndTest();
    }

    #region Steps for Thin Client IRegion<object, object> with Interest

    public void StepFourIL()
    {
      VerifyCreated(m_regionNames[0], m_keys[0]);
      VerifyCreated(m_regionNames[1], m_keys[2]);
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
    }

    public void StepFourRegex3()
    {
      IRegion<object, object> region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      IRegion<object, object> region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      try
      {
        Util.Log("Registering empty regular expression.");
        region0.GetSubscriptionService().RegisterRegex(string.Empty);
        Assert.Fail("Did not get expected exception!");
      }
      catch (Exception ex)
      {
        Util.Log("Got expected exception {0}: {1}", ex.GetType(), ex.Message);
      }
      try
      {
        Util.Log("Registering null regular expression.");
        region1.GetSubscriptionService().RegisterRegex(null);
        Assert.Fail("Did not get expected exception!");
      }
      catch (Exception ex)
      {
        Util.Log("Got expected exception {0}: {1}", ex.GetType(), ex.Message);
      }
      try
      {
        Util.Log("Registering non-existent regular expression.");
        region1.GetSubscriptionService().UnregisterRegex("Non*Existent*Regex*");
        Assert.Fail("Did not get expected exception!");
      }
      catch (Exception ex)
      {
        Util.Log("Got expected exception {0}: {1}", ex.GetType(), ex.Message);
      }
    }

    public void StepFourFailoverRegex()
    {
      VerifyCreated(m_regionNames[0], m_keys[0]);
      VerifyCreated(m_regionNames[1], m_keys[2]);
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);

      UpdateEntry(m_regionNames[1], m_keys[1], m_vals[1], true);
      UnregisterRegexes(null, m_regexes[2]);
    }

    public void StepFiveIL()
    {
      VerifyCreated(m_regionNames[0], m_keys[1]);
      VerifyCreated(m_regionNames[1], m_keys[3]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);

      UpdateEntry(m_regionNames[0], m_keys[0], m_nvals[0], false);
      UpdateEntry(m_regionNames[1], m_keys[2], m_nvals[2], false);
    }

    public void StepFiveRegex()
    {
      CreateEntry(m_regionNames[0], m_keys[2], m_vals[2]);
      CreateEntry(m_regionNames[1], m_keys[3], m_vals[3]);
    }

    public void CreateAllEntries(string regionName)
    {
      CreateEntry(regionName, m_keys[0], m_vals[0]);
      CreateEntry(regionName, m_keys[1], m_vals[1]);
      CreateEntry(regionName, m_keys[2], m_vals[2]);
      CreateEntry(regionName, m_keys[3], m_vals[3]);
    }

    public void VerifyAllEntries(string regionName, bool newVal, bool checkVal)
    {
      string[] vals = newVal ? m_nvals : m_vals;
      VerifyEntry(regionName, m_keys[0], vals[0], checkVal);
      VerifyEntry(regionName, m_keys[1], vals[1], checkVal);
      VerifyEntry(regionName, m_keys[2], vals[2], checkVal);
      VerifyEntry(regionName, m_keys[3], vals[3], checkVal);
    }

    public void VerifyInvalidAll(string regionName, params string[] keys)
    {
      if (keys != null)
      {
        foreach (string key in keys)
        {
          VerifyInvalid(regionName, key);
        }
      }
    }

    public void UpdateAllEntries(string regionName, bool checkVal)
    {
      UpdateEntry(regionName, m_keys[0], m_nvals[0], checkVal);
      UpdateEntry(regionName, m_keys[1], m_nvals[1], checkVal);
      UpdateEntry(regionName, m_keys[2], m_nvals[2], checkVal);
      UpdateEntry(regionName, m_keys[3], m_nvals[3], checkVal);
    }

    public void DoNetsearchAllEntries(string regionName, bool newVal,
      bool checkNoKey)
    {
      string[] vals;
      if (newVal)
      {
        vals = m_nvals;
      }
      else
      {
        vals = m_vals;
      }
      DoNetsearch(regionName, m_keys[0], vals[0], checkNoKey);
      DoNetsearch(regionName, m_keys[1], vals[1], checkNoKey);
      DoNetsearch(regionName, m_keys[2], vals[2], checkNoKey);
      DoNetsearch(regionName, m_keys[3], vals[3], checkNoKey);
    }

    public void StepFiveFailoverRegex()
    {
      UpdateEntry(m_regionNames[0], m_keys[0], m_nvals[0], false);
      UpdateEntry(m_regionNames[1], m_keys[2], m_nvals[2], false);
      VerifyEntry(m_regionNames[1], m_keys[1], m_vals[1], false);
    }

    public void StepSixIL()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
      IRegion<object, object> region0 = CacheHelper.GetRegion<object, object>(m_regionNames[0]);
      IRegion<object, object> region1 = CacheHelper.GetRegion<object, object>(m_regionNames[1]);
      region0.Remove(m_keys[1]);
      region1.Remove(m_keys[3]);
    }

    public void StepSixRegex()
    {
      CreateEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      CreateEntry(m_regionNames[1], m_keys[1], m_vals[1]);
      VerifyEntry(m_regionNames[0], m_keys[2], m_vals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);

      UnregisterRegexes(null, m_regexes[3]);
    }

    public void StepSixFailoverRegex()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0], false);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2], false);
      UpdateEntry(m_regionNames[1], m_keys[1], m_nvals[1], false);
    }

    public void StepSevenIL()
    {
      VerifyDestroyed(m_regionNames[0], m_keys[1]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);
    }

    public void StepSevenRegex()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[1], m_keys[1], m_vals[1]);
      UpdateEntry(m_regionNames[0], m_keys[2], m_nvals[2], true);
      UpdateEntry(m_regionNames[1], m_keys[3], m_nvals[3], true);

      UnregisterRegexes(null, m_regexes[1]);
    }

    public void StepSevenRegex2()
    {
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      VerifyEntry(m_regionNames[0], m_keys[2], m_vals[2]);

      DoNetsearch(m_regionNames[0], m_keys[0], m_vals[0], true);
      DoNetsearch(m_regionNames[0], m_keys[3], m_vals[3], true);

      UpdateAllEntries(m_regionNames[1], true);
    }

    public void StepSevenInterestResultPolicyInv()
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region.GetSubscriptionService().RegisterRegex(m_regex23);

      VerifyInvalidAll(m_regionNames[0], m_keys[1], m_keys[2]);
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0], true);
      VerifyEntry(m_regionNames[0], m_keys[3], m_vals[3], true);
    }

    public void StepSevenFailoverRegex()
    {
      UpdateEntry(m_regionNames[0], m_keys[0], m_vals[0], true);
      UpdateEntry(m_regionNames[1], m_keys[2], m_vals[2], true);
      VerifyEntry(m_regionNames[1], m_keys[1], m_nvals[1]);
    }

    public void StepEightIL()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_nvals[2]);
    }

    public void StepEightRegex()
    {
      VerifyEntry(m_regionNames[0], m_keys[2], m_nvals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);
      UpdateEntry(m_regionNames[0], m_keys[0], m_nvals[0], true);
      UpdateEntry(m_regionNames[1], m_keys[1], m_nvals[1], true);
    }

    public void StepEightInterestResultPolicyInv()
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region.GetSubscriptionService().RegisterAllKeys();

      VerifyInvalidAll(m_regionNames[1], m_keys[0], m_keys[1],
      m_keys[2], m_keys[3]);
      UpdateAllEntries(m_regionNames[0], true);
    }

    public void StepEightFailoverRegex()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
    }

    public void StepNineRegex()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[1], m_keys[1], m_vals[1]);
    }

    public void StepNineRegex2()
    {
      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_nvals[1]);
      VerifyEntry(m_regionNames[0], m_keys[2], m_nvals[2]);
      VerifyEntry(m_regionNames[0], m_keys[3], m_vals[3]);
    }

    public void StepNineInterestResultPolicyInv()
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region.GetSubscriptionService().UnregisterRegex(m_regex23);
      List<Object> keys = new List<Object>();
      keys.Add(m_keys[0]);
      keys.Add(m_keys[1]);
      keys.Add(m_keys[2]);  
      region.GetSubscriptionService().RegisterKeys(keys);

      VerifyInvalidAll(m_regionNames[0], m_keys[0], m_keys[1], m_keys[2]);
    }

    public void PutUnicodeKeys(string regionName, bool updates)
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(regionName);
      string key;
      object val;
      for (int index = 0; index < m_numUnicodeStrings; ++index)
      {
        key = GetUnicodeString(index);
        if (updates)
        {
          val = index + 100;
        }
        else
        {
          val = (float)index + 20.0F;
        }
        region[key] = val;
      }
    }

    public void RegisterUnicodeKeys(string regionName)
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(regionName);
      string[] keys = new string[m_numUnicodeStrings];
      for (int index = 0; index < m_numUnicodeStrings; ++index)
      {
        keys[m_numUnicodeStrings - index - 1] = GetUnicodeString(index);
      }
      region.GetSubscriptionService().RegisterKeys(keys);
    }

    public void VerifyUnicodeKeys(string regionName, bool updates)
    {
      IRegion<object, object> region = CacheHelper.GetVerifyRegion<object, object>(regionName);
      string key;
      object expectedVal;
      for (int index = 0; index < m_numUnicodeStrings; ++index)
      {
        key = GetUnicodeString(index);
        if (updates)
        {
          expectedVal = index + 100;
          Assert.AreEqual(expectedVal, region.GetEntry(key).Value,
            "Got unexpected value");
        }
        else
        {
          expectedVal = (float)index + 20.0F;
          Assert.AreEqual(expectedVal, region[key],
            "Got unexpected value");
        }
      }
    }

    public void CreateRegionsInterestNotify_Pool(string[] regionNames,
      string locators, string poolName, bool notify, string nbs)
    {
      Properties<string, string> props = Properties<string, string>.Create<string, string>();
      //props.Insert("notify-by-subscription-override", nbs);
      CacheHelper.InitConfig(props);
      CacheHelper.CreateTCRegion_Pool(regionNames[0], true, true,
        new TallyListener<object, object>(), locators, poolName, notify);
      CacheHelper.CreateTCRegion_Pool(regionNames[1], true, true,
        new TallyListener<object, object>(), locators, poolName, notify);
      CacheHelper.CreateTCRegion_Pool(regionNames[2], true, true,
        new TallyListener<object, object>(), locators, poolName, notify);
    }

    /*
    public void CreateRegionsInterestNotify(string[] regionNames,
      string endpoints, bool notify, string nbs)
    {
      Properties props = Properties.Create();
      //props.Insert("notify-by-subscription-override", nbs);
      CacheHelper.InitConfig(props);
      CacheHelper.CreateTCRegion(regionNames[0], true, false,
        new TallyListener(), endpoints, notify);
      CacheHelper.CreateTCRegion(regionNames[1], true, false,
        new TallyListener(), endpoints, notify);
      CacheHelper.CreateTCRegion(regionNames[2], true, false,
        new TallyListener(), endpoints, notify);
    }
     * */

    public void DoFeed()
    {
      foreach (string regionName in RegionNamesForInterestNotify)
      {
        IRegion<object, object> region = CacheHelper.GetRegion<object, object>(regionName);
        foreach (string key in m_keysNonRegex)
        {
          region[key] = "00";
        }
        foreach (string key in m_keysForRegex)
        {
          region[key] = "00";
        }
      }
    }

    public void DoFeederOps()
    {
      foreach (string regionName in RegionNamesForInterestNotify)
      {
        IRegion<object, object> region = CacheHelper.GetRegion<object, object>(regionName);
        foreach (string key in m_keysNonRegex)
        {
          region[key] = "11";
          region[key] = "22";
          region[key] = "33";
          region.GetLocalView().Invalidate(key);
          region.Remove(key);
        }
        foreach (string key in m_keysForRegex)
        {
          region[key] = "11";
          region[key] = "22";
          region[key] = "33";
          region.GetLocalView().Invalidate(key);
          region.Remove(key);
        }
      }
    }

    public void DoRegister()
    {
      DoRegisterInterests(RegionNamesForInterestNotify[0], true);
      DoRegisterInterests(RegionNamesForInterestNotify[1], false);
      // We intentionally do not register interest in Region3
      //DoRegisterInterestsBlah(RegionNamesForInterestNotifyBlah[2]);
    }

    public void DoRegisterInterests(string regionName, bool receiveValues)
    {
      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(regionName);
      List<string> keys = new List<string>();
      foreach (string key in m_keysNonRegex)
      {
        keys.Add(key);
      }
      region.GetSubscriptionService().RegisterKeys(keys.ToArray(), false, false, receiveValues);
      region.GetSubscriptionService().RegisterRegex("key-regex.*", false, false, receiveValues);
    }

    public void DoUnregister()
    {
      DoUnregisterInterests(RegionNamesForInterestNotify[0]);
      DoUnregisterInterests(RegionNamesForInterestNotify[1]);
    }

    public void DoUnregisterInterests(string regionName)
    {
      List<string> keys = new List<string>();
      foreach (string key in m_keysNonRegex)
      {
        keys.Add(key);
      }
      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(regionName);
      region.GetSubscriptionService().UnregisterKeys(keys.ToArray());
      region.GetSubscriptionService().UnregisterRegex("key-regex.*");
    }

    public void DoValidation(string clientName, string regionName,
      int creates, int updates, int invalidates, int destroys)
    {
      IRegion<object, object> region = CacheHelper.GetRegion<object, object>(regionName);
      TallyListener<object, object> listener = region.Attributes.CacheListener as TallyListener<object, object>;

      Util.Log(clientName + ": " + regionName + ": creates expected=" + creates +
        ", actual=" + listener.Creates);
      Util.Log(clientName + ": " + regionName + ": updates expected=" + updates +
        ", actual=" + listener.Updates);
      Util.Log(clientName + ": " + regionName + ": invalidates expected=" + invalidates +
        ", actual=" + listener.Invalidates);
      Util.Log(clientName + ": " + regionName + ": destroys expected=" + destroys +
        ", actual=" + listener.Destroys);

      Assert.AreEqual(creates, listener.Creates, clientName + ": " + regionName);
      Assert.AreEqual(updates, listener.Updates, clientName + ": " + regionName);
      Assert.AreEqual(invalidates, listener.Invalidates, clientName + ": " + regionName);
      Assert.AreEqual(destroys, listener.Destroys, clientName + ": " + regionName);
    }

    #endregion


    [Test]
    public void InterestList2()
    {
			CacheHelper.SetupJavaServers(true, "cacheserver_notify_subscription.xml");
			CacheHelper.StartJavaLocator(1, "GFELOC");
			Util.Log("Locator started");
			CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
			Util.Log("Cacheserver 1 started.");

			m_client1.Call(CreateTCRegions_Pool, RegionNames,
				CacheHelper.Locators, "__TESTPOOL1_", true);
			Util.Log("StepOne complete.");

			m_client2.Call(CreateTCRegions_Pool, RegionNames,
				CacheHelper.Locators, "__TESTPOOL1_", true);
			Util.Log("StepTwo complete.");

			m_client1.Call(StepThree);
			m_client1.Call(RegisterAllKeys,
				new string[] { RegionNames[0], RegionNames[1] });
			Util.Log("StepThree complete.");

			m_client2.Call(StepFour);
			m_client2.Call(RegisterAllKeys, new string[] { RegionNames[0] });
			Util.Log("StepFour complete.");

			m_client1.Call(StepFiveIL);
			m_client1.Call(UnregisterAllKeys, new string[] { RegionNames[1] });
			Util.Log("StepFive complete.");

			m_client2.Call(StepSixIL);
			Util.Log("StepSix complete.");

			m_client1.Call(StepSevenIL);
			Util.Log("StepSeven complete.");

			m_client1.Call(Close);
			m_client2.Call(Close);

			CacheHelper.StopJavaServer(1);
			Util.Log("Cacheserver 1 stopped.");

			CacheHelper.StopJavaLocator(1);
			Util.Log("Locator stopped");

			CacheHelper.ClearEndpoints();
			CacheHelper.ClearLocators();
		}

  }
}
