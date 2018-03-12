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
using NUnit.Framework;
using Apache.Geode.DUnitFramework;
using Apache.Geode.Client;

namespace Apache.Geode.Client.UnitTests
{

  [TestFixture]
  public class ThinClientMultipleCacheTests
  {
    [Test]
    public void RegisterSerializerForTwoCaches()
    {
      var cacheFactory = new CacheFactory();
      var cacheOne = cacheFactory.Create();
      var cacheTwo = cacheFactory.Create();

      Assert.AreNotEqual(cacheOne, cacheTwo);
      
      var dummyPdxSerializer = new DummySerializer();
      cacheOne.TypeRegistry.PdxSerializer = dummyPdxSerializer;

      Assert.AreSame(dummyPdxSerializer, cacheOne.TypeRegistry.PdxSerializer);
      //cacheOne.getTypeRegistry().RegisiterPdxSerilizer();

      //new Serializable().PdxSerializer = );
    }
  }

  internal class DummySerializer : IPdxSerializer
  {
    public object FromData(string classname, IPdxReader reader)
    {
      throw new NotImplementedException();
    }

    public bool ToData(object o, IPdxWriter writer)
    {
      throw new NotImplementedException();
    }
  }
}
