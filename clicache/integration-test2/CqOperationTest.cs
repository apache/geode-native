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

namespace Apache.Geode.Client.IntegrationTests
{
    public class MyCqListener<TKey, TResult> : ICqListener<TKey, TResult>
    {
        public virtual void OnEvent(CqEvent<TKey, TResult> ev)
        {
            Order val = ev.getNewValue() as Order;
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
                    Console.WriteLine("Unexpected operation encountered {0}", ev.getQueryOperation());
                    break;
            }

            if (val != null)
            {
                Console.WriteLine("MyCqListener::OnEvent({0}) called with key {1}, value {2}", operationType, key, val.ToString());
            }
            else
            {
                Console.WriteLine("MyCqListener::OnEvent({0}) called with key {1}, value null", operationType, key);
            }
        }

        public virtual void OnError(CqEvent<TKey, TResult> ev)
        {
            Console.WriteLine("MyCqListener::OnError called");
        }

        public virtual void Close()
        {
            Console.WriteLine("MyCqListener::close called");
        }
    }

    [Trait("Category", "Integration")]
    public class CqOperationTest : IDisposable
    {
        private readonly Cache _cacheOne;
        private readonly GeodeServer _geodeServer;

        public CqOperationTest()
        {
            var cacheFactory = new CacheFactory();

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
            Assert.True(false);
        }
    }
}