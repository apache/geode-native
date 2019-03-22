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
using Xunit.Abstractions;

namespace Apache.Geode.Client.IntegrationTests
{
    public class MyOrder : IPdxSerializable
    {
        private const string ORDER_ID_KEY_ = "order_id";
        private const string NAME_KEY_ = "name";
        private const string QUANTITY_KEY_ = "quantity";
        public long OrderId { get; set; }
        public string Name { get; set; }
        public short Quantity { get; set; }
        // A default constructor is required for deserialization
        public MyOrder() { }
        public MyOrder(int orderId, string name, short quantity)
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
            return new MyOrder();
        }
    }

    public abstract class CqListener<TKey, TResult> : ICqListener<TKey, TResult>
    {
        public AutoResetEvent RegionClearEvent { get; private set; }
        public AutoResetEvent CreatedEvent { get; private set; }
        public AutoResetEvent UpdatedEvent { get; private set; }
        public AutoResetEvent DestroyedNonNullEvent { get; private set; }
        public AutoResetEvent DestroyedNullEvent { get; private set; }
        public AutoResetEvent InvalidatedEvent { get; private set; }
        public bool ReceivedUnknownEventType { get; internal set; }

        public CqListener()
        {
            CreatedEvent = new AutoResetEvent(false);
            UpdatedEvent = new AutoResetEvent(false);
            DestroyedNullEvent = new AutoResetEvent(false);
            DestroyedNonNullEvent = new AutoResetEvent(false);
            RegionClearEvent = new AutoResetEvent(false);
            InvalidatedEvent = new AutoResetEvent(false);
            ReceivedUnknownEventType = false;
        }

        public abstract void OnEvent(CqEvent<TKey, TResult> ev);

        public virtual void OnError(CqEvent<TKey, TResult> ev)
        {
        }
        public virtual void Close()
        {

        }
    }

    public class PdxCqListener<TKey, TResult> : CqListener<TKey, TResult>
    {
        public override void OnEvent(CqEvent<TKey, TResult> ev)
        {
            Debug.WriteLine("PdxCqListener::OnEvent called");
            var val = ev.getNewValue() as MyOrder;
            TKey key = ev.getKey();

            switch (ev.getQueryOperation())
            {
                case CqOperation.OP_TYPE_REGION_CLEAR:
                    RegionClearEvent.Set();
                    break;
                case CqOperation.OP_TYPE_CREATE:
                    CreatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_UPDATE:
                    UpdatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_INVALIDATE:
                    InvalidatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_DESTROY:
                    if (val == null)
                    {
                        DestroyedNullEvent.Set();
                    }
                    else
                    {
                        DestroyedNonNullEvent.Set();
                    }
                    break;
                default:
                    ReceivedUnknownEventType = true;
                    break;
            }
        }
    }

    public class DataCqListener<TKey, TResult> : CqListener<TKey, TResult>
    {
        public override void OnEvent(CqEvent<TKey, TResult> ev)
        {
            Debug.WriteLine("CqListener::OnEvent called");
            var val = ev.getNewValue() as Position;
            TKey key = ev.getKey();

            switch (ev.getQueryOperation())
            {
                case CqOperation.OP_TYPE_REGION_CLEAR:
                    RegionClearEvent.Set();
                    break;
                case CqOperation.OP_TYPE_CREATE:
                    CreatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_UPDATE:
                    UpdatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_INVALIDATE:
                    InvalidatedEvent.Set();
                    break;
                case CqOperation.OP_TYPE_DESTROY:
                    if (val == null)
                    {
                        DestroyedNullEvent.Set();
                    }
                    else
                    {
                        DestroyedNonNullEvent.Set();
                    }
                    break;
                default:
                    ReceivedUnknownEventType = true;
                    break;
            }
        }
    }

    [Trait("Category", "Integration")]
    public class CqOperationTest : TestBase
    {
        private static int waitInterval_ = 1000;

        public CqOperationTest(ITestOutputHelper testOutputHelper) : base(testOutputHelper)
        {
        }

        [Fact]
        public void PdxSerializableNotificationsHaveCorrectValues()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("cqTestRegion")
                    .withType("REPLICATE")
                    .execute());

                var cache = cluster.CreateCache();

                cache.TypeRegistry.RegisterPdxType(MyOrder.CreateDeserializable);

                var pool = cluster.ApplyLocators(cache.GetPoolFactory())
                    .SetSubscriptionEnabled(true)
                    .Create("pool");

                var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("pool");
          
                var region = regionFactory.Create<string, MyOrder>("cqTestRegion");
          
                var queryService = pool.GetQueryService();
                var cqAttributesFactory = new CqAttributesFactory<string, MyOrder>();
                var cqListener = new PdxCqListener<string, MyOrder>();
                cqAttributesFactory.AddCqListener(cqListener);
                var cqAttributes = cqAttributesFactory.Create();
                
                var query = queryService.NewCq("MyCq", "SELECT * FROM /cqTestRegion WHERE quantity > 30", cqAttributes, false);
                Debug.WriteLine("Executing continuous query");
                query.Execute();
                  
                Debug.WriteLine("Putting and changing Position objects in the region");
                var order1 = new MyOrder(1, "product x", 23);
                var order2 = new MyOrder(2, "product y", 37);
                var order3 = new MyOrder(3, "product z", 101);
          
                region.Put("order1", order1);
          
                region.Put("order2", order2);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");
          
                order1.Quantity = 60;
                region.Put("order1", order1);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");
          
                order2.Quantity = 45;
                region.Put("order2", order2);
                Assert.True(cqListener.UpdatedEvent.WaitOne(waitInterval_), "Didn't receive expected UPDATE event");
          
                order2.Quantity = 11;
                region.Put("order2", order2);
                Assert.True(cqListener.DestroyedNonNullEvent.WaitOne(waitInterval_), "Didn't receive expected DESTROY event");
          
                region.Remove("order1");
                Assert.True(cqListener.DestroyedNullEvent.WaitOne(waitInterval_), "Didn't receive expected DESTROY event");
          
                region.Put("order3", order3);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");
          
                region.Clear();
                Assert.True(cqListener.RegionClearEvent.WaitOne(waitInterval_), "Didn't receive expected CLEAR event");
          
                Assert.False(cqListener.ReceivedUnknownEventType, "An unknown event was received by CQ listener");

                cache.Close();
            }
        }
  
        [Fact]
        public void DataSerializableNotificationsHaveCorrectValues()
        {
            using (var cluster = new Cluster(output, CreateTestCaseDirectoryName(), 1, 1))
            {
                Assert.True(cluster.Start());
                Assert.Equal(0, cluster.Gfsh.deploy()
                    .withJar(Config.JavaobjectJarPath)
                    .execute());
                Assert.Equal(0, cluster.Gfsh.create()
                    .region()
                    .withName("cqTestRegion")
                    .withType("REPLICATE")
                    .execute());
                Assert.Equal(0, cluster.Gfsh.executeFunction()
                    .withId("InstantiateDataSerializable")
                    .withMember("DataSerializableNotificationsHaveCorrectValues_server_0")
                    .execute());

                var cache = cluster.CreateCache();

                cache.TypeRegistry.RegisterType(Position.CreateDeserializable, 22);

                var pool = cluster.ApplyLocators(cache.GetPoolFactory())
                    .SetSubscriptionEnabled(true)
                    .Create("pool");

                var regionFactory = cache.CreateRegionFactory(RegionShortcut.PROXY)
                    .SetPoolName("pool");

                var region = regionFactory.Create<string, Position>("cqTestRegion");

                var queryService = pool.GetQueryService();
                var cqAttributesFactory = new CqAttributesFactory<string, Position>();
                var cqListener = new DataCqListener<string, Position>();
                cqAttributesFactory.AddCqListener(cqListener);
                var cqAttributes = cqAttributesFactory.Create();

                var query = queryService.NewCq("MyCq", "SELECT * FROM /cqTestRegion WHERE sharesOutstanding > 30", cqAttributes, false);
                Debug.WriteLine("Executing continuous query");
                query.Execute();

                Debug.WriteLine("Putting and changing Position objects in the region");
                var order1 = new Position("GOOG", 23);
                var order2 = new Position("IBM", 37);
                var order3 = new Position("PVTL", 101);

                region.Put("order1", order1);
                var Value = region["order1"];

                region.Put("order2", order2);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");

                order1.SharesOutstanding = 55;
                region.Put("order1", order1);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");

                order2.SharesOutstanding = 77;
                region.Put("order2", order2);
                Assert.True(cqListener.UpdatedEvent.WaitOne(waitInterval_), "Didn't receive expected UPDATE event");

                order2.SharesOutstanding = 11;
                region.Put("order2", order2);
                Assert.True(cqListener.DestroyedNonNullEvent.WaitOne(waitInterval_), "Didn't receive expected DESTROY event");

                region.Remove("order1");
                Assert.True(cqListener.DestroyedNullEvent.WaitOne(waitInterval_), "Didn't receive expected DESTROY event");

                region.Put("order3", order3);
                Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receive expected CREATE event");

                region.Clear();
                Assert.True(cqListener.RegionClearEvent.WaitOne(waitInterval_), "Didn't receive expected CLEAR event");

                Assert.False(cqListener.ReceivedUnknownEventType, "An unknown event was received by CQ listener");

                cache.Close();
            }
        }
    }
}
