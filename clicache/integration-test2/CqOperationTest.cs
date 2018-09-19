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
        public virtual void OnEvent(CqEvent<TKey, TResult> ev)
        {
            Debug.WriteLine("MyCqListener::OnEvent called");
            MyOrder val = ev.getNewValue() as MyOrder;
            TKey key = ev.getKey();
            string operationType = "UNKNOWN";


            switch (ev.getQueryOperation())
            {
                case CqOperation.OP_TYPE_CREATE:
                    operationType = "CREATE";
                    break;
                case CqOperation.OP_TYPE_UPDATE:
                    operationType = "UPDATE";
                    break;
                case CqOperation.OP_TYPE_DESTROY:
                    operationType = "DESTROY";
                    break;
                default:
                    string message = string.Format("Operation value {0} is not valid!", ev.getQueryOperation());
                    Assert.True(false, message);
                    break;
            }

            if (val != null)
            {
                Debug.WriteLine("MyCqListener::OnEvent({0}) called with key {1}, value {2}", operationType, key, val.ToString());
            }
            else
            {
                Debug.WriteLine("MyCqListener::OnEvent({0}) called with key {1}, value null", operationType, key);
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
        private readonly Cache _cacheOne;
        private readonly GeodeServer _geodeServer;

        public CqOperationTest()
        {
            var cacheFactory = new CacheFactory()
                .Set("log-level", "error");

            _cacheOne = cacheFactory.Create();
            _geodeServer = new GeodeServer();

        }

        public void Dispose()
        {
            _cacheOne.Close();
            _geodeServer.Dispose();
        }

        [Fact]
        public void NotificationsHaveCorrectValues()
        {
//            var cacheXml = new CacheXml(new FileInfo("cache.xml"), _geodeServer);
//            _cacheOne.InitializeDeclarativeCache(cacheXml.File.FullName);

            _cacheOne.TypeRegistry.RegisterPdxType(MyOrder.CreateDeserializable);

            var poolFactory = _cacheOne.GetPoolFactory()
                .AddLocator("localhost", _geodeServer.LocatorPort);
            var pool = poolFactory
              .SetSubscriptionEnabled(true)
              .Create("pool");

            var regionFactory = _cacheOne.CreateRegionFactory(RegionShortcut.PROXY)
                .SetPoolName("pool");

            var region = regionFactory.Create<string, MyOrder>("testRegion");
            //var region = _cacheOne.GetRegion<string, MyOrder>("testRegion");

            var queryService = pool.GetQueryService();
            var cqAttributesFactory = new CqAttributesFactory<string, MyOrder>();
            var cqListener = new MyCqListener<string, MyOrder>();
            cqAttributesFactory.AddCqListener(cqListener);
            var cqAttributes = cqAttributesFactory.Create();
            
            var query = queryService.NewCq("MyCq", "SELECT * FROM /testRegion WHERE quantity > 30", cqAttributes, false);
            Debug.WriteLine("Executing continuous query");
            query.Execute();
                  
            Debug.WriteLine("Putting and changing Order objects in the region");
            var order1 = new MyOrder(1, "product x", 23);
            var order2 = new MyOrder(2, "product y", 37);
            
            region.Put("order1", order1);
            region.Put("order2", order2);
  
            Debug.WriteLine("Post PUTTING... let's sleep");
            System.Threading.Thread.Sleep(2000);     
            Debug.WriteLine("Waking up and going away");
        }
    }
}