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
using System.Reflection;

#pragma warning disable 618

namespace Apache.Geode.Client.UnitTests
{
  using NUnit.Framework;
  using Apache.Geode.DUnitFramework;
  
 

  [TestFixture]
  [Category("group1")]
  [Category("unicast_only")]
  [Category("generics")]
  public class AttributesFactoryTests : UnitTests
  {

    private UnitProcess m_client1, m_client2;

    protected override ClientBase[] GetClients()
    {
      m_client1 = new UnitProcess();
      m_client2 = new UnitProcess();

      return new ClientBase[] { m_client1, m_client2 };
    }


    [Test]
    public void fluentModeltest()
    {
      AttributesFactory<string, string> af = new AttributesFactory<string, string>();
      Apache.Geode.Client.RegionAttributes<string, string> rattrs = af.SetLruEntriesLimit(0).SetInitialCapacity(5).CreateRegionAttributes();
      Assert.IsNotNull(rattrs);
      Assert.True(rattrs.LruEntriesLimit == 2);
      Assert.True(rattrs.InitialCapacity == 5);
    }
  }
}
