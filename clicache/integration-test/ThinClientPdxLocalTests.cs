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
using PdxTests;

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using DUnitFramework;
  using Client;
  using Region = IRegion<object, object>;

  [TestFixture]
  [Category("group4")]
  [Category("unicast_only")]
  [Category("generics")]
  internal class ThinClientPdxLocalTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1;

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      return new ClientBase[] {m_client1};
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

    private void cleanup()
    {
      {
        CacheHelper.SetExtraPropertiesFile(null);
        if (m_clients != null)
        {
          foreach (var client in m_clients)
          {
            try
            {
              client.Call(CacheHelper.Close);
            }
            catch (System.Runtime.Remoting.RemotingException)
            {
            }
            catch (System.Net.Sockets.SocketException)
            {
            }
          }
        }

        CacheHelper.Close();
      }
    }

    private void LocalOpsStep()
    {
      try
      {
        CacheHelper.DCache.TypeRegistry.RegisterType(PdxInsideIGeodeSerializable.CreateDeserializable, 5005);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(NestedPdx.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes1.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes2.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes3.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes4.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes5.CreateDeserializable);
        CacheHelper.DCache.TypeRegistry.RegisterPdxType(PdxTypes6.CreateDeserializable);
      }
      catch (Exception)
      {
      }

      var region0 = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var localregion = region0.GetLocalView();


      var p1 = new PdxTypes1();
      var x = "";
      localregion.Add(p1, x);
      var val = localregion[p1];
      //object val = region0[p1];
      val = localregion[p1];
      val = localregion[p1];
      Assert.IsTrue(val.Equals(x), "value should be equal");
      Assert.IsTrue(localregion.Remove(new KeyValuePair<object, object>(p1, x)),
        "Result of remove should be true, as this value null exists locally.");
      Assert.IsFalse(localregion.ContainsKey(p1), "containsKey should be false");
      try
      {
        localregion[p1] = null;
        Assert.Fail("Expected IllegalArgumentException here for put");
      }
      catch (IllegalArgumentException)
      {
        Util.Log("Got Expected IllegalArgumentException");
      }

      localregion[p1] = 1;
      localregion.Invalidate(p1);
      try
      {
        var retVal = localregion[p1];
      }
      catch (Client.KeyNotFoundException)
      {
        Util.Log("Got expected KeyNotFoundException exception");
      }

      Assert.IsFalse(localregion.Remove(new KeyValuePair<object, object>(p1, 1)),
        "Result of remove should be false, as this value does not exists locally.");
      Assert.IsTrue(localregion.ContainsKey(p1), "containsKey should be true");
      localregion[p1] = 1;
      Assert.IsTrue(localregion.Remove(p1), "Result of remove should be true, as this value exists locally.");
      Assert.IsFalse(localregion.ContainsKey(p1), "containsKey should be false");

      var p2 = new PdxTypes2();
      localregion.Add(p2, 1);
      var
        intVal1 = localregion[p2]; // local get work for pdx object as key but it wont work with caching enable. Throws KeyNotFoundException.
      Assert.IsTrue(intVal1.Equals(1), "intVal should be 1.");

      var p3 = new PdxTypes3();
      localregion.Add(p3, "testString");
      if (localregion.ContainsKey(p3))
      {
        var strVal1 = localregion[p3];
        Assert.IsTrue(strVal1.Equals("testString"), "strVal should be testString.");
      }

      try
      {
        if (localregion.ContainsKey(p3))
        {
          localregion.Add(p3, 11);
          Assert.Fail("Expected EntryExistException here");
        }
      }
      catch (EntryExistsException)
      {
        Util.Log(" Expected EntryExistsException exception thrown by localCreate");
      }

      var p4 = new PdxTypes4();
      localregion.Add(p4, p1);
      var objVal1 = localregion[p4];
      Assert.IsTrue(objVal1.Equals(p1), "valObject and objVal should match.");
      Assert.IsTrue(localregion.Remove(new KeyValuePair<object, object>(p4, p1)),
        "Result of remove should be true, as this value exists locally.");
      Assert.IsFalse(localregion.ContainsKey(p4), "containsKey should be false");
      localregion[p4] = p1;
      Assert.IsTrue(localregion.Remove(p4), "Result of remove should be true, as this value exists locally.");
      Assert.IsFalse(localregion.ContainsKey(p4), "containsKey should be false");

      var p5 = new PdxTypes5();

      //object cval = region0[p1]; //this will only work when caching is enable else throws KeyNotFoundException

      localregion.Clear();
    }

    private void runLocalOps()
    {
      CacheHelper.SetupJavaServers(true, "cacheserver.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      m_client1.Call(CreateTCRegions_Pool, RegionNames,
        CacheHelper.Locators, "__TESTPOOL1_", false, false, true /*local caching false*/);
      Util.Log("StepOne (pool locators) complete.");


      m_client1.Call(LocalOpsStep);
      Util.Log("localOps complete.");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #region Tests

    [Test]
    public void LocalOps()
    {
      runLocalOps();
    }

    #endregion
  }
}