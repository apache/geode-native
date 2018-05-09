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

namespace Apache.Geode.Client.UnitTests
{
    using NUnit.Framework;
    using Apache.Geode.DUnitFramework;

    [TestFixture]
    [Category("group3")]
    [Category("unicast_only")]
    [Category("generics")]
    public class TestCacheXmlInitialization : ThinClientRegionSteps
    {
        protected override ClientBase[] GetClients()
        {
            return new ClientBase[] { };
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


        [Test]
        public void TestCacheXmlInitialization_GetRegion()
        {
            CacheHelper.SetupJavaServers(true, "cacheserver_pdxinstance_hashcode.xml");

            CacheHelper.StartJavaLocator(1, "GFELOC");

            CacheHelper.StartJavaServerWithLocators(1, "GFECS1", 1);

            CacheHelper.InitConfig("client_pdx.xml");

            var region = CacheHelper.GetRegion<object, object>("DistRegionAck");

            Close();
        }
    }
}
