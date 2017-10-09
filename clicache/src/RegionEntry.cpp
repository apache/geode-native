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

//#include "geode_includes.hpp"
#include "RegionEntry.hpp"
#include "Region.hpp"
#include "CacheStatistics.hpp"
#include "impl/SafeConvert.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TKey, class TValue>
      TKey RegionEntry<TKey, TValue>::Key::get( )
      {        
        try
        {
          return Serializable::GetManagedValueGeneric<TKey>(m_nativeptr->get()->getKey());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      TValue RegionEntry<TKey, TValue>::Value::get( )
      {
        try
        {
          return Serializable::GetManagedValueGeneric<TValue>(m_nativeptr->get()->getValue());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ RegionEntry<TKey, TValue>::Region::get( )
      {
        try
        {
          return Apache::Geode::Client::Region<TKey, TValue>::Create(m_nativeptr->get()->getRegion());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      Apache::Geode::Client::CacheStatistics^ RegionEntry<TKey, TValue>::Statistics::get( )
      {
        apache::geode::client::CacheStatisticsPtr nativeptr;
        try
        {
          nativeptr = m_nativeptr->get()->getStatistics();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return Apache::Geode::Client::CacheStatistics::Create( nativeptr);
      }

      generic<class TKey, class TValue>
      bool RegionEntry<TKey, TValue>::IsDestroyed::get( )
      {
        try
        {
          return m_nativeptr->get()->isDestroyed( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
