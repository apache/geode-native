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
  internal class ThinClientNameTests : ThinClientRegionSteps
  {
    #region Private members

    private UnitProcess m_client1, m_client2;

    #endregion

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();
      return new ClientBase[] {m_client1, m_client2};
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
        CacheHelper.ClearLocators();
      }
      finally
      {
        CacheHelper.StopJavaServers();
        CacheHelper.StopJavaLocators();
      }

      base.EndTest();
    }

    void cleanup()
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

    private void DoputAndVerifyClientName()
    {
      //CacheableString cVal = new CacheableString(new string('A', 1024));
      var region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);

      var cVal = new string('A', 5);

      Util.Log("Putting key = key-0");
      region["key-0"] = cVal;
      Util.Log("Put Operation done successfully");

      //Verify the Client Name.
      var cReceivedName = region["clientName1"] as string;
      Util.Log(" DoputAndVerifyClientName Received Client Name = {0} ", cReceivedName);
      Assert.AreEqual(cReceivedName.Equals("Client-1"), true, "Did not find the expected value.");
    }

    private void DoGetAndVerifyClientName()
    {
      var region = CacheHelper.GetVerifyRegion<object, object>(m_regionNames[0]);
      var cReceivedName = region["clientName2"] as string;

      Util.Log("Get Operation done successfully");

      //Verify the Client Name.
      Util.Log(" DoGetAndVerifyClientName Received Client Name = {0} ", cReceivedName);
      Assert.AreEqual(cReceivedName.Equals("Client-2"), true, "Did not find the expected value.");
    }

    private void ConfigClient1AndCreateRegions_Pool(string[] regionNames,
      string locators, string poolName, bool clientNotification, bool ssl, bool cachingEnable)
    {
      //Configure Client "name" for Client-1
      var props = Properties<string, string>.Create();
      props.Insert("name", "Client-1");
      CacheHelper.InitConfig(props);

      CacheHelper.CreateTCRegion_Pool<object, object>(regionNames[0], true, cachingEnable,
        null, locators, poolName, clientNotification, ssl, false);
      CacheHelper.CreateTCRegion_Pool<object, object>(regionNames[1], false, cachingEnable,
        null, locators, poolName, clientNotification, ssl, false);
      m_regionNames = regionNames;
    }

    private void ConfigClient2AndCreateRegions_Pool(string[] regionNames,
      string locators, string poolName, bool clientNotification, bool ssl, bool cachingEnable)
    {
      //Configure Client "name" for Client-2
      var props = Properties<string, string>.Create();
      props.Insert("name", "Client-2");
      CacheHelper.InitConfig(props);

      CacheHelper.CreateTCRegion_Pool<object, object>(regionNames[0], true, cachingEnable,
        null, locators, poolName, clientNotification, ssl, false);
      CacheHelper.CreateTCRegion_Pool<object, object>(regionNames[1], false, cachingEnable,
        null, locators, poolName, clientNotification, ssl, false);
      m_regionNames = regionNames;
    }

    #region Tests

    [Test]
    //NON PDX UnitTest for Ticket#866 on NC OR SR#13306117704. Set client name via native client API
    public void UserCanSetClientName()
    {
      CacheHelper.SetupJavaServers(true, "cacheserverPdx.xml");
      CacheHelper.StartJavaLocator(1, "GFELOC");
      Util.Log("Locator 1 started.");
      CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);
      Util.Log("Cacheserver 1 started.");

      //Client Notification and local caching enabled for clients
      m_client1.Call(ConfigClient1AndCreateRegions_Pool, RegionNames, CacheHelper.Locators, "__TESTPOOL1_", true, false,
        true);
      Util.Log("StepOne (pool locators) complete.");

      m_client2.Call(ConfigClient2AndCreateRegions_Pool, RegionNames, CacheHelper.Locators, "__TESTPOOL1_", true, false,
        true);
      Util.Log("StepTwo (pool locators) complete.");

      m_client1.Call(DoputAndVerifyClientName);
      Util.Log("StepThree complete.");

      m_client2.Call(DoGetAndVerifyClientName);
      Util.Log("StepFour complete.");

      m_client1.Call(Close);
      Util.Log("Client 1 closed");
      m_client2.Call(Close);
      Util.Log("Client 2 closed");

      CacheHelper.StopJavaServer(1);
      Util.Log("Cacheserver 1 stopped.");

      CacheHelper.StopJavaLocator(1);
      Util.Log("Locator 1 stopped.");

      CacheHelper.ClearEndpoints();
      CacheHelper.ClearLocators();
    }

    #endregion
  }
}