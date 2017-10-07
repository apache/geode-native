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

#pragma once

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/PoolManager.hpp>
#include "end_native.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      //generic<class TKey, class TValue>
      ref class Pool;
     // generic<class TKey, class TValue>
      ref class PoolFactory;

      namespace native = apache::geode::client;

      /// <summary>
      /// This interface provides for the configuration and creation of instances of PoolFactory.
      /// </summary>
     // generic<class TKey, class TValue>
      public ref class PoolManager
      {
      public:

        /// <summary>
        /// Creates a new PoolFactory which is used to configure and create Pools.
        /// </summary>
        PoolFactory/*<TKey, TValue>*/^ CreateFactory();

        /// <summary>
        /// Returns a map containing all the pools in this manager.
        /// The keys are pool names and the values are Pool instances.
        /// </summary>
        const Dictionary<String^, Pool/*<TKey, TValue>*/^>^ GetAll();

        /// <summary>
        /// Find by name an existing connection pool.
        /// </summary>
        Pool/*<TKey, TValue>*/^ Find(String^ name);

        /// <summary>
        /// Find the pool used by the given region.
        /// </summary>
        Pool/*<TKey, TValue>*/^ Find(Client::Region<Object^, Object^>^ region);

        /// <summary>
        /// Destroys all created pools.
        /// </summary>
        void Close(Boolean KeepAlive);

        /// <summary>
        /// Destroys all created pools.
        /// </summary>
        void Close();

      internal:

        native::PoolManager& GetNative()
        {
          return m_nativeref;
        }

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline PoolManager(native::PoolManager& nativeref)
          : m_nativeref(nativeref)
        {
        }

        native::PoolManager& m_nativeref;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

