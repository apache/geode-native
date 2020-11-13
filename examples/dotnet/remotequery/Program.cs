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
            var cache = new CacheFactory()
              .Set("log-level", "none")
              .Create();

            Console.WriteLine("Registering for data serialization");

            cache.TypeRegistry.RegisterPdxType(Order.CreateDeserializable);

            var pool = cache.GetPoolManager()
                .CreateFactory()
                .AddLocator("localhost", 10334)
                .Create("pool");

            var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                .SetPoolName("pool");
            var orderRegion = regionFactory.Create<string, Order>("custom_orders");

            Console.WriteLine("Create orders");
            var order1 = new Order(1, "product x", 23);
            var order2 = new Order(2, "product y", 37);
            var order3 = new Order(3, "product z", 1);
            var order4 = new Order(4, "product z", 102);
            var order5 = new Order(5, "product x", 17);
            var order6 = new Order(6, "product z", 42);

            Console.WriteLine("Storing orders in the region");
            orderRegion.Put("Order1", order1);
            orderRegion.Put("Order2", order2);
            orderRegion.Put("Order3", order3);
            orderRegion.Put("Order4", order4);
            orderRegion.Put("Order5", order5);
            orderRegion.Put("Order6", order6);

            var queryService = pool.GetQueryService();

            Console.WriteLine("Getting the orders from the region");
            var query = queryService.NewQuery<Order>("SELECT * FROM /custom_orders WHERE quantity > 30");
            var queryResults = query.Execute();

            Console.WriteLine("The following orders have a quantity greater than 30:");

            foreach (Order value in queryResults)
            {
                Console.WriteLine(value.ToString()); 
            }

            cache.Close();
        }
    }
}


