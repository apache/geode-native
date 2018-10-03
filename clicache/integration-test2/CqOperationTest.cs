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

    public class MyCqListener<TKey, TResult> : ICqListener<TKey, TResult>
    {
        public AutoResetEvent RegionClearEvent { get; private set; }
        public AutoResetEvent CreatedEvent { get; private set; }
        public AutoResetEvent UpdatedEvent { get; private set; }
        public AutoResetEvent DestroyedNonNullEvent { get; private set; }
        public AutoResetEvent DestroyedNullEvent { get; private set; }
        public AutoResetEvent InvalidatedEvent { get; private set; }
        public bool ReceivedUnknownEventType { get; private set; }

        public MyCqListener()
        {
            CreatedEvent = new AutoResetEvent(false);
            UpdatedEvent = new AutoResetEvent(false);
            DestroyedNullEvent = new AutoResetEvent(false);
            DestroyedNonNullEvent = new AutoResetEvent(false);
            RegionClearEvent = new AutoResetEvent(false);
            InvalidatedEvent = new AutoResetEvent(false);
            ReceivedUnknownEventType = false;
        }

        public virtual void OnEvent(CqEvent<TKey, TResult> ev)
        {
            Debug.WriteLine("MyCqListener::OnEvent called");
            MyOrder val = ev.getNewValue() as MyOrder;
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

        public virtual void OnError(CqEvent<TKey, TResult> ev)
        {
            Debug.WriteLine("MyCqListener::OnError called");
        }

        public virtual void Close()
        {
            Debug.WriteLine("MyCqListener::close called");
        }
    }

    [Trait("Category", "Integration")]
    public class CqOperationTest : IDisposable
    {
        private readonly Cache cache_;
        private static int waitInterval_ = 1000;
        private Cluster cluster_;

        public CqOperationTest()
        {
            var cacheFactory = new CacheFactory()
                .Set("log-level", "error");

            cache_ = cacheFactory.Create();
            cluster_ = new Cluster("CqOperationTest", 1, 1);
            Assert.Equal(cluster_.Start(), true);
            Assert.Equal(cluster_.Gfsh.create()
                .region()
                .withName("cqTestRegion")
                .withType("REPLICATE")
                .execute(), 0);
        }

        public void Dispose()
        {
            cache_.Close();
            cluster_.Dispose();
        }

        [Fact]
        public void NotificationsHaveCorrectValues()
        {
            cache_.TypeRegistry.RegisterPdxType(MyOrder.CreateDeserializable);

            var poolFactory = cache_.GetPoolFactory()
                .AddLocator("localhost", cluster_.Gfsh.LocatorPort);
            var pool = poolFactory
              .SetSubscriptionEnabled(true)
              .Create("pool");

            var regionFactory = cache_.CreateRegionFactory(RegionShortcut.PROXY)
                .SetPoolName("pool");

            var region = regionFactory.Create<string, MyOrder>("cqTestRegion");

            var queryService = pool.GetQueryService();
            var cqAttributesFactory = new CqAttributesFactory<string, MyOrder>();
            var cqListener = new MyCqListener<string, MyOrder>();
            cqAttributesFactory.AddCqListener(cqListener);
            var cqAttributes = cqAttributesFactory.Create();
            
            var query = queryService.NewCq("MyCq", "SELECT * FROM /cqTestRegion WHERE quantity > 30", cqAttributes, false);
            Debug.WriteLine("Executing continuous query");
            query.Execute();
                  
            Debug.WriteLine("Putting and changing Order objects in the region");
            var order1 = new MyOrder(1, "product x", 23);
            var order2 = new MyOrder(2, "product y", 37);
            var order3 = new MyOrder(3, "product z", 101);
            
            region.Put("order1", order1);
            region.Put("order2", order2);
            Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receieve expected CREATE event");

            order1.Quantity = 60;
            region.Put("order1", order1);
            Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receieve expected CREATE event");

            order2.Quantity = 45;
            region.Put("order2", order2);
            Assert.True(cqListener.UpdatedEvent.WaitOne(waitInterval_), "Didn't receieve expected UPDATE event");

            order2.Quantity = 11;
            region.Put("order2", order2);
            Assert.True(cqListener.DestroyedNonNullEvent.WaitOne(waitInterval_), "Didn't receieve expected DESTROY event");

            region.Remove("order1");
            Assert.True(cqListener.DestroyedNullEvent.WaitOne(waitInterval_), "Didn't receieve expected DESTROY event");

            region.Put("order3", order3);
            Assert.True(cqListener.CreatedEvent.WaitOne(waitInterval_), "Didn't receieve expected CREATE event");

            region.Clear();
            Assert.True(cqListener.RegionClearEvent.WaitOne(waitInterval_), "Didn't receive expected CLEAR event");

            Assert.False(cqListener.ReceivedUnknownEventType, "An unknown event was received by CQ listener");
        }
    }
}



