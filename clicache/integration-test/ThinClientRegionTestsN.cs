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
using System.Text;
using System.Threading;
using Apache.Geode.DUnitFramework;
using NUnit.Framework;

namespace Apache.Geode.Client.UnitTests
{
  [Serializable]
  public class CustomPartitionResolver<TValue> : IPartitionResolver<int, TValue>
  {
    public string GetName()
    {
      return "CustomPartitionResolver";
    }

    public object GetRoutingObject(EntryEvent<int, TValue> key)
    {
      Util.Log("CustomPartitionResolver::GetRoutingObject");
      return key.Key + 5;
    }

    public static CustomPartitionResolver<TValue> Create()
    {
      return new CustomPartitionResolver<TValue>();
    }
  }

  [Serializable]
  public class CustomPartitionResolver1<TValue> : IFixedPartitionResolver<int, TValue>
  {
    public string GetName()
    {
      return "CustomPartitionResolver1";
    }

    public object GetRoutingObject(EntryEvent<int, TValue> key)
    {
      Util.Log("CustomPartitionResolver1::GetRoutingObject");
      var nkey = key.Key;

      return nkey + 5;
    }

    public static CustomPartitionResolver1<TValue> Create()
    {
      return new CustomPartitionResolver1<TValue>();
    }

    public string GetPartitionName(EntryEvent<int, TValue> entryEvent)
    {
      Util.Log("CustomPartitionResolver1::GetPartitionName");
      var newkey = entryEvent.Key % 6;
      if (newkey == 0)
      {
        return "P1";
      }

      if (newkey == 1)
      {
        return "P2";
      }

      if (newkey == 2)
      {
        return "P3";
      }

      if (newkey == 3)
      {
        return "P4";
      }

      if (newkey == 4)
      {
        return "P5";
      }

      if (newkey == 5)
      {
        return "P6";
      }

      return "Invalid";
    }
  }

  [Serializable]
  public class CustomPartitionResolver2<TValue> : IFixedPartitionResolver<int, TValue>
  {
    public string GetName()
    {
      return "CustomPartitionResolver2";
    }

    public object GetRoutingObject(EntryEvent<int, TValue> key)
    {
      Util.Log("CustomPartitionResolver2::GetRoutingObject");
      return key.Key + 4;
    }

    public static CustomPartitionResolver2<TValue> Create()
    {
      return new CustomPartitionResolver2<TValue>();
    }

    public string GetPartitionName(EntryEvent<int, TValue> entryEvent)
    {
      Util.Log("CustomPartitionResolver2::GetPartitionName");
      var key = entryEvent.Key.ToString();
      var numKey = Convert.ToInt32(key);
      var newkey = numKey % 6;
      if (newkey == 0)
      {
        return "P1";
      }

      if (newkey == 1)
      {
        return "P2";
      }

      if (newkey == 2)
      {
        return "P3";
      }

      if (newkey == 3)
      {
        return "P4";
      }

      if (newkey == 4)
      {
        return "P5";
      }

      if (newkey == 5)
      {
        return "P6";
      }

      return "Invalid";
    }
  }

  [Serializable]
  public class CustomPartitionResolver3<TValue> : IFixedPartitionResolver<int, TValue>
  {
    public string GetName()
    {
      return "CustomPartitionResolver3";
    }

    public object GetRoutingObject(EntryEvent<int, TValue> key)
    {
      Util.Log("CustomPartitionResolver3::GetRoutingObject");
      return key.Key % 5;
    }

    public static CustomPartitionResolver3<TValue> Create()
    {
      return new CustomPartitionResolver3<TValue>();
    }

    public string GetPartitionName(EntryEvent<int, TValue> entryEvent)
    {
      Util.Log("CustomPartitionResolver3::GetPartitionName");
      var key = entryEvent.Key.ToString();
      var numKey = Convert.ToInt32(key);
      var newkey = numKey % 3;
      if (newkey == 0)
      {
        return "P1";
      }

      if (newkey == 1)
      {
        return "P2";
      }

      if (newkey == 2)
      {
        return "P3";
      }

      return "Invalid";
    }
  }

  public class TradeKey : ICacheableKey
  {
    public int MId;
    public int MAccountid;

    public TradeKey()
    {
    }

    public TradeKey(int id)
    {
      MId = id;
      MAccountid = 1 + id;
    }

    public TradeKey(int id, int accId)
    {
      MId = id;
      MAccountid = accId;
    }

