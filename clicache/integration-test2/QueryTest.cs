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
using System.IO;
using Xunit;
using System.Diagnostics;
using System.Threading;

namespace Apache.Geode.Client.IntegrationTests
{
  public class QueryOrder : IPdxSerializable
  {
    private const string ORDER_ID_KEY_ = "order_id";
    private const string NAME_KEY_ = "name";
    private const string QUANTITY_KEY_ = "quantity";
    public long OrderId { get; set; }
    public string Name { get; set; }
    public short Quantity { get; set; }
    // A default constructor is required for deserialization
    public QueryOrder() { }
    public QueryOrder(int orderId, string name, short quantity)
    {
      OrderId = orderId;
      Name = name;
      Quantity = quantity;
    }
    public override string ToString()
    {
      return string.Format("Order: [{0}, {1}, {2}]", OrderId, Name, Quantity);
    }
    public void ToData(IPdxWriter output)
    {
      output.WriteLong(ORDER_ID_KEY_, OrderId);
      output.MarkIdentityField(ORDER_ID_KEY_);
      output.WriteString(NAME_KEY_, Name);
      output.MarkIdentityField(NAME_KEY_);
      output.WriteInt(QUANTITY_KEY_, Quantity);
      output.MarkIdentityField(QUANTITY_KEY_);
    }
    public void FromData(IPdxReader input)
    {
      OrderId = input.ReadLong(ORDER_ID_KEY_);
      Name = input.ReadString(NAME_KEY_);
      Quantity = (short)input.ReadInt(QUANTITY_KEY_);
    }
    public static IPdxSerializable CreateDeserializable()
    {
      return new QueryOrder();
    }
  }


  [Trait("Category", "Integration")]
  public class QueryTest : TestBase, IDisposable
  {
    private readonly Cache cache_;

    public QueryTest()
    {
      var cacheFactory = new CacheFactory()
          .Set("log-level", "error");
      cache_ = cacheFactory.Create();
    }

    public void Dispose()
    {
      cache_.Close();
    }

    [Fact]
    public void PdxSerializableNotificationsHaveCorrectValues()
    {
      using (var cluster_ = new Cluster(CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.Equal(cluster_.Start(), true);
        Assert.Equal(cluster_.Gfsh.create()
            .region()
            .withName("cqTestRegion")
            .withType("REPLICATE")
            .execute(), 0);
        cache_.TypeRegistry.RegisterPdxType(QueryOrder.CreateDeserializable);
        var poolFactory = cache_.GetPoolFactory()
            .AddLocator("localhost", cluster_.Gfsh.LocatorPort);
        var pool = poolFactory
            .SetSubscriptionEnabled(true)
            .Create("pool");

        var regionFactory = cache_.CreateRegionFactory(RegionShortcut.PROXY)
            .SetPoolName("pool");

        var region = regionFactory.Create<string, QueryOrder>("cqTestRegion");

        var queryService = pool.GetQueryService();

        Debug.WriteLine("Putting and changing Position objects in the region");
        var order1 = new QueryOrder(1, "product x", 23);
        var order2 = new QueryOrder(2, "product y", 37);
        var order3 = new QueryOrder(3, "product z", 101);

        region.Put("order1", order1);

        region.Put("order2", order2);

        region.Put("order3", order3);

        order1.Quantity = 20;
        region.Put("order1", order1);

        order2.Quantity = 45;
        region.Put("order2", order2);

        order3.Quantity = 11;
        region.Put("order3", order3);


        var results = region.Query<QueryOrder>("SELECT * FROM /cqTestRegion WHERE quantity > 30");
        Assert.Equal(results.Size, 1UL);
        Assert.Equal(results[0].Name, "product y");

      }
    }

    [Fact]
    public void DataSerializableNotificationsHaveCorrectValues()
    {
      using (var cluster_ = new Cluster(CreateTestCaseDirectoryName(), 1, 1))
      {
        Assert.Equal(cluster_.Start(), true);
        Assert.Equal(cluster_.Gfsh.deploy()
            .withJar(Config.JavaobjectJarPath)
            .execute(), 0);
        Assert.Equal(cluster_.Gfsh.create()
            .region()
            .withName("cqTestRegion")
            .withType("REPLICATE")
            .execute(), 0);

        cluster_.Gfsh.executeFunction()
            .withId("InstantiateDataSerializable")
            .withMember("DataSerializableNotificationsH_server_0")
            .execute();

        cache_.TypeRegistry.RegisterType(Position.CreateDeserializable, 22);

        var poolFactory = cache_.GetPoolFactory()
            .AddLocator("localhost", cluster_.Gfsh.LocatorPort);
        var pool = poolFactory
            .SetSubscriptionEnabled(true)
            .Create("pool");

        var regionFactory = cache_.CreateRegionFactory(RegionShortcut.PROXY)
            .SetPoolName("pool");

        var region = regionFactory.Create<string, Position>("cqTestRegion");

        Debug.WriteLine("Putting and changing Position objects in the region");
        var order1 = new Position("GOOG", 23);
        var order2 = new Position("IBM", 37);
        var order3 = new Position("PVTL", 101);

        region.Put("order1", order1);
        var Value = region["order1"];

        region.Put("order2", order2);

        order1.SharesOutstanding = 55;
        region.Put("order1", order1);

        order2.SharesOutstanding = 77;
        region.Put("order2", order2);

        order2.SharesOutstanding = 11;
        region.Put("order2", order2);

        region.Remove("order1");

        region.Put("order3", order3);

        var results = region.Query<Position>("SELECT * FROM /cqTestRegion WHERE sharesOutstanding > 50");
        Assert.Equal(results.Size, 1UL);
        Assert.Equal(results[0].SecId, "PVTL");
      }
    }
  }
}
