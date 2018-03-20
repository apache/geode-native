//=========================================================================
// Copyright (c) 2002-2014 Pivotal Software, Inc. All Rights Reserved.
// This product is protected by U.S. and international copyright
// and intellectual property laws. Pivotal products are covered by
// more patents listed at http://www.pivotal.io/patents.
//========================================================================

using System;

namespace Apache.Geode.Client.UnitTests
{
    using NUnit.Framework;
    using Apache.Geode.DUnitFramework;
    using System.Diagnostics;
    using System.Threading.Tasks;

    [TestFixture]
    [Category("group3")]
    [Category("unicast_only")]
    [Category("generics")]
    public class TestCacheXmlInitialization : ThinClientRegionSteps
    {
        private UnitProcess _m_client1;

        protected override ClientBase[] GetClients()
        {
            _m_client1 = new UnitProcess();
            return new ClientBase[] { _m_client1 };
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

            CacheHelper.StopJavaServer(1);

            CacheHelper.StopJavaLocator(1);
        }

        public void InitClientXml(string cacheXml, int serverport1, int serverport2)
        {
            CacheHelper.HOST_PORT_1 = serverport1;
            CacheHelper.HOST_PORT_2 = serverport2;
            CacheHelper.InitConfig(cacheXml);
        }
    }
}
