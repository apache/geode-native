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

namespace Apache.Geode.Examples.ContinuousQuery
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
          .SetSubscriptionEnabled(true)
          .Create("pool");

      var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
          .SetPoolName("pool");
      var orderRegion = regionFactory.Create<string, Order>("example_orderobject");

      var queryService = pool.GetQueryService();

      var cqAttributesFactory = new CqAttributesFactory<string, Order>();

      var cqListener = new MyCqListener<string, Order>();

      cqAttributesFactory.AddCqListener(cqListener);

      var cqAttributes = cqAttributesFactory.Create();
      try
      {
        var query = queryService.NewCq("MyCq", "SELECT * FROM /example_orderobject WHERE quantity > 30", cqAttributes, false);

        Console.WriteLine("Executing continuous query");
        query.Execute();

        Console.WriteLine("Create orders");
        var order1 = new Order(1, "product x", 23);
        var order2 = new Order(2, "product y", 37);
        var order3 = new Order(3, "product z", 1);
        var order4 = new Order(4, "product z", 102);
        var order5 = new Order(5, "product x", 17);
        var order6 = new Order(6, "product z", 42);

        Console.WriteLine("Putting and changing Order objects in the region");
        orderRegion.Put("Order1", order1);
        orderRegion.Put("Order2", order2);
        orderRegion.Put("Order3", order3);
        orderRegion.Put("Order4", order4);
        orderRegion.Put("Order5", order5);
        orderRegion.Put("Order6", order6);

        orderRegion.Put("Order2", new Order(2, "product y", 45));
        orderRegion.Put("Order2", new Order(2, "product y", 29));
        orderRegion.Remove("Order6");

        System.Threading.Thread.Sleep(2000);

        query.Stop();
        query.Close();
      }
      catch (IllegalStateException ex)
      {
        Console.WriteLine(ex.Message);
      }

      cache.Close();
    }
  }
}
