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
using System.Threading;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using DUnitFramework;
  using Client;

  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  public class RegionRemoveTests : ThinClientRegionSteps
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

    protected virtual void RemoveAllStep1()
    {
      CreateEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      CreateEntry(m_regionNames[0], m_keys[1], m_vals[1]);
      CreateEntry(m_regionNames[0], m_keys[2], m_vals[2]);

      CreateEntry(m_regionNames[1], m_keys[3], m_vals[3]);
      CreateEntry(m_regionNames[1], m_keys[4], m_vals[4]);
      CreateEntry(m_regionNames[1], m_keys[5], m_vals[5]);
      Util.Log("RemoveAllStep1 complete.");
    }

    protected virtual void RemoveAllStep2()
    {
      IRegion<object, object> reg0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      IRegion<object, object> reg1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      ICollection<object> keys0 = new List<object>();
      ICollection<object> keys1 = new List<object>();
      for (int i = 0; i < 3; i++)
      {
        keys0.Add(m_keys[i]);
        keys1.Add(m_keys[i + 3]);
      }

      //try remove all
      reg0.RemoveAll(keys0);
      reg1.RemoveAll(keys1);
      Util.Log("RemoveAllStep2 complete.");
    }

    protected virtual void RemoveAllStep3()
    {
      VerifyDestroyed(m_regionNames[0], m_keys[0]);
      VerifyDestroyed(m_regionNames[0], m_keys[1]);
      VerifyDestroyed(m_regionNames[0], m_keys[2]);

      VerifyDestroyed(m_regionNames[1], m_keys[3]);
      VerifyDestroyed(m_regionNames[1], m_keys[4]);
      VerifyDestroyed(m_regionNames[1], m_keys[5]);

      IRegion<object, object> region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      IRegion<object, object> region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      Assert.AreEqual(region0.Count, 0, "Remove all should remove the entries specified");
      Assert.AreEqual(region1.Count, 0, "Remove all should remove the entries specified");
      Util.Log("RemoveAllStep3 complete.");
    }

    private void RunRemoveAll()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
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

      _mClient1.Call(RemoveAllStep1);
      Util.Log("RemoveAllStep1 complete.");

      _mClient1.Call(RemoveAllStep2);
      Util.Log("RemoveAllStep2 complete.");

      _mClient2.Call(RemoveAllStep3);
      Util.Log("RemoveAllStep3 complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    private void RunRemoveOps1()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver1_expiry.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _mClient1.Call(CreateTCRegions_Pool, RegionNames2,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _mClient2.Call(CreateTCRegions_Pool, RegionNames2,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _mClient2.Call(RemoveStepEight);
      Util.Log("RemoveStepEight complete.");

      _mClient1.Call(Close);
      Util.Log("Client 1 closed");
      _mClient2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void RemoveAll()
    {
      RunRemoveAll();
    }

    [Test]
    public void RemoveOps1()
    {
      RunRemoveOps1();
    }

    #endregion
  }
}