    public void FromData(DataInput input)
    {
      MId = input.ReadInt32();
      MAccountid = input.ReadInt32();
    }

    public void ToData(DataOutput output)
    {
      output.WriteInt32(MId);
      output.WriteInt32(MAccountid);
    }


    public ulong ObjectSize
    {
      get
      {
        uint objectSize = 0;
        objectSize += sizeof(int);
        objectSize += sizeof(int);
        return objectSize;
      }
    }

    public static ISerializable CreateDeserializable()
    {
      return new TradeKey();
    }

    public bool Equals(ICacheableKey other)
    {
      if (other == null)
        return false;
      var bc = other as TradeKey;
      if (bc == null)
        return false;

      if (bc == this)
        return true;

      if (bc.MId == MId)
      {
        return true;
      }
      return false;
    }

    public override int GetHashCode()
    {
      return base.GetHashCode();
    }
  }

  [Serializable]
  public class TradeKeyResolver : IPartitionResolver<TradeKey, object>
  {
    public string GetName()
    {
      return "TradeKeyResolver";
    }

    public object GetRoutingObject(EntryEvent<TradeKey, object> key)
    {
      Util.Log("TradeKeyResolver::GetRoutingObject");
      var tkey = key.Key;
      Util.Log("TradeKeyResolver::GetRoutingObject done {0} ", tkey.MId + 5);
      return tkey.MId + 5;
    }

    public static TradeKeyResolver Create()
    {
      return new TradeKeyResolver();
    }
  }

  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  public class ThinClientRegionTests : ThinClientRegionSteps
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

    private void EmptyByteArrayTest()
    {
      var region = CacheHelper.GetVerifyRegion<int, byte[]>(RegionNames3[0]);

      var regServ = region.RegionService;

      var cache = regServ as Cache;

      Assert.IsNotNull(cache);

      region[0] = new byte[0];
      Util.Log("Put empty byteArray in region");
      Assert.AreEqual(0, region[0].Length);

      region[1] = new byte[2];
      Util.Log("Put non empty byteArray in region");

      Assert.AreEqual(2, region[1].Length);

      region[2] = Encoding.ASCII.GetBytes("TestString");
      Util.Log("Put string in region");

      Assert.AreEqual(0, "TestString".CompareTo(Encoding.ASCII.GetString(region[2])));
      Util.Log("EmptyByteArrayTest completed successfully");
    }

    private void CheckAndPutKey()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var regServ = region0.RegionService;

      var cache = regServ as Cache;

      Assert.IsNotNull(cache);

