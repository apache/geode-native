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
using Apache.Geode.Client;

namespace Apache.Geode.Examples.Serializer
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var cacheFactory = new CacheFactory()
                .Set("log-level", "none");
            var cache = cacheFactory.Create();

            Console.WriteLine("Registering for reflection-based auto serialization");

            cache.TypeRegistry.PdxSerializer = new ReflectionBasedAutoSerializer();

            var poolFactory = cache.GetPoolFactory()
                .AddLocator("localhost", 10334);
            poolFactory.Create("pool");

            var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                .SetPoolName("pool");
            var orderRegion = regionFactory.Create<int, Order>("example_orderobject");

            Console.WriteLine("Storing order object in the region");

            const int orderKey = 65;

            var order = new Order(orderKey, "Vox AC30", 11);

            Console.WriteLine("order to put is " + order);
            orderRegion.Put(orderKey, order, null);

            Console.WriteLine("Successfully put order, getting now...");
            var orderRetrieved = orderRegion.Get(orderKey, null);

            Console.WriteLine("Order key: " + orderKey + " = " + orderRetrieved);

            cache.Close();
        }
    }
}


