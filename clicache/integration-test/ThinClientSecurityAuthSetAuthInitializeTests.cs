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
	using Apache.Geode.Client.Tests;
	using Apache.Geode.DUnitFramework;
	using Apache.Geode.Client;

	[TestFixture]
	[Category("group1")]
	[Category("unicast_only")]
	[Category("generics")]
	public class ThinClientSecurityAuthSetAuthInitializeTests : ThinClientRegionSteps
	{
		#region Private members

		private UnitProcess m_client1;
		private const string CacheXml1 = "cacheserver_notify_subscription.xml";

		private UsernamePasswordAuthInitialize authInitialize = new UsernamePasswordAuthInitialize();

		#endregion

		protected override ClientBase[] GetClients()
		{
			m_client1 = new UnitProcess();
			return new ClientBase[] { m_client1 };
		}

		[TearDown]
		public override void EndTest()
		{
			try {
				m_client1.Call(CacheHelper.Close);
				CacheHelper.ClearEndpoints();
				CacheHelper.ClearLocators();
			} finally {
				CacheHelper.StopJavaServers();
			}
			base.EndTest();
		}


		public void CreateClient(string regionName, string locators)
		{
			CacheHelper.Close();
			Properties<string, string> sysProps = new Properties<string, string>();
			sysProps.Insert("security-username", "root");
			sysProps.Insert("security-password", "root");

			CacheHelper.InitConfig(sysProps, authInitialize);

			Assert.IsFalse(authInitialize.called);

			CacheHelper.CreatePool<object, object>("__TESTPOOL1_", locators, (string)null,
				0, true, -1, false);

			CacheHelper.CreateTCRegion_Pool<object, object>(regionName, true, true,
				null, locators, "__TESTPOOL1_", true);
		}

		public void AssertAuthInitializeCalled(bool called)
		{
			Assert.AreEqual(called, authInitialize.called);
		}

		void runValidCredentials()
		{
			var dataDir = Util.GetEnvironmentVariable("TESTSRC");
			Properties<string, string> javaProps = new Properties<string, string>();
			javaProps.Insert("gemfire.security-authz-xml-uri", dataDir + "\\..\\..\\templates\\security\\authz-dummy.xml");
			string authenticator = "javaobject.DummyAuthenticator.create";

			// Start the server
			CacheHelper.SetupJavaServers(true, CacheXml1);
			CacheHelper.StartJavaLocator(1, "GFELOC");
			Util.Log("Locator started");
			CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1, SecurityTestUtil.GetServerArgs(
				authenticator, null, javaProps));
			Util.Log("Cacheserver 1 started.");


			m_client1.Call(CreateClient, RegionName, CacheHelper.Locators);

			// Perform some put operations from client1
			//m_client1.Call(AssertAuthInitializeCalled, false);
			m_client1.Call(DoPuts, 4);
			m_client1.Call(AssertAuthInitializeCalled, true);

			// Verify that the puts succeeded
			m_client1.Call(DoGets, 4);

			m_client1.Call(Close);

			CacheHelper.StopJavaServer(1);

			CacheHelper.StopJavaLocator(1);

			CacheHelper.ClearEndpoints();
			CacheHelper.ClearLocators();

		}

		[Test]
		public void ValidCredentials()
		{
			runValidCredentials();
		}

	}


	public class UsernamePasswordAuthInitialize : IAuthInitialize
	{
		public bool called = false;

		public void Close() { }

		public Properties<string, object> GetCredentials(Properties<string, string> props, string server)
		{
			called = true;
			var credentials = new Properties<string, object>();
			credentials.Insert("security-username", props.Find("security-username"));
			credentials.Insert("security-password", props.Find("security-password"));
			return credentials;
		}
	}
}