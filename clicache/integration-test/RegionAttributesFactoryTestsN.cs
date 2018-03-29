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
  public class RegionAttributesFactoryTests : UnitTests
  {

    protected override ClientBase[] GetClients()
    {
      return new ClientBase[] {};
    }


    [Test]
    public void fluentModeltest()
    {
      var regionAttributesFactory = new RegionAttributesFactory<string, string>();
      var regionAttributes = regionAttributesFactory.SetLruEntriesLimit(2).SetInitialCapacity(5).Create();
      Assert.IsNotNull(regionAttributes);
      Assert.True(regionAttributes.LruEntriesLimit == 2);
      Assert.True(regionAttributes.InitialCapacity == 5);
    }
  }
}
