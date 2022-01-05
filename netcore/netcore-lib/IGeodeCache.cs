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

namespace Apache.Geode.Client {
  public interface IGeodeCache<TKey, TValue> : IRegionService<TKey, TValue>, IDisposable {
    bool GetPdxIgnoreUnreadFields();
    bool GetPdxReadSerialized();
    //                void InitializeDeclarativeCache(String cacheXml);

    //                CacheTransactionManager CacheTransactionManager { get; }
    //                DistributedSystem DistributedSystem { get; }
    String Name { get; }
    PoolManager PoolManager { get; }
    PoolFactory PoolFactory { get; }
    void Close();
    void Close(bool keepalive);
    bool Closed { get; }
  }
}