      if (region0.ContainsKey("keyKey01"))
      {
        Assert.Fail("Did not expect keyKey01 to be on Server");
      }
      region0["keyKey01"] = "valueValue01";
      if (!region0.ContainsKey("keyKey01"))
      {
        Assert.Fail("Expected keyKey01 to be on Server");
      }
    }

    private void GetInterests()
    {
      string[] testregex = { "Key-*1", "Key-*2", "Key-*3", "Key-*4", "Key-*5", "Key-*6" };
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0.GetSubscriptionService().RegisterRegex(testregex[0]);
      region0.GetSubscriptionService().RegisterRegex(testregex[1]);
      ICollection<object> myCollection1 = new Collection<object>();
      myCollection1.Add(m_keys[0]);
      region0.GetSubscriptionService().RegisterKeys(myCollection1);

      ICollection<object> myCollection2 = new Collection<object>();
      myCollection2.Add(m_keys[1]);
      region0.GetSubscriptionService().RegisterKeys(myCollection2);

      var regvCol = region0.GetSubscriptionService().GetInterestListRegex();
      var regv = new string[regvCol.Count];
      regvCol.CopyTo(regv, 0);

      if (regv.Length != 2)
      {
        Assert.Fail("regex list length is not 2");
      }
      for (var i = 0; i < regv.Length; i++)
      {
        Util.Log("regv[{0}]={1}", i, regv[i]);
        var found = false;
        for (var j = 0; j < regv.Length; j++)
        {
          if (regv[i].Equals(testregex[j]))
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          Assert.Fail("Unexpected regex");
        }
      }

      var keyvCol = region0.GetSubscriptionService().GetInterestList();
      var keyv = new string[keyvCol.Count];
      keyvCol.CopyTo(keyv, 0);

      if (keyv.Length != 2)
      {
        Assert.Fail("interest list length is not 2");
      }
      for (var i = 0; i < keyv.Length; i++)
      {
        Util.Log("keyv[{0}]={1}", i, keyv[i]);
        var found = false;
        for (var j = 0; j < keyv.Length; j++)
        {
          if (keyv[i].Equals(m_keys[j]))
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          Assert.Fail("Unexpected key");
        }
      }
    }

    private void RunDistOps()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateNonExistentRegion, CacheHelper.Locators);
      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _client1.Call(StepThree);
      Util.Log("StepThree complete.");

      _client2.Call(StepFour);
      Util.Log("StepFour complete.");

      _client1.Call(CheckServerKeys);
      _client1.Call(StepFive, true);
      Util.Log("StepFive complete.");

      _client2.Call(StepSix, true);
      Util.Log("StepSix complete.");

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

    private void RunDistOps2()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml", "cacheserver2.xml", "cacheserver3.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepOne (pool locators) complete.");

      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false);
      Util.Log("StepTwo (pool locators) complete.");

      _client1.Call(StepThree);
      Util.Log("StepThree complete.");

      _client2.Call(StepFour);
      Util.Log("StepFour complete.");

      _client1.Call(StepFive, true);
      Util.Log("StepFive complete.");

      _client2.Call(StepSix, true);
      Util.Log("StepSix complete.");
      //m_client1.Call(GetAll, pool, locator);

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

    private void RunNotification()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepOne (pool locators) complete.");

      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepTwo (pool locators) complete.");

      _client1.Call(RegisterAllKeysR0WithoutValues);
      _client1.Call(RegisterAllKeysR1WithoutValues);

      _client2.Call(RegisterAllKeysR0WithoutValues);
      _client2.Call(RegisterAllKeysR1WithoutValues);

      _client1.Call(StepThree);
      Util.Log("StepThree complete.");

      _client2.Call(StepFour);
      Util.Log("StepFour complete.");

      _client1.Call(StepFive, true);
      Util.Log("StepFive complete.");

      _client2.Call(StepSixNotify, false);
      Util.Log("StepSix complete.");

      _client1.Call(StepSevenNotify, false);
      Util.Log("StepSeven complete.");

      _client2.Call(StepEightNotify, false);
      Util.Log("StepEight complete.");

      _client1.Call(StepNineNotify, false);
      Util.Log("StepNine complete.");

      _client2.Call(StepTen);
      Util.Log("StepTen complete.");

      _client1.Call(StepEleven);
      Util.Log("StepEleven complete.");

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

    private void RunPartitionResolver()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver1_pr.xml",
          "cacheserver2_pr.xml", "cacheserver3_pr.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      var putGetTest = new PutGetTests();
      // Create and Add partition resolver to the regions.
      //CustomPartitionResolver<object> cpr = CustomPartitionResolver<object>.Create();

      _client1.Call(CreateTCRegions_Pool2_WithPartitionResolver, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, true);
      Util.Log("Client 1 (pool locators) regions created");
      _client2.Call(CreateTCRegions_Pool2_WithPartitionResolver, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, true);
      Util.Log("Client 2 (pool locators) regions created");

      _client1.Call(putGetTest.SetRegion, RegionNames[1]);
      _client2.Call(putGetTest.SetRegion, RegionNames[1]);
      putGetTest.DoPRSHPartitionResolverTasks(_client1, _client2, RegionNames[1]);

      _client1.Call(putGetTest.SetRegion, RegionNames[0]);
      _client2.Call(putGetTest.SetRegion, RegionNames[0]);
      putGetTest.DoPRSHPartitionResolverTasks(_client1, _client2, RegionNames[0]);

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

    private void RunTradeKeyResolver()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver1_TradeKey.xml",
        "cacheserver2_TradeKey.xml", "cacheserver3_TradeKey.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");
      CacheHelper.StartJavaServerWithLocators(3, "GFECS3", 1);
      Util.Log("Cacheserver 3 started.");

      _client1.Call(CreateTCRegion2, TradeKeyRegion, true, true, TradeKeyResolver.Create(),
        CacheHelper.Locators, true);
      Util.Log("Client 1 (pool locators) region created");

      var putGetTest = new PutGetTests();
      _client1.Call(putGetTest.DoPRSHTradeResolverTasks, TradeKeyRegion);

      _client1.Call(Close);
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

    private void ClearRegionListenersStep1()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0.GetSubscriptionService().RegisterAllKeys();
      var attrMutator = region0.AttributesMutator;
      var listener = new TallyListener<object, object>();
      attrMutator.SetCacheListener(listener);
      var writer = new TallyWriter<object, object>();
      attrMutator.SetCacheWriter(writer);
    }

    private void ClearRegionStep1()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      region0.GetSubscriptionService().RegisterAllKeys();
      CreateEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      CreateEntry(m_regionNames[0], m_keys[1], m_nvals[1]);
      if (region0.Count != 2)
      {
        Assert.Fail("Expected region size 2");
      }
    }

    private void ClearRegionListenersStep2()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      if (region0.Count != 2)
      {
        Assert.Fail("Expected region size 2");
      }
    }

    private void ClearRegionStep2()
    {
      //Console.WriteLine("IRegion<object, object> Name = {0}", m_regionNames[0]);

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      if (region0 == null)
      {
        Console.WriteLine("Region0 is Null");
      }
      else
      {
        //Console.WriteLine("NIL:Before clear call");
        region0.Clear();
        //Console.WriteLine("NIL:After clear call");

        if (region0.Count != 0)
        {
          Assert.Fail("Expected region size 0");
        }
      }
      Thread.Sleep(20000);
    }

    private void ClearRegionListenersStep3()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      if (region0.Count != 0)
      {
        Assert.Fail("Expected region size 0");
      }
      var attr = region0.Attributes;
      var listener = attr.CacheListener as TallyListener<object, object>;
      var writer = attr.CacheWriter as TallyWriter<object, object>;
      if (listener.Clears != 1)
      {
        Assert.Fail("Expected listener clear count 1");
      }
      if (writer.Clears != 1)
      {
        Assert.Fail("Expected writer clear count 1");
      }
      CreateEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      CreateEntry(m_regionNames[0], m_keys[1], m_nvals[1]);
      if (region0.Count != 2)
      {
        Assert.Fail("Expected region size 2");
      }
      region0.GetLocalView().Clear();
      if (listener.Clears != 2)
      {
        Assert.Fail("Expected listener clear count 2");
      }
      if (writer.Clears != 2)
      {
        Assert.Fail("Expected writer clear count 2");
      }
      if (region0.Count != 0)
      {
        Assert.Fail("Expected region size 0");
      }
    }

    private void ClearRegionStep3()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      if (region0.Count != 2)
      {
        Assert.Fail("Expected region size 2");
      }

      if (!region0.ContainsKey(m_keys[0]))
      {
        Assert.Fail("m_key[0] is not on Server");
      }
      if (!region0.ContainsKey(m_keys[1]))
      {
        Assert.Fail("m_key[1] is not on Server");
      }
    }

    private void PutAllStep3()
    {
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var resultKeys = new List<object>();
      object key0 = m_keys[0];
      object key1 = m_keys[1];
      resultKeys.Add(key0);
      resultKeys.Add(key1);
      region0.GetSubscriptionService().RegisterKeys(resultKeys.ToArray());
      Util.Log("Step three completes");
    }

    private void PutAllStep4()
    {
      var map0 = new Dictionary<object, object>();
      var map1 = new Dictionary<object, object>();
      object key0 = m_keys[0];
      object key1 = m_keys[1];
      var val0 = m_vals[0];
      var val1 = m_vals[1];
      map0.Add(key0, val0);
      map0.Add(key1, val1);

      object key2 = m_keys[2];
      object key3 = m_keys[3];
      var val2 = m_vals[2];
      var val3 = m_vals[3];
      map1.Add(key2, val2);
      map1.Add(key3, val3);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region0.PutAll(map0);
      region1.PutAll(map1);
      Util.Log("Put All Complets");
    }

    private void PutAllStep5()
    {
      VerifyCreated(m_regionNames[0], m_keys[0]);
      VerifyCreated(m_regionNames[0], m_keys[1]);

      VerifyEntry(m_regionNames[0], m_keys[0], m_vals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_vals[1]);

      DoNetsearch(m_regionNames[1], m_keys[2], m_vals[2], true);
      DoNetsearch(m_regionNames[1], m_keys[3], m_vals[3], true);

      Util.Log("StepFive complete.");
    }

    private void PutAllStep6()
    {
      var map0 = new Dictionary<object, object>();
      var map1 = new Dictionary<object, object>();

      object key0 = m_keys[0];
      object key1 = m_keys[1];
      var val0 = m_nvals[0];
      var val1 = m_nvals[1];
      map0.Add(key0, val0);
      map0.Add(key1, val1);

      object key2 = m_keys[2];
      object key3 = m_keys[3];
      var val2 = m_nvals[2];
      var val3 = m_nvals[3];
      map1.Add(key2, val2);
      map1.Add(key3, val3);
      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var region1 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[1]);
      region0.PutAll(map0);
      region1.PutAll(map1);
      Util.Log("Step6 Complets");
    }

    private void PutAllStep7() //client 1
    {
      //Region0 is changed at client 1 because keys[0] and keys[1] were registered.PutAllStep3
      VerifyEntry(m_regionNames[0], m_keys[0], m_nvals[0]);
      VerifyEntry(m_regionNames[0], m_keys[1], m_nvals[1]);
      // region1 is not changed at client beacuse no regsiter interest.
      VerifyEntry(m_regionNames[1], m_keys[2], m_vals[2]);
      VerifyEntry(m_regionNames[1], m_keys[3], m_vals[3]);
      Util.Log("PutAllStep7 complete.");
    }

    private void RunPutAll()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver_notify_subscription.xml",
        "cacheserver_notify_subscription2.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      CacheHelper.StartJavaServerWithLocators(2, "GFECS2", 1);
      Util.Log("Cacheserver 2 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);  //Client Notification true for client 1
      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);  //Cleint notification true for client 2

      Util.Log("IRegion<object, object> creation complete.");

      _client1.Call(PutAllStep3);
      Util.Log("PutAllStep3 complete.");

      _client2.Call(PutAllStep4);
      Util.Log("PutAllStep4 complete.");

      _client1.Call(PutAllStep5);
      Util.Log("PutAllStep5 complete.");

      _client2.Call(PutAllStep6);
      Util.Log("PutAllStep6 complete.");

      _client1.Call(PutAllStep7);
      Util.Log("PutAllStep7 complete.");

      _client1.Call(Close);
      Util.Log("Client 1 closed");
      _client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaServer(2);
      Util.Log("Cacheserver 2 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void PutEmptyByteArrayTest()
    {
      CacheHelper.SetupJavaServers("cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");
      _client1.Call(CreateTCRegions_Pool, RegionNames3,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepOne of region creation complete.");

      _client1.Call(EmptyByteArrayTest);
      Util.Log("EmptyByteArrayTest completed.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      CacheHelper.StopJavaLocator(1);
    }

    [Test]
    public void CheckKeyOnServer()
    {
      CacheHelper.SetupJavaServers("cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("StepOne of region creation complete.");

      _client1.Call(CheckAndPutKey);
      Util.Log("Check for ContainsKeyOnServer complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      CacheHelper.StopJavaLocator(1);
    }

    [Test]
    public void GetInterestsOnClient()
    {
      CacheHelper.SetupJavaServers("cacheserver_notify_subscription.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);


      Util.Log("StepOne of region creation complete.");

      _client1.Call(GetInterests);
      Util.Log("StepTwo of check for GetInterestsOnClient complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      CacheHelper.StopJavaLocator(1);
    }

    [Test]
    public void DistOps()
    {
      RunDistOps();
    }

    [Test]
    public void DistOps2()
    {
      RunDistOps2();
    }

    [Test]
    public void Notification()
    {
      RunNotification();
    }

    [Test]
    public void CheckPartitionResolver()
    {
      RunPartitionResolver();
    }

    [Test]
    public void TradeKeyPartitionResolver()
    {
      RunTradeKeyResolver();
    }

    [Test]
    public void RegionClearTest()
    {
      CacheHelper.SetupJavaServers("cacheserver_notify_subscription.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      _client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("client 1 StepOne of region creation complete.");
      _client2.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", true);
      Util.Log("client 2 StepOne of region creation complete.");

      _client1.Call(ClearRegionListenersStep1);
      _client2.Call(ClearRegionStep1);
      _client1.Call(ClearRegionListenersStep2);
      _client2.Call(ClearRegionStep2);
      _client1.Call(ClearRegionListenersStep3);
      _client2.Call(ClearRegionStep3);
      Util.Log("StepTwo of check for RegionClearTest complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");
      CacheHelper.StopJavaLocator(1);
    }

    [Test]
    public void PutAll()
    {
      RunPutAll();
    }

    #endregion
  }
}
